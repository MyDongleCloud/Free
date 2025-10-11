//Browser
//dongle.page.ts	appButton
//appWrapper.js		Module._button
//backend-web.c		button
//backend.c			processButton
//logic.c			logicKey
//communication.c	communicationJSON
//comHtml.c			serverWriteDataHtml
//appWrapper.js		appServerWriteDataHtml
//ble.ts			writeData, BleClient.write (potentially split in chunks)
//OVER-THE-AIR browser->dongle
//comBle.c			le_callback
//communication.c	communicationReceive, write(eventFdBle)
//backend.c			logicKey
//logic.c			logicUpdate
//communication.c	communicationState, communicationJSON
//comBle.c			serverWriteDataBle, write_ctic
//OVER-THE-AIR dongle->browser
//ble.ts			bleNotifyDataCb, appServerReceive(b64=0)
//appWrapper.js		Module._serverReceiveHtml
//comHtml.c			serverReceiveHtml
//communication.c	communicationReceive
//logic.c			logicUpdate
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "macro.h"
#include "cJSON.h"
#ifdef WEB
#include "comHtml.h"
#else
#include "comBle.h"
#include "comSocket.h"
#include "comWebSocket.h"
#include "json.h"
#include "wifi.h"
#include "common.h"
#include "space.h"
#endif
#include "base64.h"
#include "logic.h"
#include "backend.h"
#include "settings.h"
#include "password.h"

//Public variable
int communicationConnected = 0;

//Functions
void communicationConnection(int s) {
	communicationConnected = s;
	if (slaveMode && s == 0)
		logicSlaveNotConnected();
}

int communicationString(char *sz) {
	int ret;
#ifdef WEB
	ret = serverWriteDataHtml(sz, strlen(sz));
#else
	if (communicationConnected == 1)
		ret = serverWriteDataBle(sz, strlen(sz));
	else if (communicationConnected == 2)
		ret = serverWriteDataWebSocket(sz, strlen(sz));
#endif
	return ret;
}

int communicationJSON(void *el) {
	char *sz = cJSON_Print((cJSON *)el);
	int ret = communicationString(sz);
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

static void *communicationState_t(void *arg) {
	usleep(1000 * 1000);
	communicationState();
	return 0;
}

void communicationDoState() {
		pthread_t pth;
		pthread_create(&pth, NULL, communicationState_t, NULL);
}

void communicationReceive(unsigned char *data, int size, char *orig) {
	//PRINTF("communicationReceive: (%d)#%s# via %s\n", size, data, orig);
//Examples:
//{"a":"passcode"}
//{"a":"sutdown"}
//{"a":"key", "k":0, "l":0}
//{"a":"state", "p":"blah_encoded64"}
//{"a":"setup", "space":{}, "email":"", "name":"", "password":"", "ssid":"", "security":"", "fullchain":"", "privatekey":"", "proxy":{} }
//{"a":"space"} -> {"a":"space", inline space.json }
	cJSON *el = cJSON_Parse(data);
	if (el) {
		char *action = cJSON_GetStringValue2(el, "a");
		if (strcmp(action, "passcode") == 0) {
			PRINTF("communicationReceive: Passcode\n");
			int v = -1;
			if (cJSON_HasObjectItem(el, "v"))
				v = (int)cJSON_GetNumberValue2(el, "v");
			if (v == 0)
				logicPasscodeFinished();
			else
				logicPasscode(v);
		} else if (strcmp(action, "shutdown") == 0) {
			PRINTF("communicationReceive: Shutdown\n");
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
		} else if (strcmp(action, "pam") == 0) {
			char *user = cJSON_GetStringValue2(el, "u");
			char *service = cJSON_GetStringValue2(el, "s");
			char *type = cJSON_GetStringValue2(el, "t");
			//PRINTF("PAM: user:%s service:%s type:%s\n", user, service, type);
		} else if (strcmp(action, "setup") == 0) {
			PRINTF("communicationReceive: Setup\n");
			if (cJSON_GetStringValue2(el, "ssid") && cJSON_GetStringValue2(el, "security"))
				wiFiAddActivate(cJSON_GetStringValue2(el, "ssid"), cJSON_GetStringValue2(el, "security"));
			jsonWrite(cJSON_GetObjectItem(el, "space"), ADMIN_PATH "mydonglecloud/space.json");
			jsonWrite(cJSON_GetObjectItem(el, "proxy"), ADMIN_PATH "mydonglecloud/proxy.json");
			FILE *fpC = fopen(ADMIN_PATH "letsencrypt/fullchain.pem", "w");
			if (fpC) {
				fwrite(cJSON_GetStringValue2(el, "fullchain"), strlen(cJSON_GetStringValue2(el, "fullchain")), 1, fpC);
				fclose(fpC);
			}
			FILE *fpK = fopen(ADMIN_PATH "letsencrypt/privkey.pem", "w");
			if (fpK) {
				fwrite(cJSON_GetStringValue2(el, "privatekey"), strlen(cJSON_GetStringValue2(el, "privatekey")), 1, fpK);
				fclose(fpK);
			}
			cJSON *data = cJSON_CreateObject();
			cJSON_AddStringToObject(data, "email", cJSON_GetStringValue2(el, "email"));
			cJSON_AddStringToObject(data, "name", cJSON_GetStringValue2(el, "name"));
			cJSON_AddStringToObject(data, "password", cJSON_GetStringValue2(el, "password"));
			char buf[1024];
			char *post = cJSON_Print(data);
			downloadURLBuffer("http://localhost:8091/MyDongleCloud/Auth/sign-up/email", buf, "Content-Type: application/json", post);
			free(post);
			cJSON_Delete(data);
			serviceAction("betterauth.service", "RestartUnit");
			system("sudo /usr/local/modules/mydonglecloud/setup.sh");
			spaceSetup();
			communicationString("{\"status\":1}");
			jingle();
		} else if (strcmp(action, "space") == 0) {
			cJSON *space = jsonRead(ADMIN_PATH "mydonglecloud/space.json");
			cJSON_AddStringToObject(space, "a", "space");
			communicationJSON(space);
			cJSON_Delete(space);
#endif
		} else if (strcmp(action, "date") == 0) {
			;
		} else {
			PRINTF("communicationReceive: action:%s via %s\n", action, orig);
#ifndef WEB
			jsonDump(el);
#endif
		}
	}
	cJSON_Delete(el);
}
