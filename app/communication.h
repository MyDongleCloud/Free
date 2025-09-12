#ifndef COMMUNICATION_H
#define COMMUNICATION_H

//Public variable
extern int communicationConnected;

//Functions
void communicationConnection(int s);
int communicationJSON(void *el);
int communicationState();
void communicationReceive(unsigned char *data, int size, char *orig);
void communicationSocket();

#endif
