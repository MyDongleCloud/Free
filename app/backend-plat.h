#ifndef BACKEND_H
#define BACKEND_H

//Global variable
extern unsigned char *fbPublic;

//Global functions
void backendInit_(int argc, char *argv[]);
void backendRotate_();
void backendLoop_();
void backendUpdate_(int x, int y, int w, int h, unsigned char *colorp);
void backendUninit_();
void backendInitPointer();

#endif
