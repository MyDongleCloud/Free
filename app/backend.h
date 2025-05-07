#ifndef BACKEND_H
#define BACKEND_H

//Global variable
extern int doLoop;

//Functions
int backendRotate(int incr);
void cleanExit(int todo);
void processInput(char c);
void backendInit(int daemon);
void backendLoop();
void backendUninit(int daemon);

#endif
