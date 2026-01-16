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
#include "apache2.h"

//Private variables
static pthread_mutex_t cloudMutex = PTHREAD_MUTEX_INITIALIZER;
static int inSetup = 0;

//Functions
static void cloudInit_() {
	pthread_mutex_lock(&cloudMutex);
	PRINTF("CloudInit_\n");
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

void cloudInit() {
	if (inSetup)
		return;
	cloudInit_();
}

static void setup(int i, int total, char *name, int root, cJSON *modules) {
	int p = RANGE(1, 99, i * 100 / total);
	logicSetup(name, p);
	char sz[256];
	snprintf(sz, sizeof(sz), "{ \"a\":\"status\", \"progress\":%d, \"module\":\"%s\", \"state\":\"start\" }", p, name);
	communicationString(sz);
	snprintf(sz, sizeof(sz), "sudo /usr/local/modules/mydonglecloud/reset.sh -u %d %s", root, name);
	system(sz);
	cJSON *el = cJSON_CreateObject();
	cJSON_AddBoolToObject(el, "setupDone", 1);
	cJSON_AddItemToObject(modules, name, el);
	jsonWrite(modules, ADMIN_PATH "_config_/_modules_.json");
}

void setupLoop(int *i, int total, cJSON *cloud, cJSON *modulesDefault, cJSON *modules, cJSON *fqdn, int priority) {
	cJSON *elModule;
	cJSON_ArrayForEach(elModule, modulesDefault)
		if (cJSON_HasObjectItem(elModule, "setup") && cJSON_HasObjectItem(elModule, "setupPriority") == priority) {
			cJSON *elModuleDepString;
			cJSON_ArrayForEach(elModuleDepString, cJSON_GetObjectItem(elModule, "setupDependencies")) {
				cJSON *elModuleDep = cJSON_GetObjectItem(modulesDefault, cJSON_GetStringValue(elModuleDepString));
				if (cJSON_HasObjectItem(elModuleDep, "setup")) {
					int setupAlreadyDone = cJSON_HasObjectItem(modules, elModuleDep->string) && cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(modules, elModuleDep->string), "setupDone"));
					if (setupAlreadyDone == 0) {
						setup((*i)++, total, elModuleDep->string, cJSON_HasObjectItem(elModuleDep, "setupRoot"), modules);
						buildApache2Conf(cloud, modulesDefault, modules, fqdn);
#ifndef DESKTOP
						serviceAction("apache2.service", "ReloadOrRestartUnit");
#endif
					}
				}
			}
			int setupAlreadyDone = cJSON_HasObjectItem(modules, elModule->string) && cJSON_IsTrue(cJSON_GetObjectItem(cJSON_GetObjectItem(modules, elModule->string), "setupDone"));
			if (setupAlreadyDone == 0) {
				setup((*i)++, total, elModule->string, cJSON_HasObjectItem(elModule, "setupRoot"), modules);
				buildApache2Conf(cloud, modulesDefault, modules, fqdn);
#ifndef DESKTOP
				serviceAction("apache2.service", "ReloadOrRestartUnit");
#endif
			}
		}
}

void cloudSetup(cJSON *el) {
	if (inSetup) {
		PRINTF("cloudSetup not run because inSetup\n");
		return;
	}
	cJSON *elCheck = jsonRead(ADMIN_PATH "_config_/_cloud_.json");
	if (cJSON_GetArraySize(elCheck) > 0) {
		PRINTF("cloudSetup not run because already setup\n");
		cJSON_Delete(elCheck);
		return;
	}
	cJSON_Delete(elCheck);
	inSetup = 1;
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
	cJSON *fqdn = fqdnInit(elCloud);
	setupLoop(&i, total, elCloud, modulesDefault, modules, fqdn, 1);
	PRINTF("Saving(1) _modules_.json during setup\n");
	jsonWrite(modules, ADMIN_PATH "_config_/_modules_.json");
	cloudInit_();
	buzzer(1);
	setupLoop(&i, total, elCloud, modulesDefault, modules, fqdn, 0);
	logicSetup(L("Finalization"), 100);
	communicationString("{ \"a\":\"status\", \"progress\":100, \"module\":\"_setup_\", \"state\":\"finish\" }");
	cJSON_Delete(fqdn);
	cJSON_Delete(modulesDefault);
	PRINTF("Saving(2) _modules_.json during setup\n");
	jsonWrite(modules, ADMIN_PATH "_config_/_modules_.json");
	cJSON_Delete(modules);
	logicMessage(1, 1);
	jingle();
	sync();
	inSetup = 0;
}
