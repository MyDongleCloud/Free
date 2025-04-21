#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "macro.h"
#include "common.h"

//Functions
void logicOath() {
	char secret[33];
	int  otp = oathGenerate(secret);
	FILE *pf = fopen(OATH_PATH, "w");
	if (pf) {
		char sz2[64];
		sprintf(sz2, "HOTP mdc - %s", secret);
		fwrite(sz2, strlen(sz2), 1, pf);
		fclose(pf);
		system("chown root:root " OATH_PATH ";chmod 400 " OATH_PATH);
	}
}

int logicIsSetup() {
	struct stat statTest;
	return stat(SPACESNAME_PATH, &statTest) == 0;
}

void logicSetup(char *name, char *email) {
	FILE *pf = fopen(SPACESNAME_PATH, "w");
	if (pf) {
		char sz2[64];
		sprintf(sz2, "%s:%s", name, email);
		fwrite(sz2, strlen(sz2), 1, pf);
		fclose(pf);
	}
}
