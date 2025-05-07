#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#ifndef WEB
#include <linux/input.h>
#endif
#include "lvgl.h"
#include "macro.h"
#include "common.h"
#include "backend-plat.h"
#include "ui.h"
#include "logic.h"
#include "settings.h"

//Defines
//from linux/input.h
#define KEY_UP		103
#define KEY_LEFT	105
#define KEY_RIGHT	106
#define KEY_DOWN	108
#define KEY_ESC		1
#define KEY_DELETE	111

//Public variable
int doLoop = 0;

//Private variables
static lv_indev_t *indevK;
#ifndef WEB
static struct pollfd pollfd[2];
#endif

//Functions
int backendRotate(int incr) {
#ifdef WEB
	logicMessage(0);
	return -1;
#endif
	sio.rotation = (sio.rotation + 4 + incr) % 4;
	settingsSave();
	backendRotate_plat(sio.rotation);
	return 0;
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
	case KEY_ESC:
		ret = LV_KEY_ESC;
		break;
	case KEY_DELETE:
		ret = LV_KEY_DEL;
		break;
	}
#ifndef DESKTOP
	if (ignore == 0)
		ret = LV_KEY_UP + ((ret - LV_KEY_UP + sio.rotation) % 4);
#endif
	//PRINTF("Real:%s -> Virtual:%s\n", k == KEY_LEFT ? "KEY_LEFT": k == KEY_RIGHT ? "KEY_RIGHT": k == KEY_UP ? "KEY_UP": k == KEY_DOWN ? "KEY_DOWN" : k == KEY_ESC ? "KEY_ESC": k == KEY_DELETE ? "KEY_DELETE": "", ret == LV_KEY_LEFT ? "LV_KEY_LEFT": ret == LV_KEY_RIGHT ? "LV_KEY_RIGHT": ret == LV_KEY_UP ? "LV_KEY_UP": ret == LV_KEY_DOWN ? "LV_KEY_DOWN": ret == LV_KEY_ESC ? "LV_KEY_ESC": ret == LV_KEY_DEL ? "LV_KEY_DEL": "");
	return ret;
}

static void processButton(int b, int ignore) {
	logicKey(rotateKey(b, ignore));
}

//UP, DOWN
//ESC, DEL (long press)
//RIGHT, LEFT
//keyboard: 65, 66, 27, 255, 67, 68
//KEY_: 103, 108, 1, 111, 106, 105
//LV_KEY_: 17, 18, 27, 127, 19, 20
void processInput(unsigned char c) {
	char szz[128];
	PRINTF("processInput %d\n", c);
	switch (c) {
	case 27:
		processButton(KEY_ESC, 1);
		break;
	case 65:
		processButton(KEY_UP, 1);
		break;
	case 66:
		processButton(KEY_DOWN, 1);
		break;
	case 67:
		processButton(KEY_RIGHT, 1);
		break;
	case 68:
		processButton(KEY_LEFT, 1);
		break;
	case 255:
		processButton(KEY_DELETE, 1);
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

void backendInit(int daemon) {
	doLoop = 1;
	logicWelcome();
#ifndef WEB
	int fdStdin = -1;
	int fdButton = -1;
	if (!daemon) {
		enterInputMode();
		fdStdin = fileno(stdin);
	}
#ifndef DESKTOP
	fdButton = open(BUTTON_PATH, O_RDONLY);
#endif
	pollfd[0].fd = fdStdin;
	pollfd[0].events = POLLIN;
	pollfd[1].fd = fdButton;
	pollfd[1].events = POLLIN;
#endif
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

void backendLoop() {
	lv_tick_inc(tickGet());
	uint32_t time_till_next = lv_timer_handler();
	//PRINTF("Timer: %dms\n", time_till_next);
	if (time_till_next == LV_NO_TIMER_READY)
		time_till_next = LV_DEF_REFR_PERIOD;
#ifdef WEB
	usleep(1000 * time_till_next);
#else
	int ret = poll(pollfd, 2, time_till_next);
	if (doLoop == 0)
		return;
	if (pollfd[0].revents & POLLIN) {
		unsigned char c[2];
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
					if (ev[i].code == KEY_UP)
						ev[i].code = KEY_ESC;
					if (ev[i].code == KEY_DOWN)
						ev[i].code = KEY_DELETE;
					processButton(ev[i].code, 0);
					longPressDone = 1;
				} else if (ev[i].value == 0 && longPressDone == 0)
					processButton(ev[i].code, 0);
			}
	}
#endif
	static int count = 0;
	if (count++ % 30 == 0)
		uiUpdate();
}

void backendUninit(int daemon) {
#ifndef WEB
	if (!daemon)
		leaveInputMode();
#endif
}
