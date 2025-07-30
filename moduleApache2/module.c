#include <stdlib.h>
#include <stdio.h>
#include "util_cookies.h"
#include "http_log.h"
#include "apr_strings.h"
#include "macro.h"
#include "cJSON.h"

//Struct
typedef struct {
	const char *name;
} moduleNameConfig;

//Define
#undef PRINTF_
#undef PRINTF
#define PRINTF_(level, format, ...) {ap_log_error(APLOG_MARK, level, 0, s, format, ##__VA_ARGS__);}
#define PRINTF(format, ...) PRINTF_(APLOG_INFO, format, ##__VA_ARGS__)

//Private variable
cJSON *users = NULL;

//Functions
module AP_MODULE_DECLARE_DATA mydonglecloud_module;

static void usersLoad() {
	FILE *f = fopen(ADMIN_PATH "MyDongleCloud/users.json", "r");
	if (f) {
		char sz[1024];
		fread(sz, sizeof(sz), 1, f);
		users = cJSON_Parse(sz);
		fclose(f);
	}
}

static void *createDirConfig(apr_pool_t *p, char *dirspec) {
	moduleNameConfig *conf = apr_pcalloc(p, sizeof(moduleNameConfig));
	conf->name = NULL;
	return conf;
}

static const char *moduleNameSet(cmd_parms *cmd, void *mconfig, const char *arg) {
	server_rec *s = cmd->server;
    moduleNameConfig *conf = (moduleNameConfig *)mconfig;
    conf->name = arg;
	PRINTF("MDC: Set %s", arg);
    return NULL;
}

static int authorizationChecker(request_rec *r) {
	server_rec *s = r->server;
	const char *cookie_header = apr_table_get(r->headers_in, "Cookie");
    moduleNameConfig *conf = ap_get_module_config(r->per_dir_config, &mydonglecloud_module);
	PRINTF("MDC: authorizationChecker (%s)", conf->name);
	return DECLINED;
}

static const command_rec directives[] = {
	AP_INIT_TAKE1("MyDongleCloudModule", moduleNameSet, NULL, RSRC_CONF | ACCESS_CONF, "MyDongleCloud module name"),
	{NULL}
};

static void registerHooks(apr_pool_t *p) {
	ap_hook_handler(authorizationChecker, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA mydonglecloud_module = {
	STANDARD20_MODULE_STUFF, createDirConfig, NULL, NULL, NULL, directives, registerHooks
};
