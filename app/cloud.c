#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "macro.h"
#include "cJSON.h"
#include "json.h"
#include "modules.h"
#include "logic.h"
#include "wifi.h"
#include "common.h"
#include "communication.h"
#include "language.h"

//Functions
void cloudInit() {
	cJSON *cloud = jsonRead(ADMIN_PATH "_config_/_cloud_.json");
	modulesInit(cloud);
	cJSON_Delete(cloud);
}

static void setup(int i, int total, char *name) {
	logicSetup(name, MAX2(1, i * 100 / total));
	char sz[256];
	snprintf(sz, sizeof(sz), "{\"status\":1, \"name\":%s}", name);
	communicationString(sz);
	snprintf(sz, sizeof(sz), "sudo /usr/local/modules/mydonglecloud/setup.sh %s", name);
	system(sz);
}

void cloudSetup(cJSON *el) {
	logicSetup(L("Initialization"), 0);
	cJSON *elWifi = cJSON_GetObjectItem(el, "wifi");
	if (elWifi && cJSON_GetStringValue2(elWifi, "ssid") && cJSON_GetStringValue2(elWifi, "password"))
		wiFiAddActivate(cJSON_GetStringValue2(elWifi, "ssid"), cJSON_GetStringValue2(elWifi, "password"));
	jsonWrite(cJSON_GetObjectItem(el, "cloud"), ADMIN_PATH "_config_/_cloud_.json");
	cJSON *elLetsencrypt = cJSON_GetObjectItem(el, "letsencrypt");
	if (elLetsencrypt) {
		mkdir(ADMIN_PATH "letsencrypt", 0775);
		FILE *fpC = fopen(ADMIN_PATH "letsencrypt/fullchain.pem", "w");
		if (fpC) {
			fwrite(cJSON_GetStringValue2(elLetsencrypt, "fullchain"), strlen(cJSON_GetStringValue2(elLetsencrypt, "fullchain")), 1, fpC);
			fclose(fpC);
		}
		FILE *fpK = fopen(ADMIN_PATH "letsencrypt/privkey.pem", "w");
		if (fpK) {
			fwrite(cJSON_GetStringValue2(elLetsencrypt, "privatekey"), strlen(cJSON_GetStringValue2(elLetsencrypt, "privatekey")), 1, fpK);
			fclose(fpK);
		}
	}
	cJSON * elBetterauth = cJSON_GetObjectItem(el, "betterauth");
	char buf[1024];
	char *post = cJSON_Print(elBetterauth);
	char szPath[17];
	generateUniqueId(szPath);
	memcpy(szPath, "/tmp/cookie", strlen("/tmp/cookie"));
	downloadURLBuffer("http://localhost:8091/MyDongleCloud/Auth/sign-up/email", buf, "Content-Type: application/json", post, NULL, szPath);
#ifdef DEFAULT_2FA
	downloadURLBuffer("http://localhost:8091/MyDongleCloud/Auth/two-factor/enable", buf, "Content-Type: application/json", post, szPath, NULL);
#endif
	unlink(szPath);
	free(post);
	serviceAction("betterauth.service", "RestartUnit");
	cJSON *modulesDefault = jsonRead(LOCAL_PATH "mydonglecloud/modulesdefault.json");
	cJSON *elModule;
	int i = 1;
	int total = cJSON_GetArraySize(modulesDefault);
	cJSON_ArrayForEach(elModule, modulesDefault)
		if (cJSON_HasObjectItem(elModule, "reset") && cJSON_HasObjectItem(elModule, "resetPriority"))
			setup(i++, total, elModule->string);
	cJSON_ArrayForEach(elModule, modulesDefault) {
		if (cJSON_HasObjectItem(elModule, "reset") && !cJSON_HasObjectItem(elModule, "resetPriority"))
			setup(i, total, elModule->string);
		i++;
	} 
	logicSetup(L("Finalization"), 100);
	communicationString("{\"status\":2}");
	cloudInit();
	logicMessage(1, 1);
	communicationString("{\"status\":3}");
	sync();
	jingle();
}
