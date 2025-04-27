#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "macro.h"
#include "lvgl.h"
#include "src/drivers/libinput/lv_libinput.h"
#include "backend.h"

//Define
//#define NOMMAP

//Global variable
unsigned char *fbPublic;

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
	fbPublic = (unsigned char *)malloc(WIDTH * HEIGHT * DEPTH);
#else
	int screenFile = open(SCREEN_FILE, O_RDWR);
	if (screenFile)
		fbPublic = mmap(NULL, WIDTH * HEIGHT * DEPTH, PROT_WRITE | PROT_READ, MAP_SHARED, screenFile, 0);
#endif
	writeValueKey(SCREEN_PATH, "init", "1");
}

void backendLoop_() {
	backendWork();
}

static unsigned int convert24to16(unsigned char r, unsigned char g, unsigned char b) {
	return ((r >> 3) << 11) | ((g >> 2) << 5) | ((b >> 3) << 0);
}

void backendUpdate_(int x, int y, int w, int h, unsigned char *colorp) {
	char sz[64];
#ifdef NOMMAP
	int xx, yy;
	for (yy = 0; yy < h; yy++)
		for (xx = 0; xx < w; xx++) {
			int pos = DEPTH * (yy * w + xx);
			unsigned int c = convert24to16(fb[pos + 0], fb[pos + 1], fb[pos + 2]);
			sprintf(sz, "%d %d %d %d %d", x + xx, y + yy, 1, 1, c);
			writeValueKey(SCREEN_PATH, "rect", sz);
		}
#else
	sprintf(sz, "%d %d %d %d", x, y, w, h);
	writeValueKey(SCREEN_PATH, "update", sz);
#endif
}

void backendUninit_() {}

void backendInitPointer() {}
