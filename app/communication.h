#ifndef COMMUNICATION_H
#define COMMUNICATION_H

//Public variable
extern int communicationConnected;

//Functions
void communicationConnection(int typ, int val);
int communicationString(char *sz);
int communicationJSON(void *el);
int communicationState();
void communicationDoState();
void communicationReceive(unsigned char *data, int size, char *orig);
void communicationSocket();

#endif
