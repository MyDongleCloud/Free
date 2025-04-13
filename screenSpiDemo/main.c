#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "screen.h"
#include "macro.h"

//Global variable
uint8_t sport = 0;

//Functions
int main(int argc, char *argv[]) {
	int option;
	while ((option = getopt(argc, argv, "h")) != -1) {
		switch (option) {
		case 'h':
			PRINTF("*******************************************************\n");
			PRINTF("Usage for spiScreen [-h]\n");
			PRINTF("h:		Print this usage and exit\n");
			exit(0);
			break;
		default:
			break;
		}
	}
	screenInit();
	screenWait(4, 85, 22);
	return 0;
}
