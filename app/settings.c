#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "macro.h"
#include "settings.h"

//Global variables
settings sio;
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
	sio.version = SETTINGS_VERSION;
	sio.language = 0;
	sio.rotation = 0;
	sio.noBuzzer = 0;
	sio.setupDone = 0;
}

void settingsDump() {
	PRINTF("Settings language: %d\n", sio.language);
	PRINTF("Settings rotation: %d\n", sio.rotation);
	PRINTF("Settings noBuzzer: %d\n", sio.noBuzzer);
	PRINTF("Settings setupDone: %d\n", sio.setupDone);
}

void settingsLoad() {
	nameIdLoad();
	struct stat statTest;
	if (stat(MAIN_PATH "settings", &statTest) == -1)
		 mkdir(MAIN_PATH "settings", 0775);
	settingsDefault();
	FILE *f = fopen(MAIN_PATH "settings/sio", "rb");
	if (f) {
		fread(&sio, sizeof(sio), 1, f);
		fclose(f);
	}
}

void settingsSave() {
#ifndef DESKTOP
	FILE *f = fopen(MAIN_PATH "settings/sio", "wb+");
	if (f) {
		fwrite(&sio, sizeof(sio), 1, f);
		fclose(f);
	}
#endif
}
