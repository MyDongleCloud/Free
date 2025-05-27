#ifndef COMMUNICATION_H
#define COMMUNICATION_H

//Public variable
extern int communicationConnected;

//Functions
void communicationConnection(int s);
int communicationBinary(unsigned char *data, int size);
int communicationText(char *sz);
int communicationState();
void communicationReceive(unsigned char *data, int size);

#endif
