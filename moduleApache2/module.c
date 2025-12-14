#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <jwt.h>
#include "http_request.h"
#include "http_log.h"
#include "apr_strings.h"
#include "apr_shm.h"
#include "apr_proc_mutex.h"
#include "macro.h"
#include "cJSON.h"
#include "json.h"
#include "login.h"
#include "module.h"

//Defines
#define STATS

#undef PRINTF_
#undef PRINTF
#define PRINTF_(level, format, ...) {ap_log_error(APLOG_MARK, level, 0, s, format, ##__VA_ARGS__);}
#define PRINTF(format, ...) PRINTF_(APLOG_ERR, format, ##__VA_ARGS__)

//Functions
static apr_status_t cleanup(void *data) {
	configVH *confVH = (configVH *)data;
	int *global_hits = apr_shm_baseaddr_get(confVH->shm_hits);
	time_t *global_lasttime = apr_shm_baseaddr_get(confVH->shm_lasttime);
	if (global_hits && global_lasttime && *global_hits) {
		char sz[256];
		snprintf(sz, sizeof(sz), "/var/log/apache2/stats-%s.json", confVH->name);
		cJSON *el = jsonRead(sz);
		if (!el)
			el = cJSON_CreateObject();
		int hits = (int)cJSON_GetNumberValue2(el, "hits");
		cJSON_AddNumberToObject(el, "hits", hits + *global_hits);
		cJSON_AddNumberToObject(el, "lasttime", *global_lasttime);
		jsonWrite(el, sz);
		chmod(sz, 0666);
	}
	return APR_SUCCESS;
}

static void *createConfig(apr_pool_t *p, server_rec *s) {
	if (s->is_virtual) {
		configVH *confVH = apr_pcalloc(p, sizeof(configVH));
		memset(confVH, 0, sizeof(configVH));
		apr_shm_create(&confVH->shm_hits, sizeof(int), NULL, p);
		int *global_hits = apr_shm_baseaddr_get(confVH->shm_hits);
		*global_hits = 0;
		apr_shm_create(&confVH->shm_lasttime, sizeof(time_t), NULL, p);
		time_t *global_lasttime = apr_shm_baseaddr_get(confVH->shm_lasttime);
		*global_lasttime = 0;
#ifdef STATS
		apr_proc_mutex_create(&confVH->mutex, "stats", APR_LOCK_DEFAULT, p);
#endif
		apr_pool_cleanup_register(p, confVH, cleanup, apr_pool_cleanup_null);
		return confVH;
	} else {
		configS *confS = apr_pcalloc(p, sizeof(configS));
		memset(confS, 0, sizeof(configS));
		return confS;
	}
}

static void *mergeConfig(apr_pool_t *p, void *basev, void *addv) {
	configS *confS = (configS *)basev;
	configVH *confVH = (configVH *)addv;
	confVH->jwkPem = confS->jwkPem;
	confVH->autologin = confS->autologin;
	return confVH;
}

char *getJwkPemContent(const char *arg) {
	char *sz = NULL;
	struct stat statTest;
	if (stat(arg, &statTest) == 0) {
		if (statTest.st_size > 100) {
			int size = statTest.st_size + 1;
			sz = malloc(size);
			FILE *pf = fopen(arg, "r");
			if (pf) {
				int ret = fread(sz, 1, size, pf);
				if (ret >= 0)
					sz[ret] = '\0';
				fclose(pf);
			}
		}
	}
	return sz;
}

static const char *moduleJwkPemFileSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	configS *confS = (configS *)ap_get_module_config(s->module_config, &app_module);
	confS->jwkPem = getJwkPemContent(arg);
	PRINTF("APP: Jwt %s", arg);
	return NULL;
}

static const char *moduleNameSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	configVH *confVH = (configVH *)ap_get_module_config(s->module_config, &app_module);
	confVH->name = arg;
	PRINTF("APP: Name %s", arg);
	return NULL;
}

static const char *modulePermissionSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	configVH *confVH = (configVH *)ap_get_module_config(s->module_config, &app_module);
	PRINTF("APP: Permission %s (for %s)", arg, confVH->name);
	if (confVH->permissions == NULL)
		confVH->permissions = cJSON_CreateObject();
	cJSON_AddBoolToObject(confVH->permissions, arg, cJSON_True);
	return NULL;
}

static const char *moduleAutoLoginSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	configS *confS = (configS *)ap_get_module_config(s->module_config, &app_module);
	confS->autologin = strcmp(arg, "on") == 0 ? 1 : 0;
	PRINTF("AppAutoLogin: %s", arg);
	return NULL;
}

char *extractCookieValue(const char *cookie, const char *name, struct request_rec *r) {
	if (cookie == NULL || name == NULL)
		return NULL;
	const char *name_start = strstr(cookie, name);
	if (name_start == NULL)
		return NULL;
	if (*(name_start + strlen(name)) != '=')
		return extractCookieValue(name_start + 1, name, r);

	const char *value_start = name_start + strlen(name) + 1;
	const char *value_end = strchr(value_start, ';');

	size_t value_len = value_end == NULL ? strlen(value_start) : (value_end - value_start);
	char *extracted_value = apr_pstrndup(r->pool, value_start, value_len);
	return extracted_value;
}

