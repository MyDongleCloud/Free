#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "macro.h"
#include "ble.h"
#include "logic.h"
#include "backend.h"
#include "settings.h"

//Public variable
int communicationConnected = 0;

//Functions
void communicationConnection(int s) {
	communicationConnected = s;
	if (slaveMode && s == 0)
		logicSlaveNotConnected();
}

int communicationBinary(unsigned char *data, int size) {
	if (!communicationConnected)
		return 0;
	unsigned char *data_ = malloc(size + 1);
	data_[0] = '-';
	memcpy(data_ + 1, data, size);
	int ret = serverWriteData(data_, size + 1);
	free(data_);
	return ret;
}

static int communicationBinary2(unsigned char *data, int size, unsigned char *data2, int size2) {
	if (!communicationConnected)
		return 0;
	unsigned char *data_ = malloc(size + size2 + 1);
	data_[0] = '-';
	memcpy(data_ + 1, data, size);
	memcpy(data_ + 1 + size, data2, size2);
	int ret = serverWriteData(data_, size + size2 + 1);
	free(data_);
	return ret;
}

int communicationText(char *sz) {
	if (!communicationConnected)
		return 0;
	unsigned char *data_ = malloc(strlen(sz) + 1);
	data_[0] = '_';
	memcpy(data_ + 1, sz, strlen(sz));
	int ret = serverWriteData(data_, strlen(sz) + 1);
	free(data_);
	return ret;
}

int communicationState() {
	return communicationBinary2((unsigned char *)&smdc, sizeof(smdc), (unsigned char *)&lmdc, sizeof(lmdc));
}

void communicationReceive(unsigned char *data, int size) {
	PRINTF("communicationReceive: (%d)#%s#\n", size, data[0] == '_' ? (char *)(data + 1) : "binary");
	if (size == sizeof(smdc) + sizeof(lmdc) + 1 && data[0] == '-') {
		memcpy(&smdc, data + 1, sizeof(smdc));
		memcpy(&lmdc, data + 1 + sizeof(smdc), sizeof(lmdc));
		logicUpdate();
	} else if (data[0] == '_' && strncmp(data + 1, "key ", 4) == 0) {
		int k, l;
		if (sscanf(data +1, "key %d,%d", &k, &l) == 2) {
			uint64_t value = (k << 8) + l;
			write(eventFdBle, &value, sizeof(value));
		}
	}
}
