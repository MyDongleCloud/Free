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
	int otp;
} logics;
//#pragma pack(pop)

//Enum
enum LOGIC_SCREEN {
	LOGIC_WELCOME,
	LOGIC_SLEEP,
	LOGIC_HOME,
	LOGIC_SETUP,
	LOGIC_SETUP_SUCCESS,
	LOGIC_LOGIN,
	LOGIC_TIPS,
	LOGIC_SHUTDOWN,
	LOGIC_BYE,
	LOGIC_MESSAGE,
	LOGIC_OTP,
	LOGIC_SLAVENOTCONNECTED
};

//Public variables
extern logics lmdc;
extern int slaveMode;

//Functions
void logicUpdate();
void logicKey(int key, int longPress);
void logicWelcome();
void logicSleep(int autoSleep);
void logicHome(int force, int incr);
void logicSetup();
void logicSetupSuccess();
void logicLogin();
void logicTips(int force, int incr);
void logicShutdown();
void logicBye();
void logicMessage(int m);
void logicOtp(int forceOtp);
void logicOtpFinished();
void logicSlaveNotConnected();

#endif
