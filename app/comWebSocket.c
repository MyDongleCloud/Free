#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "communication.h"
#include "wsServer/ws.h"
#include "macro.h"

//Struct
typedef struct list {
	ws_cli_conn_t *client;
	int isWeb;
	void *previous;
	void *next;
} list;

//Private variables
static pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;
static list *first;
static list *last;

//Functions
static void onopen(ws_cli_conn_t *client) {
	PRINTF("comWebSocket: onopen\n");
	pthread_mutex_lock(&listMutex);
	list *ll = malloc(sizeof(list));
	ll->client = client;
	const char *resource = ws_getpath(client);
	ll->isWeb = strcmp(resource, "/ws/") == 0;
	if (ll->isWeb)
		communicationConnection(0, 1);
	ll->next = NULL;
	if (first == NULL) {
		ll->previous = NULL;
		first = ll;
		last = ll;
	} else {
		last->next = ll;
		ll->previous = last;
	}
	pthread_mutex_unlock(&listMutex);
}

static void onclose(ws_cli_conn_t *client) {
	PRINTF("comWebSocket: onclose %s\n", ws_getaddress(client));
	pthread_mutex_lock(&listMutex);
	list *c = first;
	while (c) {
		if (c->client == client) {
			if (c->previous == NULL && c->next == NULL) {
				first = NULL;
				last = NULL;
			} else if (c->next == NULL) {
				last = c->previous;
				last->next = NULL;
			} else if (c->previous == NULL) {
				first = c->next;		
				first->previous = NULL;
			} else {
				((list *)c->previous)->next = c->next;
				((list *)c->next)->previous = c->previous;
			}
			free(c);
			list *curr = first;
			while (curr) {
				if (curr->isWeb) {
					pthread_mutex_unlock(&listMutex);
					return;
				}
				curr = curr->next;
			}
			communicationConnection(0, 0);
			pthread_mutex_unlock(&listMutex);
			return;
		}
		c = c->next;
	}
	PRINTF("WebSocket: Lost client in list\n");
	pthread_mutex_unlock(&listMutex);
}

static void onmessage(ws_cli_conn_t *client, const unsigned char *msg, uint64_t size, int type) {
	communicationReceive((char *)msg, size, "websocket");
}

int serverWriteDataWebSocket(unsigned char *data, int size) {
	pthread_mutex_lock(&listMutex);
	int count = 0;
	list *curr = first;
	while (curr) {
		count++;
		curr = curr->next;
	}
	if (count == 0) {
		pthread_mutex_unlock(&listMutex);
		return 0;
	}
	ws_cli_conn_t **clients = malloc(count * sizeof(ws_cli_conn_t *));
	curr = first;
	for (int i = 0; i < count; i++) {
		clients[i] = curr->isWeb ? curr->client : NULL;
		curr = curr->next;
	}
	pthread_mutex_unlock(&listMutex);
	for (int i = 0; i < count; i++)
		if (clients[i])
			ws_sendframe(clients[i], data, size, 1);
	free(clients);
	return size;
}

void communicationWebSocket() {
	first = NULL;
	last = NULL;
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
