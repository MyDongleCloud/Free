#ifndef SETTINGS_H
#define SETTINGS_H

//Structs
typedef struct settings {
	int version;
	int language;
	int rotation;
	int noBuzzer;
	int sleepKeepLed;
	int setupDone;
} settings;

//Global variables
extern settings sio;
extern int nameId;

//Global functions
void nameIdSave(int nid);
void settingsDefault();
void settingsDump();
void settingsLoad();
void settingsSave();
#endif
