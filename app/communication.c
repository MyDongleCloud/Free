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
//communication.c	communicationReceive, logicKey
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
#include "cloud.h"
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
//{"a":"otp"}
//{"a":"sutdown"}
//{"a":"key", "k":0, "l":0}
//{"a":"state", "p":"blah_encoded64"}
//{"a":"setup", "betterauth": {"email":"", "name":"", "password":""}, "cloud":{"all":{}, "ollama":{}, "frp":{}, "postfix":{}}, "ssid":"", "security":"", "fullchain":"", "privatekey":"" }
//{"a":"cloud"} -> {"a":"cloud", _cloud_.json }
//{"a":"translation", "l":0}
	cJSON *el = cJSON_Parse(data);
	if (el) {
		char *action = cJSON_GetStringValue2(el, "a");
		if (strcmp(action, "otp") == 0) {
			char *email = cJSON_GetStringValue2(el, "e");
			PRINTF("communicationReceive: OTP by %s\n", email);
			int v = -1;
			if (cJSON_HasObjectItem(el, "v"))
				v = (int)cJSON_GetNumberValue2(el, "v");
			if (v == 0)
				logicOtpFinished();
			else
				logicOtp(v, email);
		} else if (strcmp(action, "shutdown") == 0) {
			PRINTF("communicationReceive: Shutdown\n");
			logicShutdown();
		} else if (strcmp(action, "key") == 0) {
			int k = (int)cJSON_GetNumberValue2(el, "k");
			int l = (int)cJSON_GetNumberValue2(el, "l");
			logicKey(k, l);
		} else if (strcmp(action, "translation") == 0) {
			int l = (int)cJSON_GetNumberValue2(el, "l");
			settingsLanguage(l);
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
			char *arg1 = cJSON_GetStringValue2(el, "o");
			//PRINTF("PAM: user:%s service:%s type:%s arg1:%s\n", user, service, type, arg1);
			if (arg1 && strcmp(arg1, "oath_success") == 0)
				logicOtpFinished();
		} else if (strcmp(action, "setup") == 0) {
			PRINTF("communicationReceive: Setup\n");
			cloudSetup(el);
		} else if (strcmp(action, "cloud") == 0) {
			cJSON *cloud = jsonRead(ADMIN_PATH "_config_/_cloud_.json");
			cJSON_AddStringToObject(cloud, "a", "cloud");
			communicationJSON(cloud);
			cJSON_Delete(cloud);
		} else if (strcmp(action, "update") == 0) {
			PRINTF("communicationReceive: Update\n");
			cloudInit();
#endif
		} else if (strcmp(action, "connection") == 0) {
			int c = (int)cJSON_GetNumberValue2(el, "c");
			communicationConnection(c);
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
