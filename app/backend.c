#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl.h"
#include "macro.h"
#include "backend-plat.h"
#include "ui.h"

//Private variable
static lv_indev_t *indevK;

//Functions
static void flushCb(lv_disp_t *disp_drv, const lv_area_t *area, unsigned char *colorp) {
	PRINTF("Update: xy:%dx%d wh:%dx%d\n", area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
	backendUpdate(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, colorp);
    lv_disp_flush_ready(disp_drv);
}

static uint32_t tickGet() {
	static struct timespec start = {0};
	if (start.tv_sec == 0)
		clock_gettime(CLOCK_REALTIME, &start);
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return (now.tv_nsec - start.tv_nsec) / 1000 / 1000 + ((now.tv_sec - start.tv_sec) * 1000);
}

void backendInit(int argc, char *argv[]) {
	lv_init();
	backendInit_(argc, argv);
	lv_display_t * disp = lv_display_create(WIDTH, HEIGHT);
	lv_display_set_buffers(disp, fb, 0, WIDTH * HEIGHT * 3, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(disp, flushCb);
	backendInitPointer();
	indevK = backendInitKeyboard();
}

void backendRun() {
	int count = 0;
	int keyLast = 0;
    while (1) {
		lv_tick_inc(tickGet());
		lv_timer_handler();
		backendRun_();
		if (count++ == 1)
			uiUpdate();
		int keyCur = lv_indev_get_key(indevK);
		if (keyCur != keyLast) {
			if (keyCur != 0)
				uiKey(keyCur);
			keyLast = keyCur;
		}
	}
}
