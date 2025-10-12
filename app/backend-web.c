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

//#define GL

//Defines
#undef DEPTH
#define DEPTH 4
#define CSS_WIDTH 300
#define CSS_HEIGHT 300

//Private variables
static lv_color_t buf1[WIDTH * HEIGHT * DEPTH];
static unsigned char fbPublic[WIDTH * HEIGHT * DEPTH];
#ifdef GL
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *sdlTexture;
#else
static lv_display_t * dispBrowser;
static lv_obj_t *imgBrowser = NULL;
#endif

//Functions
static void backendPointer(lv_indev_t *indev, lv_indev_data_t *data) {
	int x, y;
	Uint32 buttons = SDL_GetMouseState(&x, &y);
	data->point.x = (x * WIDTH) / CSS_WIDTH;
	data->point.y = (y * HEIGHT) / CSS_HEIGHT;
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
#ifndef GL
	static uint32_t last_update_time = 0;
	uint32_t current_time = lv_tick_get();
	if (current_time - last_update_time < 50)
		return;
	last_update_time = current_time;
	lv_display_set_default(dispBrowser);
	if (imgBrowser == NULL) {
		lv_obj_t *scr2 = lv_display_get_screen_active(dispBrowser);
		imgBrowser = lv_image_create(scr2);
		lv_obj_set_size(imgBrowser, CSS_WIDTH, CSS_HEIGHT);
		lv_image_set_inner_align(imgBrowser, LV_IMAGE_ALIGN_STRETCH);
	}
	lv_obj_t *scr1 = lv_display_get_screen_active(disp);
	lv_draw_buf_t *snapshot = lv_snapshot_take(scr1, LV_COLOR_FORMAT_ARGB8888);
	lv_image_set_src(imgBrowser, snapshot);
	lv_display_set_default(disp);
#endif
}

void backendInit_plat(int argc, char *argv[]) {
	lv_init();
	lv_display_t *disp = lv_display_create(WIDTH, HEIGHT);
	lv_display_set_buffers(disp, buf1, 0, WIDTH * HEIGHT * DEPTH, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(disp, backendUpdate_plat);
	backendInitPointer_plat();
#ifdef GL
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, CSS_WIDTH, CSS_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	sdlTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
#else
	dispBrowser = lv_sdl_window_create(CSS_WIDTH, CSS_HEIGHT);
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
