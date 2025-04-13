#ifndef BACKEND_H
#define BACKEND_H

//Global variable
extern unsigned char *fb;

//Global functions
void backendInit();
void backendRun();
void backendUpdate(int x, int y, int w, int h, unsigned char *colorp);

#endif
