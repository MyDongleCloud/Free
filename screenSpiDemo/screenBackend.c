#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "macro.h"

#define PP "/dev/mydonglecloud_screen/%s"

#define ONLYSPI
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

#ifdef ONLYSPI
void GPIOWD(int v) {
	writeValueKey(PP, "wd", v == 1 ? "1" : "0");
}

void SPI(uint8_t l, uint8_t a, uint8_t b) {
	char sz[5];
	if (l == 1)
		sprintf(sz, "%02x", a);
	else
		sprintf(sz, "%02x%02x", a, b);
	writeValueKey(PP, "spi", sz);
}

void WAIT_100MS() {
	usleep(1000 * 100);
}

#include "../screenAvr/screenSimple.h"
#else
void screenInit_() {
	writeValueKey(PP, "init", "1");
}

void screenRect_(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
	char sz[64];
	sprintf(sz, "%d %d %d %d %d", x, y, w, h, color);
	writeValueKey(PP, "rect", sz);
}

void screenPixel_(uint8_t x, uint8_t y, uint16_t color) {
	screenRect_(x, y, 1, 1, color);
}
#endif
