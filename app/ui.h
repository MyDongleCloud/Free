#ifndef UI_H
#define UI_H

//Define
enum UI_SCREEN {
	UI_WELCOME
};

//Public variable
extern int uiCurrent;

//Functions
void uiUpdate();
void uiKey(int k);

#endif
