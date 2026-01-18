#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "macro.h"
#include "common.h"
#include "backend.h"
#include "backend-plat.h"
#include "settings.h"
#include "logic.h"
#include "language.h"
#ifndef WEB
#include "comBle.h"
#include "comWebSocket.h"
#endif
#include "cJSON.h"
#include "cloud.h"
#include "common.h"
#include "communication.h"
#include "password.h"

//Function
int main(int argc, char *argv[]) {
	int option;
	int daemon = 0;
	int ble = 1;
	int forceRotation = -1;
	int forceLanguage = -1;
	while ((option = getopt(argc, argv, "bdhl:r:st")) != -1) {
		switch (option) {
		case 'b':
			ble = 0;
			break;
		case 'd':
			daemon = 1;
			break;
		case 'h':
			PRINTF("*******************************************************\n");
			PRINTF("Usage for app [-b -d -l la -r rot -s -t]\n");
			PRINTF("b:	Don't start ble\n");
			PRINTF("d:	Set daemon mode\n");
			PRINTF("h:	Print this usage and exit\n");
			PRINTF("l:	Force language\n");
			PRINTF("r:	Force rotation\n");
			PRINTF("s:	Put in slave mode\n");
			PRINTF("t:	Prepare translation header\n");
			exit(0);
			break;
		case 'l':
			if (strcmp(optarg, "en") == 0)
				forceLanguage = 0;
			else if (strcmp(optarg, "fr") == 0)
				forceLanguage = 1;
			break;
		case 'r':
			sscanf(optarg, "%d", &forceRotation);
			break;
		case 's':
			slaveMode =  1;
			break;
		case 't':
#ifndef WEB
			languagePrepare();
			unlink("../build/_app/language.o");
			unlink("../build/_app/language_.o");
			unlink("../build/_app/language__.o");
#endif
			exit(0);
			break;
		default:
			break;
		}
	}
#ifdef WEB
	if (forceLanguage != -1)
		smdc.language = forceLanguage;
	PRINTF("Version:%s\n", APP_VERSION);
#else
	if (killOtherPids("app")) {
		fprintf(stderr, "Exiting because process already exists\n");
		return 0;
	}
	logInit(daemon);
	cloudInit();
	getSerialID();
	PRINTF("Version:%s Serial:%s\n", APP_VERSION, szSerial);
	settingsLoad();
	if (forceRotation != -1) {
		smdc.rotation = forceRotation;
		settingsSave();
	}
	if (forceLanguage != -1) {
		smdc.language = forceLanguage;
		settingsSave();
	}
#endif
#ifdef DESKTOP
	languageTest();
#endif
#if !defined(DESKTOP) && !defined(WEB)
	writeValueKey(PLATFORM_PATH, "printk", "start app");
	chdir("/home/mdc/app");
	setenv("HOME", "/home/mdc", 1);
	if (ble)
		bleStart();
	buzzer(1);
#endif
#ifndef WEB
	communicationWebSocket();
#endif
	backendInit_plat(argc, argv);
	backendInit(daemon);
	backendRun_plat();
	backendUninit(daemon);
	backendUninit_plat();
	return 0;
}
