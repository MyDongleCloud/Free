#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
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

//Private variable
static pthread_mutex_t cloudMutex = PTHREAD_MUTEX_INITIALIZER;

//Functions
void cloudInit() {
	pthread_mutex_lock(&cloudMutex);
	PRINTF("CloudInit\n");
	cJSON *cloud = jsonRead(ADMIN_PATH "_config_/_cloud_.json");
	if (cloud == NULL)
		cloud = cJSON_CreateObject();
	cJSON *modulesDefault = jsonRead(LOCAL_PATH "mydonglecloud/modulesdefault.json");
	cJSON *modules = jsonRead(ADMIN_PATH "_config_/_modules_.json");
	if (modules == NULL)
		modules = cJSON_CreateObject();
	modulesInit(cloud, modulesDefault, modules);
	cJSON_Delete(cloud);
	cJSON_Delete(modules);
	cJSON_Delete(modulesDefault);
	pthread_mutex_unlock(&cloudMutex);
}

static void setup(int i, int total, char *name, int root, cJSON *modules) {
	logicSetup(name, RANGE(1, 99, i * 100 / total));
	char sz[256];
	snprintf(sz, sizeof(sz), "{\"status\":1, \"name\":\"%s\"}", name);
	communicationString(sz);
	snprintf(sz, sizeof(sz), "sudo /usr/local/modules/mydonglecloud/setup.sh -j 0 -d 0 -u %d -r %s", root, name);
	system(sz);
	cJSON *el = cJSON_CreateObject();
	cJSON_AddBoolToObject(el, "setupDone", 1);
	cJSON_AddItemToObject(modules, name, el);
}

void setupLoop(int *i, int total, cJSON *modulesDefault, cJSON *modules, int priority) {
	cJSON *elModule;
	cJSON_ArrayForEach(elModule, modulesDefault)
		if (cJSON_HasObjectItem(elModule, "setup") && cJSON_HasObjectItem(elModule, "setupPriority") == priority) {
			cJSON *elModuleDepString;
			cJSON_ArrayForEach(elModuleDepString, cJSON_GetObjectItem(elModule, "setupDependencies")) {
				cJSON *elModuleDep = cJSON_GetObjectItem(modulesDefault, cJSON_GetStringValue(elModuleDepString));
				if (cJSON_HasObjectItem(elModuleDep, "setup")) {
					int setupAlreadyDone = cJSON_HasObjectItem(modules, elModuleDep->string) && cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(modules, elModuleDep->string), "setupDone"));
					if (setupAlreadyDone == 0)
						setup((*i)++, total, elModuleDep->string, cJSON_HasObjectItem(elModuleDep, "setupRoot"), modules);
				}
			}
			int setupAlreadyDone = cJSON_HasObjectItem(modules, elModule->string) && cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(modules, elModule->string), "setupDone"));
			if (setupAlreadyDone == 0)
				setup((*i)++, total, elModule->string, cJSON_HasObjectItem(elModule, "setupRoot"), modules);
		}
}

void cloudSetup(cJSON *el) {
	logicSetup(L("Initialization"), 0);
	cJSON *elCloud = cJSON_GetObjectItem(el, "cloud");
	cJSON *elSecurity = cJSON_GetObjectItem(elCloud, "security");
	cJSON *elConnectivity = cJSON_GetObjectItem(elCloud, "connectivity");
	cJSON *elWifi = cJSON_GetObjectItem(elConnectivity, "wifi");
	if (elWifi && cJSON_GetStringValue2(elWifi, "ssid") && cJSON_GetStringValue2(elWifi, "password"))
		PRINTF("Setup Wi-Fi with %s %s\n", cJSON_GetStringValue2(elWifi, "ssid"), cJSON_GetStringValue2(elWifi, "password"));//wiFiAddActivate
	jsonWrite(elCloud, ADMIN_PATH "_config_/_cloud_.json");
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
	downloadURLBuffer("http://localhost:8091/auth/sign-up/email", buf, "Content-Type: application/json", post, NULL, NULL);
	free(post);
	serviceAction("betterauth.service", "RestartUnit");
	cJSON *cloud = jsonRead(ADMIN_PATH "_config_/_cloud_.json");
	cJSON *modulesDefault = jsonRead(LOCAL_PATH "mydonglecloud/modulesdefault.json");
	cJSON *modules = jsonRead(ADMIN_PATH "_config_/_modules_.json");
	if (modules == NULL)
		modules = cJSON_CreateObject();
	cJSON *elModule;
	int total = 1;
	cJSON_ArrayForEach(elModule, modulesDefault)
		if (cJSON_IsTrue(cJSON_GetObjectItem(elModule, "setup")))
			total++;
	int i = 1;
	setupLoop(&i, total, modulesDefault, modules, 1);
	PRINTF("Saving(1) _modules_.json during setup\n");
	jsonWrite(modules, ADMIN_PATH "_config_/_modules_.json");
	communicationString("{\"status\":2}");
	buzzer(1);
	setupLoop(&i, total, modulesDefault, modules, 0);
	logicSetup(L("Finalization"), 100);
	communicationString("{\"status\":3}");
	cJSON_Delete(cloud);
	cJSON_Delete(modulesDefault);
	PRINTF("Saving(2) _modules_.json during setup\n");
	jsonWrite(modules, ADMIN_PATH "_config_/_modules_.json");
	cJSON_Delete(modules);
	logicMessage(1, 1);
	jingle();
	sync();
}
