#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "lvgl.h"
#include "macro.h"
#include "logic.h"
#include "ui.h"
#include "language.h"
#include "settings.h"

//Defines color
#define COLOR_GREY 0xffffff
#define COLOR_LIGHT 0x0092ce
#define COLOR_DARK 0x013d7b
#define COLOR_WHITE 0xffffff

//Functions
static void set_angle(void *obj, int32_t v) {
	lv_arc_set_value((lv_obj_t *)obj, v);
}

static void arc() {
	lv_obj_t *arc = lv_arc_create(lv_screen_active());
	lv_arc_set_bg_angles(arc, 0, 360);
	lv_obj_set_size(arc , WIDTH, HEIGHT);
	lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
	lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_arc_color(arc, lv_color_hex(COLOR_DARK), LV_PART_INDICATOR);
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
	lv_obj_t *label0 = lv_label_create(lv_screen_active());
	lv_label_set_long_mode(label0, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
	lv_obj_set_width(label0, 100);
	lv_label_set_text(label0, sz);
	lv_obj_center(label0);

	static lv_style_t labelStyle;
	lv_style_init(&labelStyle);
	lv_style_set_text_font(&labelStyle, &lv_font_montserrat_20);
	lv_obj_add_style(label0, &labelStyle, LV_STATE_DEFAULT);
}

static void event_handler(lv_event_t *e) {
	lv_event_code_t code = lv_event_get_code(e);
	if(code == LV_EVENT_CLICKED) {
		lv_obj_t *btn = lv_event_get_current_target(e);
		int key = (int)(unsigned long)lv_obj_get_user_data(btn);
		logicKey(key, 0);
	}
}

static void button(int pos, char *sz, char *szImg) {
	int posx, posy;
	if (pos == LV_KEY_LEFT) {
		posx = 1;
		posy = 112;
	} else if (pos == LV_KEY_RIGHT) {
		posx = 78;
		posy = 112;
	} else if (pos == LV_KEY_DOWN) {
		posx = 78;
		posy = 2;
	} else if (pos == LV_KEY_UP) {
		posx = 1;
		posy = 2;
	}

    lv_obj_t *btn0 = lv_button_create(lv_screen_active());
	lv_obj_set_user_data(btn0, (void *)(unsigned long)pos);
    lv_obj_set_pos(btn0, posx, posy);
    lv_obj_set_size(btn0, 49, 14);
	static lv_style_t btnStyle;
	lv_style_init(&btnStyle);
	lv_style_set_bg_color(&btnStyle, lv_color_hex(COLOR_DARK));
	lv_style_set_radius(&btnStyle, 4);
    lv_obj_add_style(btn0, &btnStyle, 0);
    lv_obj_add_event_cb(btn0, event_handler, LV_EVENT_ALL, NULL);

    lv_obj_t *labelBtn0 = lv_label_create(btn0);
    lv_label_set_text(labelBtn0, sz);
	static lv_style_t labelStyleBtn0;
	lv_style_init(&labelStyleBtn0);
	lv_style_set_text_font(&labelStyleBtn0, &lv_font_montserrat_12);
	lv_style_set_text_color(&labelStyleBtn0, lv_color_hex(COLOR_WHITE));
	lv_obj_add_style(labelBtn0, &labelStyleBtn0, LV_STATE_DEFAULT);
	lv_obj_align_to(labelBtn0, btn0, LV_ALIGN_BOTTOM_MID, szImg ? 6 : 0, 8);

	if (szImg) {
		lv_obj_t *imgBtn0 = lv_image_create(lv_screen_active());
		lv_img_set_src(imgBtn0, szImg);
		lv_obj_set_pos(imgBtn0, posx + 5, posy + 2);
	}
}

static void advancement(int pos) {
	static lv_style_t btnStyle;
	lv_style_init(&btnStyle);
	lv_style_set_bg_color(&btnStyle, lv_color_hex(COLOR_DARK));
	lv_style_set_shadow_width(&btnStyle, 0);
	int x, y, s;

	x = 53;
	y = 118;
	s = 3;
	if (pos == 0) {
		s = 6;
		x -= 1;
		y -= 2;
	}
    lv_obj_t *btn0 = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn0, x, y);
    lv_obj_set_size(btn0, s, s);
    lv_obj_add_style(btn0, &btnStyle, 0);

	x = 59;
	y = 118;
	s = 3;
	if (pos == 1) {
		s = 6;
		x -= 1;
		y -= 2;
	}
    lv_obj_t *btn1 = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn1, x, y);
    lv_obj_set_size(btn1, s, s);
    lv_obj_add_style(btn1, &btnStyle, 0);

	x = 65;
	y = 118;
	s = 3;
	if (pos == 2) {
		s = 6;
		x -= 1;
		y -= 2;
	}
    lv_obj_t *btn2 = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn2, x, y);
    lv_obj_set_size(btn2, s, s);
    lv_obj_add_style(btn2, &btnStyle, 0);

	x = 71;
	y = 118;
	s = 3;
	if (pos == 3) {
		s = 6;
		x -= 1;
		y -= 2;
	}
    lv_obj_t *btn3 = lv_button_create(lv_screen_active());
    lv_obj_set_pos(btn3, x, y);
    lv_obj_set_size(btn3, s, s);
    lv_obj_add_style(btn3, &btnStyle, 0);

}

