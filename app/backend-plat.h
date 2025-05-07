#ifndef BACKENDPLAT_H
#define BACKENDPLAT_H

//Global variable
extern unsigned char *fbPublic;

//Global functions
void backendInit_(int argc, char *argv[]);
void backendRotate_(int rot);
void backendRun_();
void backendUninit_();

#endif
