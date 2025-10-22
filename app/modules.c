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
#include "dbBerkeley.h"

//Functions
void modulesInit(cJSON *space) {
	PRINTF("Modules:Setup: Enter\n");
	if (space == NULL || !cJSON_HasObjectItem(space, "name"))
		return;
	PRINTF("Modules:Setup: Do\n");
	char sz[256];
	cJSON *fqdn = cJSON_CreateArray();
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(space, "name"), MAIN_DOMAIN);
	cJSON *s = NULL;
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(space, "name"), MAIN_DOMAIN_FR);
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	snprintf(sz, sizeof(sz), "%s.%s", cJSON_GetStringValue2(space, "shortname"), SHORT_DOMAIN);
	s = cJSON_CreateString(sz);
	cJSON_AddItemToArray(fqdn, s);
	cJSON *ss = NULL;
	cJSON_ArrayForEach(ss, cJSON_GetObjectItem(space, "domains")) {
		s = cJSON_CreateString(ss->valuestring);
		cJSON_AddItemToArray(fqdn, s);
	}
	cJSON *modulesDefault = jsonRead(LOCAL_PATH "mydonglecloud/modulesdefault.json");
	cJSON *modules = jsonRead(ADMIN_PATH "mydonglecloud/modules.json");
	if (modules == NULL)
		modules = cJSON_CreateObject();
	for (int i = 0; i < cJSON_GetArraySize(modulesDefault); i++) {
		cJSON *elModule = cJSON_GetArrayItem(modulesDefault, i);
		cJSON *elModule2 = cJSON_GetObjectItem(modules, elModule->string);

		if (strcmp(elModule->string, "apache2") == 0) {
			mkdir(ADMIN_PATH "apache2", 0775);
			if (cJSON_IsTrue(cJSON_GetObjectItem(elModule2, "overwrite"))) {
				PRINTF("Apache2: No creation of main.conf\n");
			} else
				buildApache2Conf(modulesDefault, modules, space, fqdn);
#ifndef DESKTOP
			serviceAction("apache2.service", "ReloadOrRestartUnit");
#endif
		} else if (strcmp(elModule->string, "frp") == 0) {
			PRINTF("Modules:frp: Enter\n");
			int	used = 0;
			int port = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule, "bindingPort"));
			cJSON *elModuleS = cJSON_GetObjectItem(elModule, "services");
			cJSON *elModule2Proxy = jsonRead(ADMIN_PATH "mydonglecloud/proxy.json");
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
				FILE *pft = fopen(LOCAL_PATH "mydonglecloud/proxy-token.txt", "r");
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
						char *type = cJSON_GetStringValue2(elModuleSj, "type");
						int localPort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModuleSj, "localPort"));
						int remotePort = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(elModule2Sj, "remotePort"));
						sprintf(sz, "\
[[proxies]]\n\
name = \"%s\"\n\
type = \"%s\"\n%s\
localIP = \"localhost\"\n\
localPort = %d\n", elModuleSj->string, type, strncmp(type, "http", 4) == 0 ? "transport.proxyProtocolVersion = \"v2\"\n"  : "", localPort);
						fwrite(sz, strlen(sz), 1, pf);
						if (strncmp(type, "http", 4) == 0) {
							strcpy(sz, "customDomains = [\n");
							fwrite(sz, strlen(sz), 1, pf);
							jsonPrintArray(1, "\"", "\"", "", fqdn, "\",\n", pf);
							jsonPrintArray(1, "\"", "\"", "*", fqdn, "\",\n", pf);
							strcpy(sz, "]\n\n");
							fwrite(sz, strlen(sz), 1, pf);
						} else if (strcmp(type, "tcpmux") == 0) {
							snprintf(sz, sizeof(sz), "multiplexer = \"httpconnect\"\ncustomDomains = [ \"%s.%s\" ]\n\n", elModuleSj->string, cJSON_GetStringValue(cJSON_GetArrayItem(fqdn, 0)));
							fwrite(sz, strlen(sz), 1, pf);
						} else if (remotePort > 0) {
							snprintf(sz, sizeof(sz), "remotePort= %d\n\n", remotePort);
							fwrite(sz, strlen(sz), 1, pf);
						}
					}
				}
				fclose(pf);
			}
			cJSON_Delete(elModule2Proxy);
#ifndef DESKTOP
			serviceAction("frp.service", "RestartUnit");
#endif
		} else if (strcmp(elModule->string, "roundcube") == 0) {
#ifdef DESKTOP
			FILE *ipf = fopen("/tmp/config.inc.php.template", "r");
#else
			FILE *ipf = fopen("/etc/roundcube/config.inc.php.template", "r");
#endif
			if (ipf) {
#ifdef DESKTOP
				FILE *opf = fopen("/tmp/config.inc.php", "w");
#else
				FILE *opf = fopen("/etc/roundcube/config.inc.php", "w");
#endif
				if (opf) {
					char *line = NULL;
					size_t lineLen = 0;
					while (getline(&line, &lineLen, ipf) != -1) {
						if (strncmp(line, "$config['smtp_host']", strlen("$config['smtp_host']")) == 0) {
							char sz[] = "$config['smtp_host'] = 'ssl://localhost:465'; $config['smtp_conn_options'] = [ 'ssl' => [ 'verify_peer' => false, 'verify_peer_name' => false ] ];\n";
							fwrite(sz, strlen(sz), 1, opf);
						} else
							fwrite(line, strlen(line), 1, opf);
					}
					fclose(opf);
				}
				fclose(ipf);
			}
		} else if (strcmp(elModule->string, "postfix") == 0) {
			PRINTF("Modules:postfix: Enter\n");
			mkdir(ADMIN_PATH "mail", 0775);
#ifdef DESKTOP
			FILE *pf = fopen("/tmp/virtualhosts", "w");
#else
			FILE *pf = fopen(ADMIN_PATH "mail/virtualhosts", "w");
#endif
			if (pf) {
				jsonPrintArray(0, "", "", "", fqdn, "\n", pf);
				fclose(pf);
			}
#ifdef DESKTOP
			dbBerkeleyCreate(ADMIN_PATH "mail/virtualalias", "/tmp/virtualalias.db");
			dbBerkeleyCreate(ADMIN_PATH "mail/virtualmaps", "/tmp/virtualmaps.db");
#else
			dbBerkeleyCreate(ADMIN_PATH "mail/virtualalias", ADMIN_PATH "mail/virtualalias.db");
			dbBerkeleyCreate(ADMIN_PATH "mail/virtualmaps", ADMIN_PATH "mail/virtualmaps.db");
			serviceAction("postfix.service", "ReloadOrRestartUnit");
#endif
		}
	}

#ifndef DESKTOP
	jsonWrite(modules, ADMIN_PATH "mydonglecloud/modules.json");
#endif
	cJSON_Delete(fqdn);
	cJSON_Delete(modules);
	cJSON_Delete(modulesDefault);
}
