#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "macro.h"
#include "settings.h"
#include "cJSON.h"

//Global variables
settings smdc;
int nameId;

//Functions
#if !defined(DESKTOP) && !defined(WEB)
void settingsFromJson(char *sz, settings *psmdc) {
	cJSON *root = cJSON_Parse(sz);
	cJSON *el;
	el = cJSON_GetObjectItem(root, "version"); if (el) psmdc->version = (int)cJSON_GetNumberValue(el);
	el = cJSON_GetObjectItem(root, "language"); if (el) psmdc->language = (int)cJSON_GetNumberValue(el);
	el = cJSON_GetObjectItem(root, "rotation"); if (el) psmdc->rotation = (int)cJSON_GetNumberValue(el);
	el = cJSON_GetObjectItem(root, "noBuzzer"); if (el) psmdc->noBuzzer = (int)cJSON_GetNumberValue(el);
	el = cJSON_GetObjectItem(root, "sleepKeepLed"); if (el) psmdc->sleepKeepLed = (int)cJSON_GetNumberValue(el);
	el = cJSON_GetObjectItem(root, "setupDone"); if (el) psmdc->setupDone = (int)cJSON_GetNumberValue(el);
	cJSON_free(root);
}

char *settingsToJson(settings *psmdc) {
	cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "version", psmdc->version);
    cJSON_AddNumberToObject(root, "language", psmdc->language);
    cJSON_AddNumberToObject(root, "rotation", psmdc->rotation);
    cJSON_AddNumberToObject(root, "noBuzzer", psmdc->noBuzzer);
    cJSON_AddNumberToObject(root, "sleepKeepLed", psmdc->sleepKeepLed);
    cJSON_AddNumberToObject(root, "setupDone", psmdc->setupDone);
	char *sz = cJSON_Print(root);
	cJSON_free(root);
	return sz;
}
#endif

void nameIdSave(int nid) {
	nameId = nid;
}

static void nameIdLoad() {
#ifdef DESKTOP
	nameId = 17;
#else
	if (nameId == -1)
		nameIdSave((rand() % 128) + 1);
#endif
}

void settingsDefault() {
	smdc.version = SETTINGS_VERSION;
	smdc.language = 0;
#if !defined(DESKTOP) && !defined(WEB)
	smdc.rotation = 3;
#else
	smdc.rotation = 0;
#endif
	smdc.noBuzzer = 0;
	smdc.sleepKeepLed = 0;
	smdc.setupDone = 0;
}

void settingsDump() {
	PRINTF("Settings language: %d\n", smdc.language);
	PRINTF("Settings rotation: %d\n", smdc.rotation);
	PRINTF("Settings noBuzzer: %d\n", smdc.noBuzzer);
	PRINTF("Settings sleepKeepLed: %d\n", smdc.sleepKeepLed);
	PRINTF("Settings setupDone: %d\n", smdc.setupDone);
}

void settingsLoad() {
	nameIdLoad();
	settingsDefault();
#if !defined(DESKTOP) && !defined(WEB)
	FILE *f = fopen(ADMIN_PATH "mydonglecloud/app.json", "r");
	if (f) {
		char sz[1024];
		fread(sz, sizeof(sz), 1, f);
		settingsFromJson(sz, &smdc);
		fclose(f);
	}
#endif
}

void settingsSave() {
#if !defined(DESKTOP) && !defined(WEB)
	FILE *f = fopen(ADMIN_PATH "mydonglecloud/app.json", "w");
	if (f) {
		char *sz = settingsToJson(&smdc);
		if (sz) {
			fwrite(sz, strlen(sz), 1, f);
			free(sz);
		}
		fclose(f);
	}
#endif
}
