//Browser
//dongle.page.ts	appButton
//appWrapper.js		Module._button
//backend-web.c		button
//backend.c			processButton
//logic.c			logicKey
//communication.c	communicationJSON
//ble.c				serverWriteData, write_ctic
//OVER-THE-AIR browser->dongle
//ble.c				le_callback
//communication.c	communicationReceive, write(eventFdBle)
//backend.c			logicKey
//logic.c			logicUpdate
//communication.c	communicationState
//ble.c				serverWriteData, write_ctic
//OVER-THE-AIR dongle->browser
//ble.ts			bleNotifyDataCb, appServerReceive(b64=0)
//appWrapper.js		Module._serverReceive
//backend-web.c		serverReceive, b64_decode_ex
//communication.c	communicationReceive
//logic.c			logicUpdate
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "macro.h"
#include "cJSON.h"
#ifdef WEB
#include "backend-web.h"
#else
#include "ble.h"
#include "json.h"
#endif
#include "base64.h"
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

int communicationJSON(void *el) {
	char *sz = cJSON_Print((cJSON *)el);
	int ret = serverWriteData(sz, strlen(sz));
	free(sz);
	return ret;
}

int communicationState() {
	if (!communicationConnected)
		return 0;
	cJSON *el = cJSON_CreateObject();
	cJSON_AddStringToObject(el, "a", "state");
	unsigned char *data_ = malloc(sizeof(smdc) + sizeof(lmdc));
	memcpy(data_, &smdc, sizeof(smdc));
	memcpy(data_ + sizeof(smdc), &lmdc, sizeof(lmdc));
	char *sz = b64_encode(data_, sizeof(smdc) + sizeof(lmdc));
	free(data_);
	cJSON_AddStringToObject(el, "p", sz);
	free(sz);
	int ret = communicationJSON(el);
	cJSON_Delete(el);
	return ret;
}

void communicationReceive(unsigned char *data, int size) {
	//PRINTF("communicationReceive: (%d)#%s#\n", size, data);
//Examples:
//{"a":"passcode"}
//{"a":"sutdown"}
//{"a":"key", "k":0, "l":0}
//{"a":"state", "p":"blah_encoded64"}
//{"a":"setup", "space":"", "alias":"", byod:["", ""], "user":"admin", "pass":"", "email":"", "ssid":"", "wpa2":"", "chain":"", "key":""}
//{"a":"space"} -> {"a":"space", inline space.json }
	cJSON *el = cJSON_Parse(data);
	if (el) {
		char *action = cJSON_GetStringValue2(el, "a");
		if (strcmp(action, "passcode") == 0) {
			PRINTF("Requesting passcode\n");
			logicPasscode(-1);
		} else if (strcmp(action, "shutdown") == 0) {
			PRINTF("Requesting shutdown\n");
			logicShutdown();
		} else if (strcmp(action, "key") == 0) {
			int k = (int)cJSON_GetNumberValue2(el, "k");
			int l = (int)cJSON_GetNumberValue2(el, "l");
			uint64_t value = (k << 8) + l;
			write(eventFdBle, &value, sizeof(value));
		} else if (strcmp(action, "state") == 0) {
			unsigned long decsize;
			unsigned char *payload = b64_decode_ex(cJSON_GetStringValue2(el, "p"), &decsize);
			memcpy(&smdc, payload, sizeof(smdc));
			memcpy(&lmdc, payload + sizeof(smdc), sizeof(lmdc));
			free(payload);
			logicUpdate();
#ifndef WEB
		} else if (strcmp(action, "space") == 0) {
			cJSON *space = jsonRead(ADMIN_PATH "MyDongleCloud/space.json");
			cJSON_AddStringToObject(space, "a", "space");
			communicationJSON(space);
			cJSON_Delete(space);
#endif
		} else {
			PRINTF("communicationReceive: action:%s\n", action);
			jsonDump(el);
		}
	}
	cJSON_Delete(el);
}
