#ifndef BACKEND_H
#define BACKEND_H

//Global variable
extern unsigned char *fb;

//Global functions
void backendInit_(int argc, char *argv[]);
void backendRun_();
void backendUpdate(int x, int y, int w, int h, unsigned char *colorp);
lv_indev_t *backendInitPointer();
lv_indev_t *backendInitKeyboard();

#endif
