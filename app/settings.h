#ifndef SETTINGS_H
#define SETTINGS_H

//Struct
typedef struct settings {
	int version;
	int language;
	int rotation;
	int noBuzzer;
	int sleepKeepLed;
	int setupDone;
} settings;

//Global variables
extern settings smdc;
extern int nameId;

//Global functions
void nameIdSave(int nid);
void settingsDefault();
void settingsDump();
void settingsLoad();
void settingsSave();
void settingsLanguage(int l);

#endif
