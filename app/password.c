#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <liboath/oath.h>
#include "macro.h"
#include "common.h"

//Functions
static int oathGenerate(char secret[33]) {
	int ret = 0;
	generateRandomHexString(secret);
	oath_init();
	char otp[8];
	char secretbin[17];
	size_t secretbinlen = 16;
	oath_hex2bin(secret, secretbin, &secretbinlen);
	oath_hotp_generate(secretbin, secretbinlen, 0, 6, 0, OATH_HOTP_DYNAMIC_TRUNCATION, otp);
	oath_done();
	sscanf(otp, "%d", &ret);
	return ret;
}

static int oathValidate(char secret[33], int OTP) {
	if (OTP < 0 || OTP > 999999)
		return 0;
	char otp[8];
	sprintf(otp, "%06d", OTP);
	oath_init();
	int ret = oath_hotp_validate(secret, strlen(secret), 0, 20, otp);
	oath_done();
	return ret;
}

int oathAdmin() {
	char secret[33];
	int  otp = oathGenerate(secret);
#ifndef DESKTOP
	FILE *pf = fopen(OATH_PATH, "w");
	if (pf) {
		char sz2[64];
		snprintf(sz2, sizeof(sz2), "HOTP admin - %s", secret);
		fwrite(sz2, strlen(sz2), 1, pf);
		fclose(pf);
	}
#endif
	return otp;
}

void passwordAdminChange(char *pwd) {
	char salt[33];
	generateRandomHexString(salt);
	strncpy(salt, "$6$", 3);
	salt[19] = '\0';
    char *hashed = crypt(pwd, salt);
    FILE *fp = popen("sudo " LOCAL_PATH "MyDongleCloud/pwd.sh", "w");
	if (fp) {
	    fputs(hashed, fp);
		pclose(fp);
	}
}
