#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "lvgl.h"
#include "macro.h"
#include "common.h"
#include "logic.h"
#include "backend.h"
#include "ui.h"
#include "settings.h"

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

void logicSetupName(char *name, char *email) {
	FILE *pf = fopen(SPACESNAME_PATH, "w");
	if (pf) {
		char sz2[64];
		sprintf(sz2, "%s:%s", name, email);
		fwrite(sz2, strlen(sz2), 1, pf);
		fclose(pf);
	}
}

void logicKey(int k) {
	if (logicCur == LOGIC_WELCOME) {//Rotations, OK
		if (k == LV_KEY_ESC)
			logicSleep();
		else if (k == LV_KEY_DEL)
			logicShutdown();
		else if (k == LV_KEY_UP) {
			if (backendRotate(1) == 0)
				logicWelcome();
		} else if (k == LV_KEY_DOWN) {
			if (backendRotate(-1) == 0)
				logicWelcome();
		} else if (k == LV_KEY_LEFT) {
			if (backendRotate(-1) == 0)
				logicWelcome();
		} else if (k == LV_KEY_RIGHT)
			logicHome(0, 0);
	} else if (logicCur == LOGIC_SLEEP) {
#ifndef DESKTOP
		system("/usr/bin/mydonglecloud-leds.sh -b 1 -l normal");
#endif
		logicHome(-1, 0);
	} else if (logicCur == LOGIC_HOME) {//Rotations, Tips, Next
		if (k == LV_KEY_ESC)
			logicSleep();
		else if (k == LV_KEY_DEL)
			logicShutdown();
		else if (k == LV_KEY_UP) {
			if (backendRotate(1) == 0)
				logicHome(-1, 0);
		} else if (k == LV_KEY_DOWN) {
			if (backendRotate(-1) == 0)
				logicHome(-1, 0);
		} else if (k == LV_KEY_LEFT) {
			if (sio.setupDone)
				logicTips(0, 0);
			else
				logicSetup();
		} else if (k == LV_KEY_RIGHT)
			logicHome(-1, 1);
	} else if (logicCur == LOGIC_SETUP) {//Done
		if (k == LV_KEY_RIGHT) {
			sio.setupDone = 1;
			logicHome(0, 0);
		}
	} else if (logicCur == LOGIC_TIPS) {//Back, Setup, Previous, Next
		if (k == LV_KEY_UP)
			logicHome(-1, 0);
		else if (k == LV_KEY_DOWN)
			logicSetup();
		else if (k == LV_KEY_LEFT)
			logicTips(-1, -1);
		else if (k == LV_KEY_RIGHT)
			logicTips(-1, 1);
	} else if (logicCur == LOGIC_PASSCODE) {//Cancel, Hide
		if (k == LV_KEY_LEFT)
			logicHome(0, 0);
		else if (k == LV_KEY_RIGHT)
			logicHome(0, 0);
	} else if (logicCur == LOGIC_SHUTDOWN) {//Yes, No
		if (k == LV_KEY_LEFT)
			logicHome(0, 0);
		else if (k == LV_KEY_RIGHT)
			logicBye();
	} else if (logicCur == LOGIC_BYE) {
	} else if (logicCur == LOGIC_MESSAGE)
		logicHome(0, 0);
}

void logicWelcome() {
	PRINTF("Logic: Welcome rot:%d\n", sio.rotation);
	logicCur = LOGIC_WELCOME;
	uiScreenWelcome();
}

void logicSleep() {
	PRINTF("Logic: Sleep\n");
	logicCur = LOGIC_SLEEP;
#ifndef DESKTOP
		system("/usr/bin/mydonglecloud-leds.sh -b 0 -l off");
#endif
	uiScreenSleep();
}

void logicHome(int force, int incr) {
	static int pos = 0;
	if (force != -1)
		pos = force;
	else
		pos = (pos + 4 + incr) % 4;
	PRINTF("Logic: Home #%d rot:%d\n", pos, sio.rotation);
	logicCur = LOGIC_HOME;
	uiScreenHome(pos);
}

void logicSetup() {
	PRINTF("Logic: Setup\n");
	logicCur = LOGIC_SETUP;
	uiScreenSetup();
}

void logicTips(int force, int incr) {
	static int pos = 0;
	int size = 6;//FIXME coming from sizeof(szTips)/sizeof(szTips[0])
	if (force != -1)
		pos = force;
	else
		pos = (pos + size + incr) % size;
	PRINTF("Logic: Tips #%d\n", pos + 1);
	logicCur = LOGIC_TIPS;
	uiScreenTips(pos);
}

void logicShutdown() {
	PRINTF("Logic: Shutdown\n");
	logicCur = LOGIC_SHUTDOWN;
	uiScreenShutdown();
}

static void *bye_t(void *arg) {
	usleep(1000 * 1000);
#ifdef DESKTOP
			cleanExit(0);
#else
			cleanExit(1);
#endif
	return 0;
}

void logicBye() {
	PRINTF("Logic: Bye\n");
	logicCur = LOGIC_BYE;
	uiScreenBye();
#ifndef WEB
	pthread_t pth;
	pthread_create(&pth, NULL, bye_t, NULL);
#endif
}

void logicMessage(int m) {
	PRINTF("Logic: Message\n");
	logicCur = LOGIC_MESSAGE;
	uiScreenMessage(m);
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
	logicHome(0, 0);
}
