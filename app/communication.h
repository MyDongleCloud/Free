#ifndef COMMUNICATION_H
#define COMMUNICATION_H

//Public variable
extern int communicationConnectedBLE;

//Functions
void communicationConnectionBLE(int s);
int communicationBinary(unsigned char *data, int size);
int communicationText(char *sz);
void communicationReceive(unsigned char *data, int size);

#endif
