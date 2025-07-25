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
#include "communication.h"

//Public variables
logics lmdc;
int slaveMode = 0;

//Functions
static int logicOath() {
#ifdef WEB
	return rand() % 999999;
#endif
	char secret[33];
	int  otp = oathGenerate(secret);
#ifndef DESKTOP
	FILE *pf = fopen(OATH_PATH, "w");
	if (pf) {
		char sz2[64];
		sprintf(sz2, "HOTP mdc - %s", secret);
		fwrite(sz2, strlen(sz2), 1, pf);
		fclose(pf);
		system("chown root:root " OATH_PATH ";chmod 400 " OATH_PATH);
	}
#endif
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

void logicKey(int key, int longPress) {
	if (slaveMode) {
		char sz[16];
		sprintf(sz, "key %d,%d", key, longPress);
		communicationText(sz);
		return;
	}

	if (lmdc.current == LOGIC_WELCOME) {//Rotations, OK
		if (longPress && key == LV_KEY_UP)
			logicSleep(0);
		else if (longPress && key == LV_KEY_DOWN)
			logicShutdown();
		else if (key == LV_KEY_UP) {
			if (backendRotate(1) == 0)
				logicWelcome();
		} else if (key == LV_KEY_DOWN) {
			if (backendRotate(-1) == 0)
				logicWelcome();
		} else if (key == LV_KEY_LEFT) {
			if (backendRotate(-1) == 0)
				logicWelcome();
		} else if (key == LV_KEY_RIGHT)
			logicHome(0, 0);
	} else if (lmdc.current == LOGIC_SLEEP) {
#ifndef DESKTOP
		system("/usr/local/modules/mydonglecloud/leds.sh -b 1 -l normal");
#endif
		logicHome(-1, 0);
	} else if (lmdc.current == LOGIC_HOME) {//Rotations, Tips, Next
		if (longPress && key == LV_KEY_UP)
			logicSleep(0);
		else if (longPress && key == LV_KEY_DOWN)
			logicShutdown();
		else if (key == LV_KEY_UP) {
			if (backendRotate(1) == 0)
				logicHome(-1, 0);
		} else if (key == LV_KEY_DOWN) {
			if (backendRotate(-1) == 0)
				logicHome(-1, 0);
		} else if (key == LV_KEY_LEFT) {
			if (smdc.setupDone)
				logicTips(0, 0);
			else
				logicSetup();
		} else if (key == LV_KEY_RIGHT)
			logicHome(-1, 1);
	} else if (lmdc.current == LOGIC_SETUP) {//Done
		if (key == LV_KEY_RIGHT) {
			smdc.setupDone = 1;
			logicHome(0, 0);
		}
	} else if (lmdc.current == LOGIC_TIPS) {//Back, Setup, Previous, Next
		if (key == LV_KEY_UP)
			logicHome(-1, 0);
		else if (key == LV_KEY_DOWN)
			logicSetup();
		else if (key == LV_KEY_LEFT)
			logicTips(-1, -1);
		else if (key == LV_KEY_RIGHT)
			logicTips(-1, 1);
	} else if (lmdc.current == LOGIC_PASSCODE) {//Cancel, Hide
		if (key == LV_KEY_LEFT)
			logicHome(0, 0);
		else if (key == LV_KEY_RIGHT)
			logicHome(0, 0);
	} else if (lmdc.current == LOGIC_SHUTDOWN) {//Yes, No
		if (key == LV_KEY_LEFT)
			logicHome(0, 0);
		else if (key == LV_KEY_RIGHT)
			logicBye();
	} else if (lmdc.current == LOGIC_BYE) {
	} else if (lmdc.current == LOGIC_SLAVENOTCONNECTED) {
	} else if (lmdc.current == LOGIC_MESSAGE)
		logicHome(0, 0);
}

void logicUpdate() {
	static int first = 1;
	if (first) {
		uiScreenInit();
		first = 0;
	}
	if (lmdc.current == LOGIC_WELCOME)
		uiScreenWelcome();
	else if (lmdc.current == LOGIC_SLEEP)
		uiScreenSleep();
	else if (lmdc.current == LOGIC_HOME)
		uiScreenHome();
	else if (lmdc.current == LOGIC_SETUP)
		uiScreenSetup();
	else if (lmdc.current == LOGIC_TIPS)
		uiScreenTips();
	else if (lmdc.current == LOGIC_SHUTDOWN)
		uiScreenShutdown();
	else if (lmdc.current == LOGIC_BYE)
		uiScreenBye();
	else if (lmdc.current == LOGIC_MESSAGE)
		uiScreenMessage();
	else if (lmdc.current == LOGIC_PASSCODE)
		uiScreenPasscode(90);
	else if (lmdc.current == LOGIC_SLAVENOTCONNECTED)
		uiScreenSlaveNotConnected();

	if (!slaveMode && communicationConnected)
		communicationState();
}

void logicWelcome() {
	PRINTF("Logic: Welcome rot:%d\n", smdc.rotation);
	lmdc.current = LOGIC_WELCOME;
	logicUpdate();
}

void logicSleep(int autoSleep) {
	PRINTF("Logic: Sleep from %s\n", autoSleep ? "auto" : "user");
#ifndef DESKTOP
	if (smdc.sleepKeepLed)
		system("/usr/local/modules/mydonglecloud/leds.sh -b 0 -l normal");
	else
		system("/usr/local/modules/mydonglecloud/leds.sh -b 0 -l off");
#endif
	lmdc.current = LOGIC_SLEEP;
	logicUpdate();
}

void logicHome(int force, int incr) {
	if (force != -1)
		lmdc.homePos = force;
	else
		lmdc.homePos = (lmdc.homePos + 4 + incr) % 4;
	PRINTF("Logic: Home #%d rot:%d\n", lmdc.homePos, smdc.rotation);
	lmdc.current = LOGIC_HOME;
	logicUpdate();
}

void logicSetup() {
	PRINTF("Logic: Setup\n");
	lmdc.current = LOGIC_SETUP;
	logicUpdate();
}

void logicTips(int force, int incr) {
	int size = 6;//FIXME coming from sizeof(szTips)/sizeof(szTips[0])
	if (force != -1)
		lmdc.tipsPos = force;
	else
		lmdc.tipsPos = (lmdc.tipsPos + size + incr) % size;
	PRINTF("Logic: Tips #%d\n", lmdc.tipsPos + 1);
	lmdc.current = LOGIC_TIPS;
	logicUpdate();
}

void logicShutdown() {
	PRINTF("Logic: Shutdown\n");
	lmdc.current = LOGIC_SHUTDOWN;
	logicUpdate();
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
#ifndef WEB
	pthread_t pth;
	pthread_create(&pth, NULL, bye_t, NULL);
#endif
	lmdc.current = LOGIC_BYE;
	logicUpdate();
}

void logicMessage(int m) {
	lmdc.messageM = m;
	PRINTF("Logic: Message\n");
	lmdc.current = LOGIC_MESSAGE;
	logicUpdate();
}

void logicPasscode(int forcePasscode) {
	PRINTF("Logic: Passcode forcePasscode:%d\n", forcePasscode);
	if (forcePasscode != -1)
		lmdc.passcode = forcePasscode;
	else
		lmdc.passcode = logicOath();
	lmdc.current = LOGIC_PASSCODE;
	logicUpdate();
}

void logicPasscodeFinished() {
	PRINTF("Logic: Passcode finished\n");
	lmdc.passcode = 0;
	logicHome(0, 0);
}


void logicSlaveNotConnected() {
	PRINTF("Logic: Slave not connected\n");
	lmdc.current = LOGIC_SLAVENOTCONNECTED;
	logicUpdate();
}
