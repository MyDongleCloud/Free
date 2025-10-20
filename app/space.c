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

//Private variable
#define RESETS 10
static char *szResets[RESETS] = { "postfix", "roundcube", "mysql", "bugzilla", "jitsi", "mantisbugtracker", "osticket", "projectsend", "webtrees", "yourls" };

//Functions
void spaceInit() {
	cJSON *space = jsonRead(ADMIN_PATH "mydonglecloud/space.json");
	modulesSetup(space);
	cJSON_Delete(space);
}

void spaceSetup(cJSON *el) {
	logicMessage("Setup is starting. Please wait...", 0);
	if (cJSON_GetStringValue2(el, "ssid") && cJSON_GetStringValue2(el, "security"))
		wiFiAddActivate(cJSON_GetStringValue2(el, "ssid"), cJSON_GetStringValue2(el, "security"));
	jsonWrite(cJSON_GetObjectItem(el, "space"), ADMIN_PATH "mydonglecloud/space.json");
	jsonWrite(cJSON_GetObjectItem(el, "proxy"), ADMIN_PATH "mydonglecloud/proxy.json");
	mkdir(ADMIN_PATH "letsencrypt", 0775);
	FILE *fpC = fopen(ADMIN_PATH "letsencrypt/fullchain.pem", "w");
	if (fpC) {
		fwrite(cJSON_GetStringValue2(el, "fullchain"), strlen(cJSON_GetStringValue2(el, "fullchain")), 1, fpC);
		fclose(fpC);
	}
	FILE *fpK = fopen(ADMIN_PATH "letsencrypt/privkey.pem", "w");
	if (fpK) {
		fwrite(cJSON_GetStringValue2(el, "privatekey"), strlen(cJSON_GetStringValue2(el, "privatekey")), 1, fpK);
		fclose(fpK);
	}
	cJSON *data = cJSON_CreateObject();
	cJSON_AddStringToObject(data, "email", cJSON_GetStringValue2(el, "email"));
	cJSON_AddStringToObject(data, "name", cJSON_GetStringValue2(el, "name"));
	cJSON_AddStringToObject(data, "password", cJSON_GetStringValue2(el, "password"));
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
	char sz[256];
	for (int i = 0; i < RESETS; i++) {
		snprintf(sz, sizeof(sz), "Setting is configuring\n%s\n%d/%d\nPlease wait...", szResets[i] + 1, i + 1, RESETS);
		logicMessage(sz, 0);
		snprintf(sz, sizeof(sz), "{\"status\":1, \"name\":%s}", szResets[i]);
		communicationString(sz);
		snprintf(sz, sizeof(sz), "sudo /usr/local/modules/mydonglecloud/setup.sh %s", szResets[i]);
		system(sz);
	}
	logicMessage("Setup is finishing. Please wait...", 0);
	communicationString("{\"status\":2}");
	serviceAction("dovecot.service", "RestartUnit");
	spaceInit();
	logicMessage("Congratulations! MyDongle is now ready", 1);
	communicationString("{\"status\":3}");
	jingle();
}
