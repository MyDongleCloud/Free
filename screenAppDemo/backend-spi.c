#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "macro.h"
#include "lvgl.h"
#include "src/drivers/libinput/lv_libinput.h"

//Defines
//#define NOMMAP
#define _PATH "/dev/mydonglecloud_screen/%s"
#define _DEV "/dev/mydonglecloud_screen_f"

//Global variable
unsigned char *fb;

//Functions
static void writeValue(const char *path, const char *v) {
	int fd = open(path, O_WRONLY | O_CREAT, 0644);
	if (fd >= 0) {
		write(fd, v, strlen(v));
		close(fd);
	}
}

static void writeValueKey(const char *path, const char *key, const char *v) {
	char fullpath[256];
	sprintf(fullpath, path, key);
	writeValue(fullpath, v);
}

void backendInit_(int argc, char *argv[]) {
#ifdef NOMMAP
	fb = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);
#else
	int screenFile = open(_DEV, O_RDWR);
	if (screenFile)
		fb = mmap(NULL, WIDTH * HEIGHT * DEPTH, PROT_WRITE | PROT_READ, MAP_SHARED, screenFile, 0);
#endif
	writeValueKey(_PATH, "init", "1");
}

void backendRun_() {
}

static unsigned int convert24to16(unsigned char r, unsigned char g, unsigned char b) {
	return ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
}

void backendUpdate(int x, int y, int w, int h, unsigned char *colorp) {
	char sz[64];
#ifdef NOMMAP
	int xx, yy;
	for (yy = 0; yy < h; yy++)
		for (xx = 0; xx < w; xx++) {
			int pos = DEPTH * (yy * w + xx);
			unsigned int c = convert24to16(fb[pos + 0], fb[pos + 1], fb[pos + 2]);
			sprintf(sz, "%d %d %d %d %d", x + xx, y + yy, 1, 1, c);
			writeValueKey(_PATH, "rect", sz);
		}
#else
	sprintf(sz, "%d %d %d %d", x, y, w, h);
	writeValueKey(_PATH, "update", sz);
#endif
}

lv_indev_t * backendInitPointer() {
	return NULL;
}

lv_indev_t * backendInitKeyboard() {
	lv_indev_t *indevK = lv_libinput_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
	lv_indev_set_type(indevK, LV_INDEV_TYPE_KEYPAD);
	return indevK;
}
