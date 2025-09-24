#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include "macro.h"
#include "common.h"
#include "backend.h"
#include "backend-plat.h"
#include "settings.h"
#include "logic.h"
#include "language.h"
#include "ble.h"
#include "space.h"
#include "common.h"
#include "communication.h"
#include "password.h"

//Function
int main(int argc, char *argv[]) {
	int option;
	int daemon = 0;
	int ble = 1;
	int forceRotation = -1;
	while ((option = getopt(argc, argv, "bdhr:s")) != -1) {
		switch (option) {
		case 'b':
			ble = 0;
			break;
		case 'd':
			daemon = 1;
			break;
		case 'h':
			PRINTF("*******************************************************\n");
			PRINTF("Usage for app [-b -d -r rot -s]\n");
			PRINTF("b:	Don't start ble\n");
			PRINTF("d:	Set daemon mode\n");
			PRINTF("h:	Print this usage and exit\n");
			PRINTF("r:	Force rotation\n");
			PRINTF("s:	Put in slave mode\n");
			exit(0);
			break;
		case 'r':
			sscanf(optarg, "%d", &forceRotation);
			break;
		case 's':
			slaveMode =  1;
			break;
		default:
			break;
		}
	}
#ifndef WEB
	if (killOtherPids("app")) {
		fprintf(stderr, "Exiting because process already exists\n");
		return 0;
	}
	logInit(daemon);
#ifndef WEB
	spaceSetup();
	getSerialID();
	PRINTF("Version:%s Serial:%s\n", MDC_VERSION, szSerial);
#else
	PRINTF("Version:%s\n", MDC_VERSION);
#endif
	settingsLoad();
	if (forceRotation != -1) {
		smdc.rotation = forceRotation;
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
#if !defined(WEB)
	communicationSocket();
#endif
	backendInit_plat(argc, argv);
	backendInit(daemon);
	backendRun_plat();
	backendUninit(daemon);
	backendUninit_plat();
	return 0;
}
