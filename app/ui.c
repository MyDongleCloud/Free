#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl.h"
#include "macro.h"
#include "backend.h"

//Functions
void uiLogic() {
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x0054e9), LV_PART_MAIN);

    lv_obj_t *label1 = lv_label_create(lv_screen_active());
    lv_label_set_text(label1, "Welcome");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
}

void uiKey(int k) {
	if (k == LV_KEY_UP) {
		PRINTF("KEY_UP\n");
	} else if (k == LV_KEY_DOWN) {
		PRINTF("KEY_DOWN\n");
	} else if (k == LV_KEY_LEFT) {
		PRINTF("KEY_LEFT\n");
	} else if (k == LV_KEY_RIGHT) {
		PRINTF("KEY_RIGHT\n");
	}
}
