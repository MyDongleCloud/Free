#ifndef COMBLE_H
#define COMBLE_H

//Global variable
extern char bluetoothClassicAddr[18];

//Global functions
int serverWriteDataBle(unsigned char *dat, int size);
void bleStart();

#endif