static lv_obj_t *labelTime;
static void uiBarTime() {
	char sz[128];
	time_t timer;
	time(&timer);
	struct tm *tm_info = localtime(&timer);
	strftime(sz, 26, "%H:%M:%S\n%d %b", tm_info);
	lv_label_set_text(labelTime, sz);
}

static void uiBar() {
	lv_obj_t *rect = lv_obj_create(lv_screen_active());
	lv_obj_set_size(rect , 128, 23);
	lv_obj_set_pos(rect , 0, 0);
	lv_obj_set_style_bg_color(rect, lv_color_hex(COLOR_DARK), LV_PART_MAIN);
	lv_obj_set_style_radius(rect, 0, LV_PART_MAIN);
	lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_remove_flag(rect, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *imgNav0 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgNav0, "img/icon-left.png");
	lv_obj_set_pos(imgNav0, 1, 1);

    lv_obj_t *imgNav1 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgNav1, "img/icon-sleep.png");
	lv_obj_set_pos(imgNav1, 1, 12);

    lv_obj_t *imgNav2 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgNav2, "img/icon-right.png");
	lv_obj_set_pos(imgNav2, 117, 1);

    lv_obj_t *imgNav3 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgNav3, "img/icon-shutdown.png");
	lv_obj_set_pos(imgNav3, 117, 12);

    lv_obj_t *imgBar0 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar0, "img/wifi-ok.png");
	lv_obj_set_pos(imgBar0, 14, 2);
    lv_obj_t *imgBar1 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar1, "img/cloud-ok.png");
	lv_obj_set_pos(imgBar1, 34, 2);
    lv_obj_t *imgBar2 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgBar2, "img/temperature-ok.png");
	lv_obj_set_pos(imgBar2, 52, 2);

	labelTime = lv_label_create(lv_screen_active());
	lv_obj_set_pos(labelTime, 66, 0);
	lv_obj_set_size(labelTime , 50, 24);
	static lv_style_t labelStyle1;
	lv_style_init(&labelStyle1);
	lv_style_set_text_font(&labelStyle1, &lv_font_montserrat_10);
	lv_style_set_text_color(&labelStyle1, lv_color_hex(COLOR_WHITE));
	lv_obj_add_style(labelTime, &labelStyle1, LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(labelTime, LV_TEXT_ALIGN_CENTER, 0);

	uiBarTime();
}

