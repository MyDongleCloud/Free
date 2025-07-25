#ifndef COMMON_H
#define COMMON_H

//Global variable
extern char szSerial[32];

//Global functions
void readString(const char *path, const char *key, char *buf, int size);
int readValue(const char *path, const char *key);
void readValues2(const char *path, const char *key, int *i, int *j);
void readValues4(const char *path, const char *key, int *i, int *j, int *k, int *l);
void writeString(const char *path, const char *key, char *buf, int size);
void writeValue(const char *path, const char *v);
void writeValueKey(const char *path, const char *key, const char *v);
void writeValueInt(const char *path, int i);
void writeValueInts(const char *path, int i, int j);
void writeValueKeyInt(const char *path, const char *key, int i);
void writeValueKeyInts(const char *path, const char *key, int i, int j);
void writeValueKeyPrintf(const char *path, const char *key, const char *fmt, ...);
int readTemperature();
void enterInputMode();
void leaveInputMode();
void copyFile(char *from, char *to, void (*progresscallback)());
void generateUniqueId(char sz[17]);
void generateRandomHexString(char sz[33]);
int oathGenerate(char secret[33]);
int oathValidate(char secret[33], int OTP);
void getSerialID();
int killOtherPids(char *sz);
int fileExists(char *st);
void logInit(int daemon);
void logUninit();
void logUninit();
void buzzer(int n);
void touchClick();
void jingle();
void touch(char *szPath);
int hardwareVersion();
int downloadURLFile(char *szURL, char *szFile, int (*progresscallback)(void *, double,  double,  double,  double));
int uploadURLFile(char *szURL, char *szName0, char *szdata0, char *szName1, char *szFile1, char *szType1, char *szName2, char *szFile2, char *szType2, int (*progresscallback)(void *, double,  double,  double,  double));
int downloadURLBuffer(char *szURL, char *buf);
void deleteDirectory(char *szFolder);
void getMd5sum(char *szPath, char *szMd5sum);
int getLocalIP(char *szIPCurrent);
void fillZeroFile(FILE *pf, int ssize);
void syncForce(int delay);

#endif
