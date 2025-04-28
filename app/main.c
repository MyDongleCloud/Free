#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "macro.h"
#include "common.h"
#include "backend.h"
#include "ble.h"

//Functions
int main(int argc, char *argv[]) {
	int option;
	int daemon = 0;
	int ble = 1;
	while ((option = getopt(argc, argv, "bdh")) != -1) {
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
		default:
			break;
		}
	}
	if (killOtherPids("app")) {
		fprintf(stderr, "Exiting because process already exists\n");
		return 0;
	}
	int debug = 0;
	logInit(daemon, debug);
	backendInit(argc, argv);
#ifndef DESKTOP
	writeValueKey(PLATFORM_PATH, "printk", "start app");
	chdir("/home/mdc/app");
	setenv("HOME", "/home/mdc", 1);
	if (ble)
		bleStart();
#endif
	if (daemon)
		jingle();
	backendLoop(daemon);
	return 0;
}
