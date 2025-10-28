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
void spaceInit() {
	cJSON *space = jsonRead(ADMIN_PATH "mydonglecloud/space.json");
	modulesInit(space);
	cJSON_Delete(space);
}

static void setup(int i, int total, char *name) {
	logicSetup(name, MAX2(1, i * 100 / total));
	char sz[256];
	snprintf(sz, sizeof(sz), "{\"status\":1, \"name\":%s}", name);
	communicationString(sz);
	snprintf(sz, sizeof(sz), "sudo /usr/local/modules/mydonglecloud/setup.sh %s", name);
	system(sz);
}

void spaceSetup(cJSON *elSpace) {
	logicSetup(L("Initialization"), 0);
	if (cJSON_GetStringValue2(elSpace, "ssid") && cJSON_GetStringValue2(elSpace, "security"))
		wiFiAddActivate(cJSON_GetStringValue2(elSpace, "ssid"), cJSON_GetStringValue2(elSpace, "security"));
	jsonWrite(cJSON_GetObjectItem(elSpace, "space"), ADMIN_PATH "mydonglecloud/space.json");
	jsonWrite(cJSON_GetObjectItem(elSpace, "proxy"), ADMIN_PATH "mydonglecloud/proxy.json");
	mkdir(ADMIN_PATH "_config_", 0775);
	mkdir(ADMIN_PATH "letsencrypt", 0775);
	FILE *fpC = fopen(ADMIN_PATH "letsencrypt/fullchain.pem", "w");
	if (fpC) {
		fwrite(cJSON_GetStringValue2(elSpace, "fullchain"), strlen(cJSON_GetStringValue2(elSpace, "fullchain")), 1, fpC);
		fclose(fpC);
	}
	FILE *fpK = fopen(ADMIN_PATH "letsencrypt/privkey.pem", "w");
	if (fpK) {
		fwrite(cJSON_GetStringValue2(elSpace, "privatekey"), strlen(cJSON_GetStringValue2(elSpace, "privatekey")), 1, fpK);
		fclose(fpK);
	}
	cJSON *data = cJSON_CreateObject();
	cJSON_AddStringToObject(data, "email", cJSON_GetStringValue2(elSpace, "email"));
	cJSON_AddStringToObject(data, "name", cJSON_GetStringValue2(elSpace, "name"));
	cJSON_AddStringToObject(data, "password", cJSON_GetStringValue2(elSpace, "password"));
	char buf[1024];
	char *post = cJSON_Print(data);
	char szPath[17];
	generateUniqueId(szPath);
	memcpy(szPath, "/tmp/cookie", strlen("/tmp/cookie"));
	downloadURLBuffer("http://localhost:8091/MyDongleCloud/Auth/sign-up/email", buf, "Content-Type: application/json", post, NULL, szPath);
#ifdef DEFAULT_2FA
	downloadURLBuffer("http://localhost:8091/MyDongleCloud/Auth/two-factor/enable", buf, "Content-Type: application/json", post, szPath, NULL);
#endif
	unlink(szPath);
	free(post);
	cJSON_Delete(data);
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
	spaceInit();
	logicMessage(1, 1);
	communicationString("{\"status\":3}");
	jingle();
}
