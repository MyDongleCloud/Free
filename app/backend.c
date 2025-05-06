#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/input.h>
#include "lvgl.h"
#include "macro.h"
#include "common.h"
#include "backend-plat.h"
#include "ui.h"
#include "logic.h"
#include "settings.h"

//Private variables
static lv_indev_t *indevK;
static int doLoop = 0;

//Functions
static void backendUpdate(lv_disp_t *disp_drv, const lv_area_t *area, unsigned char *colorp) {
	//PRINTF("Update: xy:%dx%d wh:%dx%d\n", area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
	backendUpdate_(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, colorp, sio.rotation);
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
	backendRotate_(sio.rotation);
}

void backendRotate(int incr) {
	sio.rotation = (sio.rotation + 4 + incr) % 4;
	settingsSave();
	backendRotate_(sio.rotation);
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

static int rotateKey(int k, int ignore) {
	int ret = 0;
	switch (k) {
	case KEY_LEFT:
		ret = LV_KEY_LEFT;
		break;
	case KEY_RIGHT:
		ret = LV_KEY_RIGHT;
		break;
	case KEY_UP:
		ret = LV_KEY_UP;
		break;
	case KEY_DOWN:
		ret = LV_KEY_DOWN;
		break;
	}
#ifndef DESKTOP
	if (ignore == 0)
		ret = LV_KEY_UP + ((ret - LV_KEY_UP + sio.rotation) % 4);
#endif
	//PRINTF("Real:%s -> Virtual:%s\n", k == KEY_LEFT ? "KEY_LEFT": k == KEY_RIGHT ? "KEY_RIGHT": k == KEY_UP ? "KEY_UP": k == KEY_DOWN ? "KEY_DOWN" : "", ret == LV_KEY_LEFT ? "LV_KEY_LEFT": ret == LV_KEY_RIGHT ? "LV_KEY_RIGHT": ret == LV_KEY_UP ? "LV_KEY_UP": ret == LV_KEY_DOWN ? "LV_KEY_DOWN": "");
	return ret;
}

static void processButton(int b, int ignore, int longPress) {
	logicKey(rotateKey(b, ignore), longPress);
}

//UP, DOWN, RIGHT, LEFT
//keyboard: 65, 66, 67, 68
//KEY_: 103, 108, 106, 105
//LV_KEY_: 17, 18, 19, 20
void processInput(char c) {
	struct timeval tv;
	char szz[128];
	//PRINTF("processInput %d\n", c);
	switch (c) {
	case 65:
		processButton(KEY_UP, 1, 0);
		break;
	case 66:
		processButton(KEY_DOWN, 1, 0);
		break;
	case 67:
		processButton(KEY_RIGHT, 1, 0);
		break;
	case 68:
		processButton(KEY_LEFT, 1, 0);
		break;
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
	int fdButton = -1;
	doLoop = 1;
	if (!daemon) {
		enterInputMode();
		fdStdin = fileno(stdin);
	}
#ifndef DESKTOP
	fdButton = open(BUTTON_PATH, O_RDONLY);
#endif
	struct pollfd pollfd[2];
	pollfd[0].fd = fdStdin;
	pollfd[0].events = POLLIN;
	pollfd[1].fd = fdButton;
	pollfd[1].events = POLLIN;
	logicWelcome();
    while (doLoop != 0) {
		lv_tick_inc(tickGet());
		uint32_t time_till_next = lv_timer_handler();
		//PRINTF("Timer: %dms\n", time_till_next);
		if (time_till_next == LV_NO_TIMER_READY)
			time_till_next = LV_DEF_REFR_PERIOD;
		int ret = poll(pollfd, 2, time_till_next);
		if (doLoop == 0)
			break;
		if (pollfd[0].revents & POLLIN) {
			char c[2];
			read(pollfd[0].fd, &c, 1);
			if (c[0] != 27 && c[0] != 91)
				processInput(c[0]);
		}
		if (pollfd[1].revents & POLLIN) {
			struct input_event ev[64];
			int rd = read(pollfd[1].fd, ev, sizeof(ev));
			int i;
			for (i = 0; i < rd / sizeof(struct input_event); i++)
				if (ev[i].type == EV_KEY) {
					static int longPressDone = 0;
					if (ev[i].value == 1)
						longPressDone = 0;
					else if (ev[i].value == 2 && longPressDone == 0) {
						processButton(ev[i].code, 0, 1);
						longPressDone = 1;
					} else if (ev[i].value == 0 && longPressDone == 0)
						processButton(ev[i].code, 0, 0);
				}
		}
		if (count++ % 30 == 0)
			uiUpdate();
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
