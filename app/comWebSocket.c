#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "communication.h"
#include "wsServer/ws.h"
#include "macro.h"

//Private variables
static ws_cli_conn_t *soleClient;

//Functions
static void onopen(ws_cli_conn_t *client) {
	PRINTF("comWebSocket: onopen\n");
	communicationConnection(2);
	soleClient = client;
	communicationDoState();
}

static void onclose(ws_cli_conn_t *client) {
	PRINTF("comWebSocket: onclose %s\n", ws_getaddress(client));
	communicationConnection(0);
	soleClient = NULL;
}

static void onmessage(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type) {
	communicationReceive((char *)msg, strlen(msg), "websocket");
}

int serverWriteDataWebSocket(unsigned char *data, int size) {
	if (soleClient)
		ws_sendframe(soleClient, data, size, 1);
	return size;
}

void communicationWebSocket() {
	int ret = ws_socket(&(struct ws_server){
		.host = "127.0.0.1",
		.port = COMWEBSOCKET_PORT,
		.thread_loop = 1,
		.timeout_ms = 2000,
		.evs.onopen = &onopen,
		.evs.onclose = &onclose,
		.evs.onmessage = &onmessage
	});
	if (ret == 0)
		PRINTF("comWebSocket: OK\n");
}