void uiScreenWelcome() {
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(COLOR_GREY), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(COLOR_DARK), LV_PART_MAIN);

	lv_obj_clean(lv_screen_active());
	uiBar();

	lv_obj_t *label0 = lv_label_create(lv_screen_active());
	lv_label_set_text(label0, L("Welcome!"));
	lv_obj_set_width(label0, 128);
	lv_obj_set_style_text_align(label0, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(label0, LV_ALIGN_TOP_LEFT, 0, 27);

	lv_obj_t *label1 = lv_label_create(lv_screen_active());
	lv_label_set_text(label1, L("Use the three buttons to rotate the screen."));
	lv_obj_set_width(label1, 128);
	lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
	static lv_style_t labelStyle1;
	lv_style_init(&labelStyle1);
	lv_style_set_text_font(&labelStyle1, &lv_font_montserrat_10);
	lv_obj_add_style(label1, &labelStyle1, LV_STATE_DEFAULT);
	lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 0, 46);

	lv_obj_t *label2 = lv_label_create(lv_screen_active());
	lv_label_set_text(label2, L("Press long on the top buttons to turn off the screen or shutdown."));
	lv_obj_set_width(label2, 128);
	lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
	static lv_style_t labelStyle2;
	lv_style_init(&labelStyle2);
	lv_style_set_text_font(&labelStyle2, &lv_font_montserrat_10);
	lv_obj_add_style(label2, &labelStyle2, LV_STATE_DEFAULT);
	lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 0, 73);

	button(LV_KEY_LEFT, L("Rot"), "img/icon-right2.png");
	button(LV_KEY_RIGHT, L("OK"), NULL);
}

void uiScreenSleep() {
	lv_obj_clean(lv_screen_active());
}

static void doubleText(char *sz, char *sz2, int y, int x2) {
	if (sz == NULL) {
		static lv_style_t labelStyle1;
		lv_style_init(&labelStyle1);
		lv_style_set_text_font(&labelStyle1, &lv_font_montserrat_10);
		lv_style_set_text_decor(&labelStyle1, LV_TEXT_DECOR_UNDERLINE);
		lv_style_set_text_color(&labelStyle1, lv_color_hex(COLOR_LIGHT));

		lv_obj_t *label1 = lv_label_create(lv_screen_active());
		lv_label_set_long_mode(label1, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR);
		lv_obj_set_width(label1, WIDTH - 8);
		lv_label_set_text(label1, sz2);
		lv_obj_set_pos(label1, 4, y);
		lv_obj_add_style(label1, &labelStyle1, LV_STATE_DEFAULT);
	} else {
		static lv_style_t labelStyle0;
		lv_style_init(&labelStyle0);
		lv_style_set_text_font(&labelStyle0, &lv_font_montserrat_10);
		//lv_style_set_text_color(&labelStyle0, lv_color_hex(COLOR_LIGHT));

		lv_obj_t *label0 = lv_label_create(lv_screen_active());
		lv_label_set_text(label0, sz);
		lv_obj_add_style(label0, &labelStyle0, LV_STATE_DEFAULT);
		lv_obj_set_pos(label0, 4, y);

		static lv_style_t labelStyle1;
		lv_style_init(&labelStyle1);
		lv_style_set_text_font(&labelStyle1, &lv_font_montserrat_10);
		lv_style_set_text_color(&labelStyle1, lv_color_hex(COLOR_LIGHT));

		lv_obj_t *label1 = lv_label_create(lv_screen_active());
		lv_label_set_text(label1, sz2);
		lv_obj_add_style(label1, &labelStyle1, LV_STATE_DEFAULT);
		lv_obj_set_pos(label1, x2, y);
	}
}

