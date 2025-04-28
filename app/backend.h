#ifndef BACKEND_H
#define BACKEND_H

//Public variable
extern int rotationCur;

//Functions
void backendInit(int argc, char *argv[]);
void backendRotate();
void cleanExit(int todo);
void processInput(char c);
void backendWork();
void backendLoop(int daemon);

#endif
