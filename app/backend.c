#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "lvgl.h"
#include "macro.h"
#include "common.h"
#include "backend-plat.h"
#include "ui.h"
#include "logic.h"

//Private variables
static lv_indev_t *indevK;
static int doLoop = 0;

//Functions
static void backendUpdate(lv_disp_t *disp_drv, const lv_area_t *area, unsigned char *colorp) {
	//PRINTF("Update: xy:%dx%d wh:%dx%d\n", area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
	backendUpdate_(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, colorp);
    lv_disp_flush_ready(disp_drv);
}

static uint32_t tickGet() {
	static struct timespec start = {0};
	if (start.tv_sec == 0)
		clock_gettime(CLOCK_REALTIME, &start);
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	int ret = (now.tv_nsec - start.tv_nsec) / 1000 / 1000 + ((now.tv_sec - start.tv_sec) * 1000);
start = now;
	return ret;
}

void backendInit(int argc, char *argv[]) {
	lv_init();
	backendInit_(argc, argv);
	lv_display_t *disp = lv_display_create(WIDTH, HEIGHT);
	lv_display_set_buffers(disp, fbPublic, 0, WIDTH * HEIGHT * DEPTH, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(disp, backendUpdate);
	backendInitPointer();
	indevK = backendInitKeyboard();
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(COLOR_BACKGROUND), LV_PART_MAIN);
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
}

void cleanExit(int todo) {
	PRINTF("cleanExit mode:%d\n", todo);
	doLoop = 0;
	if (todo == 3) {
		PRINTF("cleanExit called for application restart\n");
	}
	logUninit();
#ifndef DESKTOP
	if (todo == 3) {
		FILE *pf = fopen("/tmp/softreset", "w+");
		if (pf)
			fclose(pf);
	} else if (todo == 2)
		system("sync || /usr/bin/mydonglecloud-leds.sh -b 0 -l off || sleep 0.1 || reboot &");
	else if (todo == 1)
		system("sync || /usr/bin/mydonglecloud-leds.sh -b 0 -l off || sleep 0.1 || shutdown -h now &");
#endif
}

void processInput(char c) {
	struct timeval tv;
	char szz[128];
	PRINTF("processInput %c\n", c);
	switch (c) {
	case 'h':
		PRINTF("*******************************************************\n");
		PRINTF("Usage for keyboard input:\n");
		PRINTF("h:	Print help information\n");
		PRINTF("x:	Exit\n");
		PRINTF("X:	Exit and power off\n");
		PRINTF("Z:	Generate a crash\n");
		PRINTF("*******************************************************\n");
		break;
	case 'x':
		cleanExit(0);
		break;
	case 'X':
		cleanExit(1);
		break;
	case 'Z': {
			int *a = NULL;
			*a = 0;
		}
		break;
	default:
		break;
	}
}

void backendWork(int daemon) {
	int count = 0;
	int keyLast = 0;
	int fdStdin = -1;
	doLoop = 1;
	if (!daemon) {
		enterInputMode();
		fdStdin = fileno(stdin);
	}
	struct pollfd pollfd[1];
	pollfd[0].fd = fdStdin;
	pollfd[0].events = POLLIN;
	logicWait();
    while (doLoop != 0) {
		lv_tick_inc(tickGet());
		uint32_t time_till_next = lv_timer_handler();
		//PRINTF("Timer: %dms\n", time_till_next);
		if (time_till_next == LV_NO_TIMER_READY)
			time_till_next = LV_DEF_REFR_PERIOD;
		int ret = poll(pollfd, 1, time_till_next);
		if (doLoop == 0)
			break;
		int keyCur = 0;
		if (pollfd[0].revents & POLLIN) {
			char c[2];
			read(pollfd[0].fd, &c, 1);
			c[1] = '\0';
			if (c[0] >= 65 && c[0] <= 68)
				keyCur = c[0] - 48;
			else if (c[0] != 27 && c[0] != 91)
				processInput(c[0]);
		}
		if (count++ % 30 == 0)
			uiUpdate();
		if (keyCur == 0)
			keyCur = lv_indev_get_key(indevK);
		if (keyCur != keyLast) {
			if (keyCur != 0)
				logicKey(keyCur);
			keyLast = keyCur;
		}
	}
	PRINTF("End of doLoop\n");
	if (!daemon)
		leaveInputMode();
	lv_deinit();
	backendUninit_();
}

void backendLoop() {
	backendLoop_();
}
