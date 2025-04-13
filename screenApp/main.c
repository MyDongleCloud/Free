#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "macro.h"
#include "backend.h"

//Functions
int main(int argc, char *argv[]) {
	backendInit(argc, argv);

	int option;
	while ((option = getopt(argc, argv, "h")) != -1) {
		switch (option) {
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

	backendRun();
	return 0;
}
