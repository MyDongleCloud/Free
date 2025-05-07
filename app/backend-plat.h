#ifndef BACKENDPLAT_H
#define BACKENDPLAT_H

//Global variable
extern unsigned char *fbPublic;

//Global functions
void backendInit_plat(int argc, char *argv[]);
void backendRotate_plat(int rot);
void backendRun_plat();
void backendUninit_plat();

#endif