static void progressBar(int w, int y, char *sz, char *sz2, int p) {
#if 0
	lv_obj_t *label0 = lv_label_create(lv_screen_active());
	lv_label_set_text(label0, sz);
	static lv_style_t labelStyle0;
	lv_style_init(&labelStyle0);
	lv_style_set_text_font(&labelStyle0, &lv_font_montserrat_10);
	lv_obj_add_style(label0, &labelStyle0, LV_STATE_DEFAULT);
	lv_obj_set_pos(label0, (128 - w) / 2, y);
#endif
	doubleText(sz, sz2, y, 35);

	static lv_style_t style_bg;
	lv_style_init(&style_bg);
	lv_style_set_border_color(&style_bg, lv_color_hex(COLOR_DARK));
	lv_style_set_border_width(&style_bg, 1);
	lv_style_set_pad_all(&style_bg, 2);
	lv_style_set_radius(&style_bg, 3);
	static lv_style_t style_indic;
	lv_style_init(&style_indic);
	lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_color_hex(COLOR_LIGHT));
	lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_HOR);
	lv_style_set_radius(&style_indic, 2);
	lv_obj_t * bar = lv_bar_create(lv_screen_active());
	lv_obj_remove_style_all(bar);
	lv_obj_add_style(bar, &style_bg, LV_PART_MAIN);
	lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
	lv_obj_set_size(bar, w, 8);
	lv_bar_set_value(bar, p, LV_ANIM_ON);
	lv_obj_set_pos(bar, (128 - w) / 2, y + 13);
}

void uiScreenHome(int pos) {
	static int pos_;
	if (pos != -1)
		pos_ = pos;
	lv_obj_clean(lv_screen_active());
	uiBar();

	if (pos_ == 1) {
		doubleText("Name", "Dongle-cat", 28, 40);
		doubleText(NULL, "https://gregoiregentil.mydongle.cloud", 42, 40);
		doubleText(NULL, "https://g2.myd.cd", 56, 40);
		doubleText("Wi-Fi", "Gregoire", 70, 40);
		doubleText("Local", "192.168.10.21", 84, 40);
		doubleText("Ext", "166.23.45.165", 98, 40);
	} else if (pos_ == 2) {
		char sz[32];
		sprintf(sz, L("%d%% (%d°C)"), 3, 45);
		progressBar(120, 28, L("CPU"), sz, 3);

		sprintf(sz, L("%d%% (%d proc)"), 10, 145);
		progressBar(120, 54, L("Mem"), sz, 10);

		sprintf(sz, L("%d%% (%d/%d GB)"), 120 * 100 / 128, 120, 128);
		progressBar(120, 82, L("Disk"), sz, 120 * 100 / 128);
	} else if (pos_ == 3) {
		doubleText("Port https (443)", "OK", 28, 100);
		doubleText("Port mail (25)", "OK", 42, 100);
		doubleText("Port pop3s (995)", "OK", 56, 100);
		doubleText("Port imaps (993)", "OK", 70, 100);
		doubleText("Port smtps (465)", "OK", 84, 100);
	} else {
		char sz[8];
		sprintf(sz, L("Home %d"), pos_ + 1);
		lv_obj_t *label0 = lv_label_create(lv_screen_active());
		lv_label_set_text(label0, sz);
		lv_obj_center(label0);
	}

	button(LV_KEY_LEFT, sio.setupDone ? L("Tips") : L("Setup"), NULL);
	button(LV_KEY_RIGHT, L("Next"), NULL);
	advancement(pos_);
}

void uiScreenSetup() {
	lv_obj_clean(lv_screen_active());

    lv_obj_t *imgNav0 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgNav0, "img/qrcode-scanme.png");
	lv_obj_set_pos(imgNav0, 2, 2);
    lv_obj_t *imgNav1 = lv_image_create(lv_screen_active());
	lv_img_set_src(imgNav1, "img/qrcode-download.png");
	lv_obj_set_pos(imgNav1, 39, 14);

	//button(LV_KEY_LEFT, L("Rot"), "img/icon-right2.png");
	button(LV_KEY_RIGHT, L("Done"), NULL);
}