int checkAccess(cJSON *elPermissions, const char *role, const char *username) {
//Permissions: [ "_public_", "_dongle_", "_localnetwork_", "_groupadmin_", "_groupuser_", "admin", "user", ... ]
//Roles: admin or user
	if (cJSON_HasObjectItem(elPermissions, "_groupadmin_") && strcmp(role, "admin") == 0)
		return 1;
	if (cJSON_HasObjectItem(elPermissions, "_groupuser_") && (strcmp(role, "admin") == 0 || strcmp(role, "user") == 0))
		return 1;
	if (cJSON_HasObjectItem(elPermissions, username))
		return 1;
}

int decodeAndCheck(server_rec *s, const char *token, const char *keyPem, cJSON *elPermissions) {
	int ret = 0;
	jwt_t* jwt_decoded;
	int result = jwt_decode(&jwt_decoded, token, (const unsigned char*)keyPem, strlen(keyPem));
	if (result == 0) {
		time_t current_time = time(NULL);
		time_t exp_time = (time_t)jwt_get_grant_int(jwt_decoded, "exp");
		if (current_time < exp_time) {
			const char *jwtRole = jwt_get_grant(jwt_decoded, "role");
			const char *jwtUsername = jwt_get_grant(jwt_decoded, "username");
			//PRINTF("APP: JWT ret:%d role:%s username:%s\n", ret, jwtRole, jwtUsername);
			ret = checkAccess(elPermissions, jwtRole, jwtUsername);
		}
		jwt_free(jwt_decoded);
	} else {
		//PRINTF("APP: JWT verification failed: %d\n", result);
	}
	return ret;
}

int authorization2(request_rec *r, int strict) {
	server_rec *s = r->server;
	configVH *confVH = (configVH *)ap_get_module_config(s->module_config, &app_module);
#ifdef STATS
	int *global_hits = apr_shm_baseaddr_get(confVH->shm_hits);
	time_t *global_lasttime = apr_shm_baseaddr_get(confVH->shm_lasttime);
	apr_proc_mutex_lock(confVH->mutex);
	(*global_hits)++;
	*global_lasttime = time(NULL);
	apr_proc_mutex_unlock(confVH->mutex);
	//PRINTF("APP: authorization1a name:%s uri:%s hits:%d", confVH->name, r->uri, *global_hits);
#endif
	//PRINTF("APP: authorization1b jwkPem: %.*s", 32, confVH->jwkPem);
	//char *ssz = cJSON_Print(confVH->permissions);
	//PRINTF("APP: authorization1c permissions: %s\n", ssz);
	//free(ssz);
	if (!strict && (confVH->permissions == NULL || cJSON_HasObjectItem(confVH->permissions, "_public_")))
		return DECLINED;
	const char *cookies = apr_table_get(r->headers_in, "Cookie");
	char *cookieJwt = extractCookieValue(cookies, "jwt", r);
	//PRINTF("APP: authorization2 cookieJwt: %s", cookieJwt);
	if (cookieJwt != NULL && strlen(cookieJwt) > 0)
		return decodeAndCheck(s, cookieJwt, confVH->jwkPem, confVH->permissions) == 1 ? DECLINED : HTTP_UNAUTHORIZED;
	return -2;
}

static int authorization(request_rec *r) {
	server_rec *s = r->server;
	const char *current_uri = r->uri;
	if (current_uri != NULL && strncmp(current_uri, "/_app_/", 7) == 0)
		return DECLINED;
	int ret = authorization2(r, 0);
	if (ret != -2)
		return ret;
	configVH *confVH = (configVH *)ap_get_module_config(s->module_config, &app_module);
	if (confVH->name != NULL && strcmp(confVH->name, "livecodes") == 0  && current_uri != NULL && strncmp(current_uri, "/livecodes/", 11) == 0)
			return DECLINED;
	//PRINTF("APP: authorization3 HTTP_UNAUTHORIZED name:%s uri:%s", confVH->name, r->uri);
	return HTTP_UNAUTHORIZED;
}

static const command_rec directives[] = {
	AP_INIT_TAKE1("AppJwkPem", moduleJwkPemFileSet, NULL, RSRC_CONF | ACCESS_CONF, "App Jwt Key"),
	AP_INIT_TAKE1("AppModule", moduleNameSet, NULL, RSRC_CONF | ACCESS_CONF, "App module name"),
	AP_INIT_ITERATE("AppModulePermission", modulePermissionSet, NULL, RSRC_CONF | ACCESS_CONF, "Permission for App module"),
	AP_INIT_TAKE1("AppALEnabled", moduleAutoLoginSet, NULL, RSRC_CONF | ACCESS_CONF, "AutoLogin Enabled"),
	{NULL}
};

static void registerHooks(apr_pool_t *p) {
	ap_hook_access_checker(authorization, NULL, NULL, APR_HOOK_FIRST);
	registerFilter();
}

module AP_MODULE_DECLARE_DATA app_module = {
	STANDARD20_MODULE_STUFF, NULL, NULL, createConfig, mergeConfig, directives, registerHooks
};
