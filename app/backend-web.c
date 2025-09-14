#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <emscripten.h>
#include "macro.h"
#include "lvgl.h"
#include "backend.h"
#include "logic.h"
#include "communication.h"
#include "base64.h"

//Global variable
unsigned char *fbPublic;

//Functions
void backendInit_plat(int argc, char *argv[]) {
	fbPublic = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);

	lv_init();
	lv_display_t * disp = lv_sdl_window_create(WIDTH, HEIGHT);
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);
	lv_sdl_mouse_create();
	lv_indev_t *indevP = lv_sdl_mouse_create();
}

void backendRotate_plat(int rot) {}

static void looping(void *arg) {
	backendLoop();
}

//HTML -> C
void button(int b, int l) {
	processButton(b, 1, l);
}

//C -> HTML
static int serverWriteDataEx(unsigned char *data, int size, int doB64) {
	char *st;
	if (doB64)
		st = b64_encode(data, size);
	else
		st = data;
	EM_ASM({
		appServerWriteData(UTF8ToString($0), $1);
	}, st, doB64);
	return size;
}

int serverWriteData(unsigned char *data, int size) {
	return serverWriteDataEx(data, size, 1);
}

//HTML -> C
void communicationStatus(int s) {
	communicationConnection(s);
}

//HTML -> C
void serverReceive(char *st, int isB64) {
	communicationConnection(1);
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

void backendRun_plat() {
	communicationConnection(0);
	SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
	SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
	SDL_EventState(SDL_KEYUP, SDL_DISABLE);
	emscripten_set_main_loop_arg(looping, NULL, -1, true);
	PRINTF("End of doLoop\n");
}

void backendUninit_plat() {
	lv_deinit();
}
