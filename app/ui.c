#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "lvgl.h"
#include "macro.h"
#include "backend.h"
#include "logic.h"
#include "ui.h"
#include "language.h"

//Defines color
#define COLOR_BACKGROUND 0xdfdfdf
#define COLOR_TEXT 0x000000
#define COLOR_BAR 0xf2184f
#define COLOR_BAR_TEXT 0xffffff
#define COLOR_BUTTON 0xf2184f
#define COLOR_BUTTON_TEXT 0xffffff

//Functions
static void set_angle(void * obj, int32_t v) {
	lv_arc_set_value((lv_obj_t *)obj, v);
}

static void arc() {
	lv_obj_t *arc = lv_arc_create(lv_screen_active());
	lv_arc_set_bg_angles(arc, 0, 360);
	lv_obj_set_size(arc , WIDTH, HEIGHT);
	lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
	lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_arc_color(arc, lv_color_hex(COLOR_BUTTON), LV_PART_INDICATOR);
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
	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_long_mode(label1, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
	lv_obj_set_width(label1, 100);
	lv_label_set_text(label1, sz);
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

	static lv_style_t label_style;
	lv_style_init(&label_style);
	lv_style_set_text_font(&label_style, &lv_font_montserrat_20);
	lv_obj_add_style(label1, &label_style, LV_STATE_DEFAULT);
}

static void button(int pos, char *sz) {
	int posx, posy;
	if (pos == LV_KEY_LEFT) {
		posx = 2;
		posy = 106;
	} else if (pos == LV_KEY_RIGHT) {
		posx = 70;
		posy = 106;
	} else if (pos == LV_KEY_DOWN) {
		posx = 70;
		posy = 2;
	} else if (pos == LV_KEY_UP) {
		posx = 2;
		posy = 2;
	}

    lv_obj_t *btn1 = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn1, posx, posy);
    lv_obj_set_size(btn1, 56, 18);
	static lv_style_t style_btn;
	lv_style_init(&style_btn);
	lv_style_set_bg_color(&style_btn, lv_color_hex(COLOR_BUTTON));
    lv_obj_add_style(btn1, &style_btn, 0);

    lv_obj_t *labelBtn1 = lv_label_create(btn1);
    lv_label_set_text(labelBtn1, sz);
	static lv_style_t label_styleBtn1;
	lv_style_init(&label_styleBtn1);
	lv_style_set_text_font(&label_styleBtn1, &lv_font_montserrat_12);
	lv_style_set_text_color(&label_styleBtn1, lv_color_hex(COLOR_BUTTON_TEXT));
	lv_obj_add_style(labelBtn1, &label_styleBtn1, LV_STATE_DEFAULT);
	lv_obj_align_to(labelBtn1, btn1, LV_ALIGN_BOTTOM_MID, 0, 7);

//lv_obj_t * imgBtn2 = lv_image_create(btn2);
//lv_img_set_src(imgBtn2, "img/reset.png");
//lv_obj_align(imgBtn2, LV_ALIGN_CENTER, 0, -6);
}

static void uiBar() {
	lv_obj_t * my_rect = lv_obj_create(lv_screen_active());
	lv_obj_set_size(my_rect , 128, 24);
	lv_obj_set_pos(my_rect , 0, 0);
	lv_obj_set_style_bg_color(my_rect, lv_color_hex(COLOR_BAR), LV_PART_MAIN);
	lv_obj_set_style_radius(my_rect, 0, LV_PART_MAIN);
	lv_obj_set_style_border_width(my_rect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_remove_flag(my_rect, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * imgBar1 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar1, "img/wifi-ok.png");
	lv_obj_set_pos(imgBar1, 2, 3);
    lv_obj_t * imgBar2 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar2, "img/cloud-ok.png");
	lv_obj_set_pos(imgBar2, 27, 3);
    lv_obj_t * imgBar3 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar3, "img/temperature-warning.png");
	lv_obj_set_pos(imgBar3, 53, 3);

	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	char sz[128];
	time_t timer;
	time(&timer);
	struct tm *tm_info = localtime(&timer);
	strftime(sz, 26, "%H:%M:%S\n%d %b", tm_info);
	lv_label_set_text(label1, sz);
	lv_obj_set_pos(label1, 75, 1);
	lv_obj_set_size(label1 , 50, 24);
	static lv_style_t label_style1;
	lv_style_init(&label_style1);
	lv_style_set_text_font(&label_style1, &lv_font_montserrat_10);
	lv_style_set_text_color(&label_style1, lv_color_hex(COLOR_BAR_TEXT));
	lv_obj_add_style(label1, &label_style1, LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
}

