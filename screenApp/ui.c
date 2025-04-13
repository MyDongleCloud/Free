#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "examples/lv_examples.h"
#include "macro.h"
#include "backend.h"

static void flush_cb(lv_disp_t *disp_drv, const lv_area_t *area, unsigned char *colorp) {
	PRINTF("Update: xy:%dx%d wh:%dx%d\n", area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
	backendUpdate(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, colorp);
    lv_disp_flush_ready(disp_drv);
}

static uint32_t tick_get() {
	static struct timespec start = {0};
	if (start.tv_sec == 0)
		clock_gettime(CLOCK_REALTIME, &start);
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return (now.tv_nsec - start.tv_nsec) / 1000 / 1000 + ((now.tv_sec - start.tv_sec) * 1000);
}

static void uiLogic() {
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);
	lv_obj_t * label = lv_label_create(lv_screen_active());
	lv_label_set_text(label, "Hello world");
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void uiInit() {
	lv_init();
	backendInit();
	lv_display_t * disp = lv_display_create(WIDTH, HEIGHT);
	lv_display_set_buffers(disp, fb, 0, WIDTH * HEIGHT * 3, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(disp, flush_cb);
}

void uiRun() {
	int count = 0;
    while (1) {
		lv_tick_inc(tick_get());
		lv_timer_handler();
		backendRun();
		if (count++ == 1)
			uiLogic();		
    }
}
