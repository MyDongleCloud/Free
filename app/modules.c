#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"
#include "macro.h"
#include "cJSON.h"
#include "json.h"
#include "apache2.h"

//Functions


void modulesSetup(cJSON *space) {
	char sz[256];
	cJSON *fqdn = cJSON_CreateArray();
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(space, "name"), MAIN_DOMAIN);
	cJSON *s = NULL;
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(space, "alias"), SHORT_DOMAIN);
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	cJSON *ss = NULL;
	cJSON_ArrayForEach(ss, cJSON_GetObjectItem(space, "domains")) {
		s = cJSON_CreateString(ss->valuestring);
		cJSON_AddItemToArray(fqdn, s);
	}
	cJSON *modulesDefault = jsonRead(LOCAL_PATH "MyDongleCloud/modulesDefault.json");
	cJSON *modules = jsonRead(ADMIN_PATH "MyDongleCloud/modules.json");
	if (modules == NULL)
		modules = cJSON_CreateObject();
	int firstTime = !cJSON_IsTrue(cJSON_GetObjectItem(modules, "initDone"));

	for (int i = 0; i < cJSON_GetArraySize(modulesDefault); i++) {
		cJSON *elModule = cJSON_GetArrayItem(modulesDefault, i);
		cJSON *elModule2 = cJSON_GetObjectItem(modules, elModule->string);

		if (strcmp(elModule->string, "Apache2") == 0) {
			if (cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "overwrite"))) {
				PRINTF("Apache2: No creation of main.conf\n");
			} else
				buildApache2Conf(modulesDefault, modules, space, fqdn);
#ifndef DESKTOP
			reloadApache2Conf();
#endif
		} else if (strcmp(elModule->string, "frp") == 0) {
			PRINTF("Modules:frp: Enter\n");
			int	used = 0;
			int port = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "bindingPort"));
			cJSON *elModuleS = cJSON_GetObjectItem(elModule, "services");
			cJSON *elModule2Proxy = jsonRead(ADMIN_PATH "MyDongleCloud/proxy.json");
			if (elModule2Proxy == NULL)
				elModule2Proxy = cJSON_CreateObject();
			elModule2 = cJSON_GetObjectItem(elModule2Proxy, "frp");
			cJSON *elModule2S = cJSON_GetObjectItem(elModule2, "services");
			mkdir(ADMIN_PATH "frp", 0775);
#ifdef DESKTOP
			FILE *pf = fopen("/tmp/frpc.toml", "w");
#else
			FILE *pf = fopen(ADMIN_PATH "frp/frpc.toml", "w");
#endif
			if (pf) {
				char token[64];
				token[0] = '\0';
				FILE *pft = fopen(LOCAL_PATH "frp/password-token.txt", "r");
				if (pft) {
					int ret = fread(token, 1, 64, pft);
					if (ret >= 0)
						token[ret] = '\0';
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
webServer.port = 7400\n\n", port, token, cJSON_GetStringValue2(space, "name"), cJSON_GetStringValue2(elModule2, "token"));
				fwrite(sz, strlen(sz), 1, pf);
				for (int j = 0; j < cJSON_GetArraySize(elModuleS); j++) {
					cJSON *elModuleSj = cJSON_GetArrayItem(elModuleS, j);
					cJSON *elModule2Sj = cJSON_GetObjectItem(elModule2S, elModuleSj->string);
					if (elModule2Sj && cJSON_IsTrue(cJSON_GetObjectItem(elModule2Sj, "enabled"))) {
						used = 1;
						int localPort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModuleSj, "localPort"));
						if (strncmp(elModuleSj->string, "http", 4) == 0) {
							sprintf(sz, "\
[[proxies]]\n\
name = \"%s\"\n\
type = \"%s\"\n\
localIP = \"localhost\"\n\
localPort = %d\n\
customDomains = [\n", elModuleSj->string, elModuleSj->string, elModuleSj->string[4] == 's' ? 443 : 80);
							fwrite(sz, strlen(sz), 1, pf);
							jsonPrintArray(1, "\"", "\"", "", fqdn, "\",\n", pf);
							jsonPrintArray(1, "\"", "\"", "*", fqdn, "\",\n", pf);
							strcpy(sz, "]\n\n");
							fwrite(sz, strlen(sz), 1, pf);
						} else {
							int remotePort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule2Sj, "remotePort"));
							snprintf(sz, sizeof(sz), "\
[[proxies]]\n\
name = \"%s\"\n\
type = \"tcp\"\n\
localIP = \"localhost\"\n\
localPort = %d\n\
remotePort = %d\n\n", elModuleSj->string, localPort, remotePort);
							fwrite(sz, strlen(sz), 1, pf);
//PRINTF("https://mydongle.cloud/master/proxy.json { \"localPort\": %d, \"serviceName\": \"%s\", \"spaceName\": \"%s\", \"remotePort\": %d }\n", localPort, elModuleSj->string, cJSON_GetStringValue2(space, "name"), remotePort);
						}
					}
				}
				fclose(pf);
			}
#ifndef DESKTOP
			if (used && getuid() == 1001) {
				PRINTF("Modules:frp: Starting service (user=admin)\n");
				system("sudo /usr/bin/systemctl start frp.service");
			}
#endif
		} else if (strcmp(elModule->string, "JitsiMeet") == 0) {
			if (firstTime)
				;//system("find /etc -exec sed -i -e \"s/m_unique_d_unique_c/${SPACE}/\" {} \;");
		} else if (strcmp(elModule->string, "osTicket") == 0) {
#ifndef DESKTOP
			if (!fileExists(ADMIN_PATH "osTicket/ost-config.php")) {
				PRINTF("osTicket: Creation of ost-config.php\n");
				mkdir(ADMIN_PATH "osTicket", 0775);
				if (!fileExists(ADMIN_PATH "osTicket/ost-config.php")) {
					copyFile(LOCAL_PATH "osTicket/include/ost-sampleconfig.php", ADMIN_PATH "osTicket/ost-config.php", NULL);
					chmod(ADMIN_PATH "osTicket/ost-config.php", 0666);
				}
			}
#endif
		} else if (strcmp(elModule->string, "OTG") == 0) {
#ifndef DESKTOP
			;
#endif
		} else if (strcmp(elModule->string, "Postfix") == 0) {
			//In /etc/mailname mail.m_unique_d_unique_c.mydongle.cloud
			//In etc/postfix/main.cf mydestination = $myhostname, mail.m_unique_d_unique_c.mydongle.cloud, mydonglecloud, localhost.localdomain, localhost
		}
	}

	if (firstTime)
		cJSON_AddBoolToObject(modules, "initDone", cJSON_True);
#ifndef DESKTOP
	jsonWrite(modules, ADMIN_PATH "MyDongleCloud/modules.json");
#endif
	cJSON_Delete(fqdn);
	cJSON_Delete(modules);
	cJSON_Delete(modulesDefault);
}
