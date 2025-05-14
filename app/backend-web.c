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

void injectInput(char c) {
	processInput(c);
}

void requestPasscode(int p) {
	logicPasscode(p);
}

void backendRun_plat() {
	SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);
	SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
	SDL_EventState(SDL_KEYUP, SDL_DISABLE);
	emscripten_set_main_loop_arg(looping, NULL, -1, true);
	PRINTF("End of doLoop\n");
}

void backendUninit_plat() {
	lv_deinit();
}
