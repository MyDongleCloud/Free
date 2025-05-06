#ifndef UI_H
#define UI_H

//Functions
void uiUpdate();
void uiKey(int k);
void uiScreenWelcome();
void uiScreenSleep();
void uiScreenHome(int pos);
void uiScreenSetup();
void uiScreenTips(char *sz, char *szButton, int pos, int total);
void uiScreenShutdown();
void uiScreenPasscode(int expiration);
void uiUpdate();

#endif
