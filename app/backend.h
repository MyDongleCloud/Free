#ifndef BACKEND_H
#define BACKEND_H

//Functions
void backendInit(int argc, char *argv[]);
void cleanExit(int todo);
void processInput(char c);
void backendWork();
void backendLoop(int daemon);

#endif
