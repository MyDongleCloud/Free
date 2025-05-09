#ifndef LOGIC_H
#define LOGIC_H

//Define
enum LOGIC_SCREEN {
	LOGIC_WELCOME,
	LOGIC_SLEEP,
	LOGIC_HOME,
	LOGIC_SETUP,
	LOGIC_TIPS,
	LOGIC_SHUTDOWN,
	LOGIC_BYE,
	LOGIC_MESSAGE,
	LOGIC_PASSCODE
};

//Public variables
extern int passcode;
extern int logicCur;

//Functions
int logicIsSetup();
void logicSetupName(char *name, char *email);
void logicKey(int key, int longPress);
void logicWelcome();
void logicSleep();
void logicHome(int force, int incr);
void logicSetup();
void logicTips(int force, int incr);
void logicShutdown();
void logicBye();
void logicMessage(int m);
void logicPasscode();
void logicPasscodeFinished();

#endif
