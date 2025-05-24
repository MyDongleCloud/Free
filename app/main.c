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
#include "language.h"
#include "ble.h"

//Functions
int main(int argc, char *argv[]) {
	int option;
	int daemon = 0;
	int ble = 1;
	int forceRotation = -1;
	while ((option = getopt(argc, argv, "bdhr:")) != -1) {
		switch (option) {
		case 'b':
			ble = 0;
			break;
		case 'd':
			daemon = 1;
			break;
		case 'h':
			PRINTF("*******************************************************\n");
			PRINTF("Usage for app [-b -d -h]\n");
			PRINTF("b:		Don't start ble\n");
			PRINTF("d:		Set daemon mode\n");
			PRINTF("h:		Print this usage and exit\n");
			exit(0);
			break;
		case 'r':
			sscanf(optarg, "%d", &forceRotation);
			break;
		default:
			break;
		}
	}
	int debug = 0;
#ifndef WEB
	if (killOtherPids("app")) {
		fprintf(stderr, "Exiting because process already exists\n");
		return 0;
	}
	logInit(daemon, debug);
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
	if (daemon)
		jingle();
#endif
	backendInit_plat(argc, argv);
	backendInit(daemon);
	backendRun_plat();
	backendUninit(daemon);
	backendUninit_plat();
	return 0;
}
