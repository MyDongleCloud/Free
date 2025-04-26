#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "lvgl.h"
#include "macro.h"
#include "backend.h"
#include "ui.h"

//Public variable
int uiCurrent = UI_WELCOME;

//Functions
static void set_angle(void * obj, int32_t v) {
	lv_arc_set_value((lv_obj_t *)obj, v);
}

static void arc() {
	lv_obj_t * arc = lv_arc_create(lv_screen_active());
	lv_arc_set_bg_angles(arc, 0, 360);
	lv_arc_set_value(arc, 10);
	lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
	lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_center(arc);
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, arc);
	lv_anim_set_exec_cb(&a, set_angle);
	lv_anim_set_duration(&a, 2000);
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_set_repeat_delay(&a, 0);
	lv_anim_set_values(&a, 0, 100);
	lv_anim_start(&a);
}

static void circulaText(char *sz) {
	static lv_anim_t animation_template;
	static lv_style_t label_style;
	lv_anim_init(&animation_template);
	lv_anim_set_delay(&animation_template, 0);
	lv_anim_set_repeat_count(&animation_template, LV_ANIM_REPEAT_INFINITE);
	lv_anim_set_repeat_delay(&animation_template, 500);
	lv_style_init(&label_style);
	lv_style_set_anim(&label_style, &animation_template);
	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_long_mode(label1, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
	lv_obj_set_width(label1, 100);
	lv_label_set_text(label1, sz);
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_style(label1, &label_style, LV_STATE_DEFAULT);
}

static void uiScreenWelcome() {
	arc();
	circulaText("It is a circularly scrolling text.");
}

void uiUpdate() {
	static int firstTime = 1;
	if (firstTime) {
		lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x0054e9), LV_PART_MAIN);
		lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
		firstTime = 0;
	}
	if (uiCurrent == UI_WELCOME)
		uiScreenWelcome();
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
