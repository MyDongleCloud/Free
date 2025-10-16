#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <jwt.h>
#include "util_cookies.h"
#include "http_request.h"
#include "http_log.h"
#include "apr_strings.h"
#include "macro.h"
#include "cJSON.h"

//Struct
typedef struct {
	const char *jwkPemFile;
	const char *jwkPem;
	const char *name;
	cJSON *permissions;
} config;

//Defines
#undef PRINTF_
#undef PRINTF
#define PRINTF_(level, format, ...) {ap_log_error(APLOG_MARK, level, 0, s, format, ##__VA_ARGS__);}
#define PRINTF(format, ...) PRINTF_(APLOG_ERR, format, ##__VA_ARGS__)

//Functions
module AP_MODULE_DECLARE_DATA mydonglecloud_module;

static void *createConfig(apr_pool_t *p, server_rec *s) {
	config *confD = apr_pcalloc(p, sizeof(config));
	confD->jwkPemFile = NULL;
	confD->jwkPem = NULL;
	confD->name = NULL;
	confD->permissions = NULL;
	return confD;
}

static void *mergeConfig(apr_pool_t *p, void *basev, void *addv) {
	config *base = (config *)basev;
	config *add = (config *)addv;
	config *conf = (config *)apr_pcalloc(p, sizeof(config));
	conf->jwkPemFile = base->jwkPemFile;
	conf->jwkPem = base->jwkPem;
	conf->name = add->name;
	conf->permissions = add->permissions;
	return conf;
}

void getJwkPemContent(server_rec *s, config *confD) {
	struct stat statTest;
	if (stat(confD->jwkPemFile, &statTest) == 0) {
		if (statTest.st_size > 100) {
			int size = statTest.st_size + 1;
			char *sz = malloc(size);
			FILE *pf = fopen(confD->jwkPemFile, "r");
			if (pf) {
				int ret = fread(sz, 1, size, pf);
				if (ret >= 0)
					sz[ret] = '\0';
				confD->jwkPem = sz;
				fclose(pf);
			}
		}
	}
}

static const char *moduleJwkPemFileSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	config *confD = (config *)ap_get_module_config(s->module_config, &mydonglecloud_module);
	confD->jwkPemFile = arg;
	getJwkPemContent(s, confD);
	PRINTF("MDC: Jwt %s", arg);
	return NULL;
}

static const char *moduleNameSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	config *confD = (config *)ap_get_module_config(s->module_config, &mydonglecloud_module);
	confD->name = arg;
	PRINTF("MDC: Name %s", arg);
	return NULL;
}

static const char *modulePermissionSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	config *confD = (config *)ap_get_module_config(s->module_config, &mydonglecloud_module);
	PRINTF("MDC: Permission %s (for %s)", arg, confD->name);
	if (confD->permissions == NULL)
		confD->permissions = cJSON_CreateObject();
	cJSON_AddBoolToObject(confD->permissions, arg, cJSON_True);
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
			//PRINTF("MDC: JWT ret:%d role:%s username:%s\n", ret, jwtRole, jwtUsername);
			ret = checkAccess(elPermissions, jwtRole, jwtUsername);
		}
		jwt_free(jwt_decoded);
	} else {
		//PRINTF("MDC: JWT verification failed: %d\n", result);
	}
	return ret;
}

static int authorization(request_rec *r) {
	server_rec *s = r->server;
	const char *current_uri = r->uri;
	if (current_uri != NULL && strncmp(current_uri, "/MyDongleCloud", 14) == 0)
		return DECLINED;
	config *confD = (config *)ap_get_module_config(s->module_config, &mydonglecloud_module);
	//PRINTF("MDC: authorization1a jwkPemFile: %s", confD->jwkPemFile);
	if (confD->jwkPem == NULL)
		getJwkPemContent(s, confD);
	//PRINTF("MDC: authorization1b jwkPem: %s", confD->jwkPem);
	//PRINTF("MDC: authorization1c name:%s uri:%s", confD->name, r->uri);
	//char *ssz = cJSON_Print(confD->permissions);
	//PRINTF("MDC: authorization1d permissions: %s\n", ssz);
	//free(ssz);
	if (confD->permissions == NULL || cJSON_HasObjectItem(confD->permissions, "_public_"))
		return DECLINED;
	const char *cookies = apr_table_get(r->headers_in, "Cookie");
	char *cookieJwt = extractCookieValue(cookies, "jwt", r);
	//PRINTF("MDC: authorization2 cookieJwt: %s", cookieJwt);
	if (cookieJwt != NULL && strlen(cookieJwt) > 0)
		return decodeAndCheck(s, cookieJwt, confD->jwkPem, confD->permissions) == 1 ? DECLINED : HTTP_UNAUTHORIZED;
	if (confD->name != NULL && strcmp(confD->name, "livecodes") == 0  && current_uri != NULL && strncmp(current_uri, "/livecodes/", 11) == 0)
			return DECLINED;
	//PRINTF("MDC: authorization3 HTTP_UNAUTHORIZED name:%s uri:%s", confD->name, r->uri);
	return HTTP_UNAUTHORIZED;
}

static const command_rec directives[] = {
	AP_INIT_TAKE1("MyDongleCloudJwkPem", moduleJwkPemFileSet, NULL, RSRC_CONF | ACCESS_CONF, "MyDongleCloud Jwt Key"),
	AP_INIT_TAKE1("MyDongleCloudModule", moduleNameSet, NULL, RSRC_CONF | ACCESS_CONF, "MyDongleCloud module name"),
	AP_INIT_ITERATE("MyDongleCloudModulePermission", modulePermissionSet, NULL, RSRC_CONF | ACCESS_CONF, "Permission for MyDongleCloud module"),
	{NULL}
};

static void registerHooks(apr_pool_t *p) {
	ap_hook_access_checker(authorization, NULL, NULL, APR_HOOK_FIRST);
}

module AP_MODULE_DECLARE_DATA mydonglecloud_module = {
	STANDARD20_MODULE_STUFF, NULL, NULL, createConfig, mergeConfig, directives, registerHooks
};
