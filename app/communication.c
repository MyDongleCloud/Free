#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <time.h>
#include "macro.h"
#include "ble.h"
#include "logic.h"

//Public variable
int communicationConnectedBLE = 0;

//Functions
void communicationConnectionBLE(int s) {
	communicationConnectedBLE = s;
}

int communicationBinary(unsigned char *data, int size) {
	unsigned char *data_ = malloc(size + 1);
	data_[0] = 2;
	memcpy(data_ + 1, data, size);
	int ret = serverWriteData(data_, size + 1);
	free(data_);
	return ret;
}

int communicationText(char *sz) {
	unsigned char *data_ = malloc(strlen(sz) + 1);
	data_[0] = 1;
	memcpy(data_ + 1, sz, strlen(sz));
	int ret = serverWriteData(data_, strlen(sz) + 1);
	free(data_);
	return ret;
}

void communicationReceive(unsigned char *data, int size) {
	PRINTF("communicationReceive: (%d)#%s#\n", size, data);
	if (size == sizeof(lmdc) + 1 && data[0] == 2) {
		memcpy(&lmdc, data + 1, sizeof(lmdc));
		logicUpdate();
	}
}
