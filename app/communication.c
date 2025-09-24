//Browser
//dongle.page.ts	appButton
//appWrapper.js		Module._button
//backend-web.c		button
//backend.c			processButton
//logic.c			logicKey
//communication.c	communicationJSON
//backend-web.c		serverWriteData, serverWriteDataEx
//appWrapper.js		appServerWriteData
//ble.ts			writeData, BleClient.write (potentially split in chunks)
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
#include <pthread.h>
#include <poll.h>
#include <arpa/inet.h>
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
#include "password.h"

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

void communicationReceive(unsigned char *data, int size, char *orig) {
	//PRINTF("communicationReceive: (%d)#%s# via %s\n", size, data, orig);
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
			int v = -1;
			if (cJSON_HasObjectItem(el, "v"))
				v = (int)cJSON_GetNumberValue2(el, "v");
			if (v == 0)
				logicPasscodeFinished();
			else
				logicPasscode(v);
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
		} else if (strcmp(action, "pam") == 0) {
			char *user = cJSON_GetStringValue2(el, "u");
			char *service = cJSON_GetStringValue2(el, "s");
			char *type = cJSON_GetStringValue2(el, "t");
			//PRINTF("PAM: user:%s service:%s type:%s\n", user, service, type);
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

static void *comSocket_t(void *arg) {
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len;
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) {
		PRINTF("comSocket: error socket\n");
		return 0;
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(COMMUNICATION_INTERNAL_PORT);
		int yes = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (bind(listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		PRINTF("comSocket: error socket bind\n");
		return 0;
	}
	if (listen(listen_sock, SOMAXCONN) < 0) {
		PRINTF("comSocket: error socket listen\n");
		return 0;
	}
	PRINTF("comSocket: OK\n");
	struct pollfd fds[SOMAXCONN + 1];
	int nfds = 1;
	fds[0].fd = listen_sock;
	fds[0].events = POLLIN;
	while (1) {
		poll(fds, nfds, -1);
		if (fds[0].revents & POLLIN) {
			int client_sock = accept(listen_sock, (struct sockaddr*)&client_addr, &client_len);
			if (client_sock < 0) {
				PRINTF("comSocket: error socket accept\n");
				continue;
			}
			//PRINTF("comSocket: new connection\n");
			if (nfds < SOMAXCONN + 1) {
				fds[nfds].fd = client_sock;
				fds[nfds].events = POLLIN;
				nfds++;
			} else {
				PRINTF("comSocket: error max reached\n");
				close(client_sock);
			}
		}
		for (int i = 1; i < nfds; i++) {
			if (fds[i].revents & (POLLIN | POLLHUP)) {
				char buf[1024];
				memset(buf, 0, sizeof(buf));
				int nbytes = read(fds[i].fd, buf, sizeof(buf));
				if (nbytes <= 0) {
					if (nbytes == 0) {
						//PRINTF("comSocket: connection ended\n");
					} else {
						PRINTF("comSocket: error socket read");
					}
					close(fds[i].fd);
					fds[i] = fds[nfds - 1];
					nfds--;
					i--;
				} else {
					communicationReceive(buf, nbytes, "socket");
					memset(buf, 0, nbytes);
					strcpy(buf, "{\"error\":0}");
					write(fds[i].fd, buf, strlen(buf));
				}
			}
		}
	}
	for (int i = 0; i < nfds; i++)
		close(fds[i].fd);
	return 0;
}

void communicationSocket() {
	pthread_t pth;
	pthread_create(&pth, NULL, comSocket_t, NULL);
}
