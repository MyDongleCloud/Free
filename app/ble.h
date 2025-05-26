#ifndef BLE_H
#define BLE_H

//Global variable
extern char bluetoothClassicAddr[18];

//Global functions
int serverWriteData(unsigned char *dat, int size);
void bleStart();

#endif
