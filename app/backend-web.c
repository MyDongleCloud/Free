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

#undef DEPTH
#define DEPTH 4
#define CSS_SIZE 300
//#define GL

//Private variables
static lv_color_t buf1[WIDTH * HEIGHT * DEPTH];
static unsigned char fbPublic[WIDTH * HEIGHT * DEPTH];
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *sdlTexture;

//Functions
static void backendPointer(lv_indev_t *indev, lv_indev_data_t *data) {
	int x, y;
	Uint32 buttons = SDL_GetMouseState(&x, &y);
	data->point.x = (x * WIDTH) / CSS_SIZE;
	data->point.y = (y * WIDTH) / CSS_SIZE;
	data->state = (buttons & SDL_BUTTON(1)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

void backendInitPointer_plat() {
	lv_indev_t *indevP = lv_indev_create();
	lv_indev_set_type(indevP, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(indevP, backendPointer);
}

static void backendUpdate_plat(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
	for (int y = area->y1; y <= area->y2; y++)
		memcpy(&fbPublic[(y * WIDTH + area->x1) * DEPTH], &px_map[(y - area->y1) * (area->x2 - area->x1 + 1) * DEPTH], (area->x2 - area->x1 + 1) * DEPTH);
	lv_display_flush_ready(disp);
}

void backendInit_plat(int argc, char *argv[]) {
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CSS_SIZE, CSS_SIZE, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	lv_init();
#ifdef GL
	lv_display_t *disp = lv_display_create(WIDTH, HEIGHT);
	lv_display_set_buffers(disp, buf1, 0, WIDTH * HEIGHT * DEPTH, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(disp, backendUpdate_plat);
	backendInitPointer_plat();
#else
	lv_display_t * disp = lv_sdl_window_create(WIDTH, HEIGHT);
	lv_sdl_mouse_create();
#endif
}

void backendRotate_plat(int rot) {}

static void looping(void *arg) {
	backendLoop();
#ifdef GL
	SDL_UpdateTexture(sdlTexture, NULL, fbPublic, WIDTH * DEPTH);
	SDL_RenderClear(renderer);
	SDL_Rect destRect = {0, 0, CSS_SIZE, CSS_SIZE};
	SDL_RenderCopy(renderer, sdlTexture, NULL, &destRect);
	SDL_RenderPresent(renderer);
#endif
}

//HTML -> C
void button(int b, int l) {
	processButton(b, 1, l);
}

void backendRun_plat() {
	SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
	SDL_EventState(SDL_KEYUP, SDL_DISABLE);
	emscripten_set_main_loop_arg(looping, NULL, 0, true);
	PRINTF("End of doLoop\n");
}

void backendUninit_plat() {
	lv_deinit();
}
