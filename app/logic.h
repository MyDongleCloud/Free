#ifndef LOGIC_H
#define LOGIC_H

//Define
enum LOGIC_SCREEN {
	LOGIC_WAIT,
	LOGIC_QUIET,
	LOGIC_ROTATE,
	LOGIC_HOME,
	LOGIC_REPORT,
	LOGIC_ACTION,
	LOGIC_CONFIRMATION,
	LOGIC_MESSAGE,
	LOGIC_PASSCODE
};

//Public variables
extern int passcode;
extern int logicCur;
extern int rotationCur;

//Functions
int logicIsSetup();
void logicSetup(char *name, char *email);
void logicKey(int k);
void logicWait();
void logicQuiet();
void logicRotate();
void logicHome();
void logicReport();
void logicAction();
void logicConfirmation();
void logicMessage();
void logicPasscode();
void logicPasscodeFinished();

#endif
