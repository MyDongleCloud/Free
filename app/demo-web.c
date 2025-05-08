#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <emscripten.h>
#include "demos/lv_demos.h"

static void hal_init(void) {
	lv_display_t * disp = lv_sdl_window_create(800, 480);
	lv_group_t * g = lv_group_create();
	lv_group_set_default(g);
	lv_sdl_mouse_create();
	lv_sdl_mousewheel_create();
	lv_sdl_keyboard_create();
	lv_indev_t * mouse = lv_sdl_mouse_create();
	lv_indev_set_group(mouse, lv_group_get_default());
	lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
	lv_indev_set_group(mousewheel, lv_group_get_default());
	lv_indev_t * keyboard = lv_sdl_keyboard_create();
	lv_indev_set_group(keyboard, lv_group_get_default());    
}

void do_loop(void *arg) {
    lv_task_handler();
}

int main(int argc, char ** argv) {
	lv_init();
	hal_init();
	extern void lv_demo_widgets(void);
	lv_demo_widgets();
	emscripten_set_main_loop_arg(do_loop, NULL, -1, true);
}
