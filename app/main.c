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
	while ((option = getopt(argc, argv, "dh")) != -1) {
		switch (option) {
		case 'd':
			daemon = 1;
			break;
		case 'h':
			PRINTF("*******************************************************\n");
			PRINTF("Usage for app [-h]\n");
			PRINTF("h:		Print this usage and exit\n");
			exit(0);
			break;
		default:
			break;
		}
	}

	backendInit(argc, argv);

	writeValueKey(PLATFORM_PATH, "printk", "start app");

	chdir("/home/mdc/app");
	setenv("HOME", "/home/mdc", 1);

	if (killOtherPids("app")) {
		fprintf(stderr, "Exiting because process already exists\n");
		return 0;
	}

	int debug = 0;
	logInit(daemon, debug);

	bleStart();

	backendRun();
	return 0;
}
