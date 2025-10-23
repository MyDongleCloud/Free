#ifndef BACKEND_H
#define BACKEND_H

//Global variables
extern int doLoop;
extern int eventFdUI;

//Functions
int backendRotate(int incr);
void cleanExit(int todo);
void processButton(int b, int ignoreRotation, int longPress);
void processInput(char c);
void backendInit(int daemon);
void backendLoop();
void backendUninit(int daemon);

#endif
