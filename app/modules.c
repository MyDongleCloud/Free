#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"
#include "macro.h"
#include "cJSON.h"
#include "json.h"
#include "apache2.h"

//Functions
void modulesInit(cJSON *elCloud) {
	PRINTF("Modules:Setup: Enter\n");
	if (elCloud == NULL || !cJSON_HasObjectItem(elCloud, "all"))
		return;
	PRINTF("Modules:Setup: Do\n");
	char sz[256];
	cJSON * elCloudAll = cJSON_GetObjectItem(elCloud, "all");
	cJSON *fqdn = cJSON_CreateArray();
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(elCloudAll, "name"), MAIN_DOMAIN);
	cJSON *s = NULL;
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(elCloudAll, "name"), MAIN_DOMAIN_FR);
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(elCloudAll, "shortname"), SHORT_DOMAIN);
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	cJSON *ss = NULL;
	cJSON_ArrayForEach(ss, cJSON_GetObjectItem(elCloud, "domains")) {
		s = cJSON_CreateString(ss->valuestring);
		cJSON_AddItemToArray(fqdn, s);
	}
	cJSON *modulesDefault = jsonRead(LOCAL_PATH "mydonglecloud/modulesdefault.json");
	cJSON *modules = jsonRead(ADMIN_PATH "_config_/_modules_.json");
	if (modules == NULL)
		modules = cJSON_CreateObject();

	cJSON *elModule, *elModule2;
	elModule = cJSON_GetObjectItem(modulesDefault, "apache2");
	elModule2 = cJSON_GetObjectItem(modules, "apache2");
	mkdir(ADMIN_PATH "apache2", 0775);
	if (cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "overwrite"))) {
		PRINTF("Apache2: No creation of main.conf\n");
	} else
		buildApache2Conf(modulesDefault, modules, fqdn);
#ifndef DESKTOP
	serviceAction("apache2.service", "ReloadOrRestartUnit");
#endif
	PRINTF("Modules:frp: Enter\n");
	elModule = cJSON_GetObjectItem(modulesDefault, "frp");
	elModule2 = cJSON_GetObjectItem(modules, "frp");
	cJSON *elModule3 = cJSON_GetObjectItem(elCloud, "frp");
	cJSON *elModuleS, *elModule2S, *elModule3S;
	if (elModule)
		elModuleS = cJSON_GetObjectItem(elModule, "services");
	if (elModule2)
		elModule2S = cJSON_GetObjectItem(elModule2, "services");
	if (elModule3)
		elModule3S = cJSON_GetObjectItem(elModule3, "services");
	int	used = 0;
	int port = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "bindingPort"));
	mkdir(ADMIN_PATH "frp", 0775);
#ifdef DESKTOP
	FILE *pf = fopen("/tmp/frpc.toml", "w");
#else
	FILE *pf = fopen(ADMIN_PATH "frp/frpc.toml", "w");
#endif
	if (pf) {
		char firstToken[64];
		firstToken[0] = '\0';
		FILE *pft = fopen(LOCAL_PATH "mydonglecloud/proxy-token.txt", "r");
		if (pft) {
			int ret = fread(firstToken, 1, 64, pft);
			if (ret >= 0)
				firstToken[ret] = '\0';
			fclose(pft);
		}
		char sz[2048];
		snprintf(sz, sizeof(sz), "\
serverAddr = \"server.mydongle.cloud\"\n\
serverPort = %d\n\
auth.method = \"token\"\n\
auth.token = \"%s\"\n\
user = \"%s\"\n\
metadatas.token = \"%s\"\n\
webServer.addr = \"127.0.0.1\"\n\
webServer.port = 7400\n\n", port, firstToken, cJSON_GetStringValue2(elCloudAll, "name"), cJSON_GetStringValue2(elModule3, "token"));
		fwrite(sz, strlen(sz), 1, pf);
		for (int t = 0; t < cJSON_GetArraySize(elModuleS); t++) {
			cJSON *elModuleSt = cJSON_GetArrayItem(elModuleS, t);
			cJSON *elModule2St = cJSON_GetObjectItem(elModule2S, elModuleSt->string);
			cJSON *elModule3St = cJSON_GetObjectItem(elModule3S, elModuleSt->string);
			int enabled = 0;
			if (cJSON_IsTrue(cJSON_GetObjectItem(elModule2St, "enabled")))
				enabled = 1;
			else if (cJSON_IsTrue(cJSON_GetObjectItem(elModuleSt, "enabled")))
				enabled = 1;
			if (enabled) {
				used = 1;
				char *type = cJSON_GetStringValue2(elModuleSt, "type");
				int localPort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModuleSt, "localPort"));
				int remotePort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule3St, "remotePort"));
				sprintf(sz, "\
[[proxies]]\n\
name = \"%s\"\n\
type = \"%s\"\n%s\
localIP = \"localhost\"\n\
localPort = %d\n", elModuleSt->string, type, strncmp(type, "http", 4) == 0 ? "transport.proxyProtocolVersion = \"v2\"\n"  : "", localPort);
				fwrite(sz, strlen(sz), 1, pf);
				if (strncmp(type, "http", 4) == 0) {
					strcpy(sz, "customDomains = [\n");
					fwrite(sz, strlen(sz), 1, pf);
					jsonPrintArray(1, "\"", "\"", "", fqdn, "\",\n", pf);
					jsonPrintArray(1, "\"", "\"", "*", fqdn, "\",\n", pf);
					strcpy(sz, "]\n\n");
					fwrite(sz, strlen(sz), 1, pf);
				} else if (strcmp(type, "tcpmux") == 0) {
					snprintf(sz, sizeof(sz), "multiplexer = \"httpconnect\"\ncustomDomains = [ \"%s.%s\" ]\n\n", elModuleSt->string, cJSON_GetStringValue(cJSON_GetArrayItem(fqdn, 0)));
					fwrite(sz, strlen(sz), 1, pf);
				} else if (remotePort > 0) {
					snprintf(sz, sizeof(sz), "remotePort= %d\n\n", remotePort);
					fwrite(sz, strlen(sz), 1, pf);
				}
			}
		}
		fclose(pf);
	}
#ifndef DESKTOP
	serviceAction("frp.service", "RestartUnit");
#endif

	cJSON_Delete(fqdn);
	cJSON_Delete(modules);
	cJSON_Delete(modulesDefault);
}
