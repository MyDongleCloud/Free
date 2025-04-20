#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl.h"
#include "macro.h"
#include "backend.h"

//Functions
static void btn_event_cb(lv_event_t *e) {
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * btn = lv_event_get_target(e);
	if(code == LV_EVENT_CLICKED) {
		PRINTF("Clicked\n");
	}
}

void uiLogic() {
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x0054e9), LV_PART_MAIN);

	lv_obj_t * my_rect = lv_obj_create(lv_screen_active());
	lv_obj_set_size(my_rect , 128, 24);
	lv_obj_set_pos(my_rect , 0, 0);
	lv_obj_set_style_bg_color(my_rect, lv_color_hex(0x1a65eb), LV_PART_MAIN);
	lv_obj_set_style_radius(my_rect, 0, LV_PART_MAIN);
	lv_obj_set_style_border_width(my_rect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(my_rect, lv_color_hex(0x1a65eb), LV_PART_MAIN);

//	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x0054e9), LV_PART_MAIN);
    lv_obj_t * imgBar1 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar1, "img/wifi-ok.png");
	lv_obj_set_pos(imgBar1, 1, 3);
    lv_obj_t * imgBar2 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar2, "img/cloud-ok.png");
	lv_obj_set_pos(imgBar2, 27, 3);
    lv_obj_t * imgBar3 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar3, "img/temperature-warning.png");
	lv_obj_set_pos(imgBar3, 53, 3);
    lv_obj_t * imgBar4 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar4, "img/cloud-error.png");
	lv_obj_set_pos(imgBar4, 79, 3);
    lv_obj_t * imgBar5 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar5, "img/cloud-error.png");
	lv_obj_set_pos(imgBar5, 105, 3);

    lv_obj_t * img1 = lv_image_create(lv_screen_active());
	lv_img_set_src(img1, "img/wait.png");
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, -20);
    lv_obj_t *label1 = lv_label_create(lv_screen_active());
    lv_label_set_text(label1, "Booting...");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 5);

    lv_obj_t *btn1 = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn1, 4, 84);
    lv_obj_set_size(btn1, 50, 36);
    lv_obj_t * imgBtn1 = lv_image_create(btn1);
	lv_img_set_src(imgBtn1, "img/rotate.png");
    lv_obj_align(imgBtn1, LV_ALIGN_CENTER, 0, -6);
    lv_obj_t *labelBtn1 = lv_label_create(btn1);
    lv_label_set_text(labelBtn1, "Rotate");
	lv_obj_align_to(labelBtn1, btn1, LV_ALIGN_BOTTOM_MID, 0, 7);

    lv_obj_t *btn2 = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn2, 74, 84);
    lv_obj_set_size(btn2, 50, 36);
    lv_obj_t * imgBtn2 = lv_image_create(btn2);
	lv_img_set_src(imgBtn2, "img/reset.png");
    lv_obj_align(imgBtn2, LV_ALIGN_CENTER, 0, -6);
    lv_obj_t *labelBtn2 = lv_label_create(btn2);
    lv_label_set_text(labelBtn2, "Reset");
	lv_obj_align_to(labelBtn2, btn2, LV_ALIGN_BOTTOM_MID, 0, 7);
}

void uiKey(int k) {
	if (k == LV_KEY_UP) {
		PRINTF("KEY_UP\n");
		//lv_obj_send_event(btn, LV_EVENT_CLICKED, NULL);
	}
}
