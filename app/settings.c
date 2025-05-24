#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "macro.h"
#include "settings.h"

//Global variables
settings smdc;
int nameId;

//Functions
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
	smdc.rotation = 0;
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
	struct stat statTest;
	if (stat(MAIN_PATH "settings", &statTest) == -1)
		 mkdir(MAIN_PATH "settings", 0775);
	settingsDefault();
	FILE *f = fopen(MAIN_PATH "settings/smdc", "rb");
	if (f) {
		fread(&smdc, sizeof(smdc), 1, f);
		fclose(f);
	}
}

void settingsSave() {
#ifndef DESKTOP
	FILE *f = fopen(MAIN_PATH "settings/smdc", "wb+");
	if (f) {
		fwrite(&smdc, sizeof(smdc), 1, f);
		fclose(f);
	}
#endif
}
