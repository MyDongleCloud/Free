#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "util_cookies.h"
#include "http_log.h"
#include "apr_strings.h"
#include "macro.h"
#include "cJSON.h"

//Struct
typedef struct {
	const char *name;
	cJSON *authorized;
} config;

//Defines
#undef PRINTF_
#undef PRINTF
#define PRINTF_(level, format, ...) {ap_log_error(APLOG_MARK, level, 0, s, format, ##__VA_ARGS__);}
#define PRINTF(format, ...) PRINTF_(APLOG_INFO, format, ##__VA_ARGS__)

//Functions
module AP_MODULE_DECLARE_DATA mydonglecloud_module;

static cJSON *usersLoad() {
	struct stat statTest;
	if (stat(ADMIN_PATH "MyDongleCloud/users.json", &statTest) != 0 || statTest.st_size == 0)
		return NULL;
	int size = statTest.st_size + 16;
	char *sz = malloc(size);
	FILE *f = fopen(ADMIN_PATH "MyDongleCloud/users.json", "r");
	cJSON *users = NULL;
	if (f) {
		int ret = fread(sz, size, 1, f);
		users = cJSON_Parse(sz);
		fclose(f);
	}
	free(sz);
	return users;
}

static void *createConfig(apr_pool_t *p, server_rec *s) {
	config *confD = apr_pcalloc(p, sizeof(config));
	confD->name = NULL;
	confD->authorized = NULL;
	return confD;
}

static const char *moduleNameSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	config *confD = (config *)ap_get_module_config(s->module_config, &mydonglecloud_module);
	confD->name = arg;
	PRINTF("MDC: Set %s", arg);
	return NULL;
}

static const char *moduleAuthorizedSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
	config *confD = (config *)ap_get_module_config(s->module_config, &mydonglecloud_module);
	PRINTF("MDC: Add user %s for %s", arg, confD->name);
	if (confD->authorized == NULL)
		confD->authorized = cJSON_CreateObject();
	cJSON_AddBoolToObject(confD->authorized, arg, cJSON_True);
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

static int authorization(request_rec *r) {
	server_rec *s = r->server;
	config *confD = (config *)ap_get_module_config(s->module_config, &mydonglecloud_module);
	//PRINTF("MDC: authorization1 name:%s", confD->name);
	if (confD->name == NULL || confD->authorized == NULL)
		return DECLINED;
	const char *cookies = apr_table_get(r->headers_in, "Cookie");
	char *cookieUser = extractCookieValue(cookies, "user", r);
	//PRINTF("MDC: authorization2 cookieUser:%s", cookieUser);
	if (cookieUser != NULL) {
		if (cJSON_HasObjectItem(confD->authorized, "_allusers_") || cJSON_HasObjectItem(confD->authorized, cookieUser)) {
			char *cookieToken = extractCookieValue(cookies, "token", r);
			//PRINTF("MDC: authorization3 cookieToken:%s", cookieToken);
			if (cookieToken != NULL) {
				cJSON *users = usersLoad(s);
				if (users != NULL) {
					cJSON *el = cJSON_GetObjectItem(users, cookieUser);
					if (el) {
						cJSON *el2 = cJSON_GetObjectItem(el, "token");
						if (el2) {
							char *st = cJSON_GetStringValue(el2);
							//PRINTF("MDC: authorization4 userToken:%s", st);
							if (st != NULL && strcmp(cookieToken, st) == 0) {
								cJSON_Delete(users);
								return DECLINED;
							}
						}
					}
					cJSON_Delete(users);
				}
			}
		}
	}
	return HTTP_UNAUTHORIZED;
}

static const command_rec directives[] = {
	AP_INIT_TAKE1("MyDongleCloudModule", moduleNameSet, NULL, RSRC_CONF | ACCESS_CONF, "MyDongleCloud module name"),
	AP_INIT_ITERATE("MyDongleCloudModuleAuthorized", moduleAuthorizedSet, NULL, RSRC_CONF | ACCESS_CONF, "Authorized users list for MyDongleCloud module"),
	{NULL}
};

static void registerHooks(apr_pool_t *p) {
	ap_hook_handler(authorization, NULL, NULL, APR_HOOK_FIRST);
}

module AP_MODULE_DECLARE_DATA mydonglecloud_module = {
	STANDARD20_MODULE_STUFF, NULL, NULL, createConfig, NULL, directives, registerHooks
};
