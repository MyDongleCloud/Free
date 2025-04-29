#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "lvgl.h"
#include "macro.h"
#include "common.h"
#include "logic.h"
#include "backend.h"
#include "ui.h"

//Public variables
int logicCur;
int passcode;

//Private variable
static int logicPrev;

//Functions
static int logicOath() {
#ifdef DESKTOP
	return 484170;
#endif
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
	return otp;
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

void logicKey(int k) {
	if (logicCur == LOGIC_WAIT)
		logicRotate();
	else if (logicCur == LOGIC_QUIET) {
#ifndef DESKTOP
		system("/usr/bin/mydonglecloud-leds.sh -b 1 -l normal");
#endif
		logicHome();
	} else if (logicCur == LOGIC_ROTATE) {
		if (k == LV_KEY_LEFT) {
			backendRotate();
			logicRotate();
		} else if (k == LV_KEY_RIGHT)
			logicHome();
	} else if (logicCur == LOGIC_HOME) {//Action, Report
		if (k == LV_KEY_LEFT)
			logicAction();
		else if (k == LV_KEY_RIGHT)
			logicReport();
	} else if (logicCur == LOGIC_REPORT) {//Back
		if (k == LV_KEY_LEFT)
			logicHome();
		else if (k == LV_KEY_RIGHT)
			logicPasscode();
	} else if (logicCur == LOGIC_ACTION) {//Back, Rotate, Quiet, OFF
		if (k == LV_KEY_UP)
			logicHome();
		else if (k == LV_KEY_DOWN)
			logicRotate();
		else if (k == LV_KEY_LEFT) {
			logicQuiet();
		} else if (k == LV_KEY_RIGHT)
			logicConfirmation();//"Shutdown");
	} else if (logicCur == LOGIC_PASSCODE) {//Cancel, Hide
		if (k == LV_KEY_LEFT)
			logicHome();
		else if (k == LV_KEY_RIGHT)
			logicHome();
	} else if (logicCur == LOGIC_CONFIRMATION) {//Cancel, OK or //Yes, No
		if (k == LV_KEY_LEFT)
			logicHome();
		else if (k == LV_KEY_RIGHT)
			logicHome();
	} else if (logicCur == LOGIC_MESSAGE) {//OK
		if (k == LV_KEY_LEFT || k == LV_KEY_RIGHT || k == LV_KEY_UP || k == LV_KEY_DOWN)
			logicHome();
	}
}

void logicWait() {
	PRINTF("Logic: Wait\n");
	logicCur = LOGIC_WAIT;
	uiScreenWait();
}

void logicQuiet() {
	PRINTF("Logic: Quiet\n");
	logicCur = LOGIC_QUIET;
#ifndef DESKTOP
		system("/usr/bin/mydonglecloud-leds.sh -b 0 -l off");
#endif
	uiScreenQuiet();
}

void logicRotate() {
	PRINTF("Logic: Rotate %d\n", rotationCur);
	logicCur = LOGIC_ROTATE;
	uiScreenRotate();
}

void logicHome() {
	PRINTF("Logic: Home\n");
	logicCur = LOGIC_HOME;
	uiScreenHome();
}

void logicReport() {
	PRINTF("Logic: Report\n");
	logicCur = LOGIC_REPORT;
	uiScreenReport();
}

void logicAction() {
	PRINTF("Logic: Action\n");
	logicCur = LOGIC_ACTION;
	uiScreenAction();
}

void logicConfirmation() {
	PRINTF("Logic: Confirmation\n");
	logicCur = LOGIC_CONFIRMATION;
	uiScreenConfirmation();
}

void logicMessage() {
	PRINTF("Logic: Message\n");
	logicCur = LOGIC_MESSAGE;
	uiScreenMessage();
}

void logicPasscode() {
	PRINTF("Logic: Passcode\n");
	logicCur = LOGIC_PASSCODE;
	passcode = logicOath();
	uiScreenPasscode(90);
}

void logicPasscodeFinished() {
	PRINTF("Logic: Passcode finished\n");
	passcode = 0;
	logicHome();
}