void uiScreenTips(char *sz, char *szButton, int pos, int total) {
	lv_obj_clean(lv_screen_active());

	lv_obj_t *label0 = lv_label_create(lv_screen_active());
	lv_label_set_text(label0, sz);
	lv_obj_set_width(label0, 128);
	lv_obj_set_style_text_align(label0, LV_TEXT_ALIGN_CENTER, 0);
	static lv_style_t labelStyle0;
	lv_style_init(&labelStyle0);
	lv_style_set_text_font(&labelStyle0, &lv_font_montserrat_12);
	lv_obj_add_style(label0, &labelStyle0, LV_STATE_DEFAULT);
	lv_obj_center(label0);

	lv_obj_t *label1 = lv_label_create(lv_screen_active());
	char sz2[16];
	sprintf(sz2, "%d/%d", pos + 1, total);
	lv_label_set_text(label1, sz2);
	lv_obj_set_width(label1, 128);
	lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(label1, LV_ALIGN_BOTTOM_LEFT, 0, -3);
	static lv_style_t labelStyle1;
	lv_style_init(&labelStyle1);
	lv_style_set_text_font(&labelStyle1, &lv_font_montserrat_10);
	lv_obj_add_style(label1, &labelStyle1, LV_STATE_DEFAULT);

	button(LV_KEY_UP, L("Back"), NULL);
	if (szButton)
		button(LV_KEY_DOWN, L(szButton), NULL);
	button(LV_KEY_LEFT, L("Prev."), NULL);
	button(LV_KEY_RIGHT, L("Next"), NULL);
}

void uiScreenShutdown() {
	lv_obj_clean(lv_screen_active());

	lv_obj_t *label0 = lv_label_create(lv_screen_active());
	lv_label_set_text(label0, L("Are you sure to shutdown the dongle?"));
	lv_obj_center(label0);

	button(LV_KEY_LEFT, L("No"), NULL);
	button(LV_KEY_RIGHT, L("Yes"), NULL);
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

	lv_obj_t *label0 = lv_label_create(lv_screen_active());
	lv_label_set_text(label0, L("Your code is"));
	lv_obj_set_width(label0, 128);
	lv_obj_set_style_text_align(label0, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(label0, LV_ALIGN_TOP_LEFT, 0, 4);
	static lv_style_t labelStyle1;
	lv_style_init(&labelStyle1);
	lv_obj_add_style(label0, &labelStyle1, LV_STATE_DEFAULT);

	lv_obj_t *label1 = lv_label_create(lv_screen_active());
	char sz2[16];
	sprintf(sz2, "%02d %02d %02d", (passcode / 100 / 100) % 100, (passcode / 100) % 100, passcode % 100);
	lv_label_set_text(label1, sz2);
	lv_obj_set_width(label1, 128);
	lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(label1, LV_ALIGN_TOP_LEFT, 0, 22);
	static lv_style_t labelStyle2;
	lv_style_init(&labelStyle2);
	lv_style_set_text_font(&labelStyle2, &lv_font_montserrat_30);
	lv_obj_add_style(label1, &labelStyle2, LV_STATE_DEFAULT);

	lv_obj_t *label2 = lv_label_create(lv_screen_active());
	char sz3[256];
	if (counter > 60)
		sprintf(sz3, L("Cancel the code\nif you didn't request.\nExpires in %dm %2ds"), counter / 60, counter % 60);
	else
		sprintf(sz3, L("Cancel the code\nif you didn't request.\nExpires in %2ds"), counter % 60);
	lv_label_set_text(label2, sz3);
	lv_obj_set_width(label2, 128);
	lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_align(label2, LV_ALIGN_TOP_LEFT, 0, 57);
	static lv_style_t labelStyle3;
	lv_style_init(&labelStyle3);
	lv_style_set_text_font(&labelStyle3, &lv_font_montserrat_12);
	lv_obj_add_style(label2, &labelStyle3, LV_STATE_DEFAULT);

	button(LV_KEY_LEFT, L("Cancel"), NULL);
	button(LV_KEY_RIGHT, L("Hide"), NULL);
}

void uiUpdate() {
	if (logicCur == LOGIC_WELCOME || logicCur == LOGIC_HOME)
		uiBarTime();
	else if (logicCur == LOGIC_PASSCODE)
		uiScreenPasscode(-1);
}