void uiScreenWait() {
	lv_obj_clean(lv_screen_active());

	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(COLOR_BACKGROUND), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

	arc();
	circulaText(L("Welcome! press a button to start..."));
}

void uiScreenQuiet() {
	lv_obj_clean(lv_screen_active());
}

void uiScreenRotate() {
	lv_obj_clean(lv_screen_active());

	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	char sz[16];
	sprintf(sz, L("Rotate %d"), rotationCur);
	lv_label_set_text(label1, sz);
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

	button(LV_KEY_LEFT, L("Rotate"));
	button(LV_KEY_RIGHT, L("OK"));
}

void uiScreenHome() {
	lv_obj_clean(lv_screen_active());
	uiBar();

	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_text(label1, L("Home"));
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

	button(LV_KEY_LEFT, L("Action"));
	button(LV_KEY_RIGHT, L("Report"));
}

void uiScreenReport() {
	lv_obj_clean(lv_screen_active());
	uiBar();
	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_text(label1, L("Report"));
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

	button(LV_KEY_LEFT, L("Back"));
}

void uiScreenAction() {
	lv_obj_clean(lv_screen_active());

	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_text(label1, L("Action"));
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

	button(LV_KEY_UP, L("Back"));
	button(LV_KEY_DOWN, L("Rotate"));
	button(LV_KEY_LEFT, L("Quiet"));
	button(LV_KEY_RIGHT, L("OFF"));
}

void uiScreenConfirmation() {//Cancel, OK or //Yes, No
	lv_obj_clean(lv_screen_active());

	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_text(label1, L("Confirmation"));
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

	button(LV_KEY_LEFT, L("Cancel"));
	button(LV_KEY_RIGHT, L("OK"));
}

void uiScreenMessage() {//OK
	lv_obj_clean(lv_screen_active());

	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_text(label1, L("Message"));
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

	button(LV_KEY_RIGHT, L("OK"));
}

void uiScreenPasscode(int expiration) {
	static struct timespec start;
	if (expiration != -1) {
		clock_gettime(CLOCK_REALTIME, &start);
		start.tv_sec += expiration;
	}
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	int counter = start.tv_sec - now.tv_sec;
	if (counter < 1)
		logicPasscodeFinished();

	lv_obj_clean(lv_screen_active());

	lv_obj_t * label1 = lv_label_create(lv_screen_active());
	lv_label_set_text(label1, L("Your code is:"));
	lv_obj_align(label1, LV_ALIGN_CENTER, 0, -52);
	static lv_style_t label_style1;
	lv_style_init(&label_style1);
	lv_obj_add_style(label1, &label_style1, LV_STATE_DEFAULT);

	lv_obj_t * label2 = lv_label_create(lv_screen_active());
	char sz2[16];
	sprintf(sz2, "%02d %02d %02d", (passcode / 100 / 100) % 100, (passcode / 100) % 100, passcode % 100);
	lv_label_set_text(label2, sz2);
	lv_obj_align(label2, LV_ALIGN_CENTER, 0, -26);
	static lv_style_t label_style2;
	lv_style_init(&label_style2);
	lv_style_set_text_font(&label_style2, &lv_font_montserrat_30);
	lv_obj_add_style(label2, &label_style2, LV_STATE_DEFAULT);

	lv_obj_t * label3 = lv_label_create(lv_screen_active());
	char sz3[256];
	if (counter > 60)
		sprintf(sz3, L("Cancel the code\nif you didn't request.\nExpires in %dm %2ds"), counter / 60, counter % 60);
	else
		sprintf(sz3, L("Cancel the code\nif you didn't request.\nExpires in %2ds"), counter % 60);
	lv_label_set_text(label3, sz3);
	lv_obj_set_width(label3, 128);
	lv_obj_set_style_text_align(label3, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(label3, LV_ALIGN_CENTER, 0, 14);
	static lv_style_t label_style3;
	lv_style_init(&label_style3);
	lv_style_set_text_font(&label_style3, &lv_font_montserrat_12);
	lv_obj_add_style(label3, &label_style3, LV_STATE_DEFAULT);

	button(LV_KEY_LEFT, L("Cancel"));
	button(LV_KEY_RIGHT, L("Hide"));
}

void uiUpdate() {
	if (logicCur == LOGIC_HOME)
		uiScreenHome();
	else if (logicCur == LOGIC_PASSCODE)
		uiScreenPasscode(-1);
}
