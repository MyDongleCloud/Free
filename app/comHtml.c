#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <emscripten.h>
#include "base64.h"
#include "communication.h"

//Functions
//C -> HTML
int serverWriteDataHtml(unsigned char *data, int size) {
	int doB64 = 1;
	char *st;
	if (doB64)
		st = b64_encode(data, size);
	else
		st = data;
	EM_ASM({
		appServerWriteDataHtml(UTF8ToString($0), $1);
	}, st, doB64);
	return size;
}

//HTML -> C
void serverReceiveHtml(char *st, int isB64) {
	size_t size;
	unsigned char *data;
	if (isB64)
		data = b64_decode_ex(st, &size);
	else {
		size = strlen(st);
		data = st;
	}
	communicationReceive(data, size, "html");
	if (isB64)
		free(data);
}

//HTML -> C
void communicationStatus(int s) {
	communicationConnection(2, s);
}
