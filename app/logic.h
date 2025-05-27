#ifndef LOGIC_H
#define LOGIC_H

//Struct
//#pragma pack(push, 1)
typedef struct logics {
	int current;
	int previous;
	int homePos;
	int tipsPos;
	int messageM;
	int passcode;
} logics;
//#pragma pack(pop)

//Enum
enum LOGIC_SCREEN {
	LOGIC_WELCOME,
	LOGIC_SLEEP,
	LOGIC_HOME,
	LOGIC_SETUP,
	LOGIC_TIPS,
	LOGIC_SHUTDOWN,
	LOGIC_BYE,
	LOGIC_MESSAGE,
	LOGIC_PASSCODE,
	LOGIC_SLAVENOTCONNECTED
};

//Public variables
extern logics lmdc;
extern int slaveMode;

//Functions
int logicIsSetup();
void logicSetupName(char *name, char *email);
void logicUpdate();
void logicKey(int key, int longPress);
void logicWelcome();
void logicSleep(int autoSleep);
void logicHome(int force, int incr);
void logicSetup();
void logicTips(int force, int incr);
void logicShutdown();
void logicBye();
void logicMessage(int m);
void logicPasscode(int forcePasscode);
void logicPasscodeFinished();
void logicSlaveNotConnected();

#endif
