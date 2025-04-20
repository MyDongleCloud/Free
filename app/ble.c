#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <bluetooth/bluetooth.h>
#include "btlib.h"
#include "bluetoothAddr.h"
#include "macro.h"
#include "common.h"

//Global variable
char bluetoothClassicAddr[18] = { 0 };

//Functions
static int le_callback(int clientnode, int operation, int cticn) {
	if(operation == LE_CONNECT) {
		PRINTF("le_callback connect from %s(%d)\n", device_name(clientnode), clientnode);
		buzzer(3);
	} else if(operation == LE_READ) {
		PRINTF("le_callback: %s read by %s\n", ctic_name(localnode(), cticn), device_name(clientnode));
	} else if(operation == LE_WRITE) {
		PRINTF("le_callback: %s written by %s\n",ctic_name(localnode(), cticn), device_name(clientnode));
		char buf[256];
		int nread = read_ctic(localnode(), cticn, buf, sizeof(buf));
		PRINTF("le_callback: len%d 0x%x %d\n", nread, buf[0], buf[0]);
	} else if(operation == LE_DISCONNECT) {
		PRINTF("le_callback: disconnect from %s\n", device_name(clientnode));
	} else if(operation == LE_TIMER) {
		PRINTF("le_callback: timer\n");
	}
	return SERVER_CONTINUE;
}

static void *bleStart_t(void *arg) {
#ifndef DESKTOP
	bluetoothAddr(bluetoothClassicAddr, 0);
	FILE *pf = fopen(MAIN_PATH "bluetooth2ServerCfg.txt", "w");
	if (pf) {
		char sz[1024];
		char nn[128];
		sprintf(nn, "MyDongleCloud-%s", "1234567890");
		sprintf(sz, "DEVICE=%s type=mesh node=2 address=%s\n", nn, bluetoothClassicAddr);
		fwrite(sz, strlen(sz), 1, pf);
		char *szTplt[] = {
		" PRIMARY_SERVICE=0000fff0-0000-1000-8000-00805f9b34fb\n",
		" lechar=Version permit=2 size=10 uuid=0000fff1-0000-1000-8000-00805f9b34fb\n",
		};
		for (int i = 0; i < sizeof(szTplt) / sizeof(char *); i++)
			fwrite(szTplt[i], strlen(szTplt[i]), 1, pf);
		fclose(pf);
	}
	int ret = init_blue(MAIN_PATH "bluetooth2ServerCfg.txt");
	if (ret != 1) {
		PRINTF("ERROR: 2. No btferret started\n");
		return 0;
	}
	write_ctic(localnode(), UUID_VERSION - 0xfff1, VERSION, 0);
	usleep(1000 * 1000);
	le_server(le_callback, 0);
	close_all();
#endif
}

void bleStart() {
	pthread_t pth;
	pthread_create(&pth, NULL, bleStart_t, NULL);
}
