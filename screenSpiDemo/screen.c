#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "macro.h"
#include "font.h"
#include "screenBackend.h"
#include "screen.h"

//Functions
void screenInit() {
    screenInit_();
}

static void screenPixel(uint8_t x, uint8_t y, uint16_t color) {
    screenPixel_(x, y, color);
}

void screenRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
    screenRect_(x, y, w, h, color);
}

void screenLine(int x0, int y0, int x1, int y1, uint8_t thickness, uint16_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for(;;) {
        screenRect(x0, y0, thickness, thickness, color);
        //screenPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

static void screenChar(uint8_t x, uint8_t y, const char c, uint16_t color, uint8_t size) {
    uint8_t i, line, j;
    for (i = 0; i < 5; i++ ) {
        line = font[(c * 5) + i];
        for (j = 0; j < 8; j++) {
            if (line & 0x1) {
                if (size == 1) // default size
                    screenPixel(x + i, y + j, color);
                else  // big size
                    screenRect(x + i * size, y + j * size, size, size, color);
            }
            line >>= 1;
        }
    }
}

static void screenString_(uint8_t x, uint8_t y, const char *c, uint16_t color, uint8_t size) {
    while (c[0] != 0) {
        screenChar(x, y, c[0], color, size);
        x += size * 6;
        c++;
        if (x + 5 >= 128) {
            y += 10;
            x = 0;
        }
    }
}

void screenString(uint8_t y, char *c, uint16_t color, uint8_t size) {
    uint8_t len = 0;
    char *d = c;
    while (d[0] != 0) {
        len += size * 6;
        d++;
    }
    screenString_(64 - (len / 2), y, c, color, size);
}

static void screenBattery(uint8_t level, uint8_t soc) {
    screenLine(4, 2, 30, 2, 1, 0xffff);
    screenLine(4, 12, 30, 12, 1, 0xffff);
    screenLine(4, 2, 4, 12, 1, 0xffff);
    screenLine(30, 3, 33, 3, 1, 0xffff);
    screenLine(30, 11, 33, 11, 1, 0xffff);
    screenLine(34, 4, 34, 10, 1, 0xffff);

    uint16_t c = 0xff00;
    if (level == 4)
        c = 0x07e0;
    else if (level == 3)
        c = 0xff00;
    else if (level == 2)
        c = 0xff00;
    else if (level == 1)
        c = 0xf800;

    if (soc == 0 && level > 0) {
        screenRect(24, 4, 5, 7, level >= 4 ? c : BACKGROUNDCOLOR2);
        screenRect(18, 4, 5, 7, level >= 3 ? c : BACKGROUNDCOLOR2);
        screenRect(12, 4, 5, 7, level >= 2 ? c : BACKGROUNDCOLOR2);
        screenRect(6, 4, 5, 7, c);
    }
    if (soc > 0) {
        char sz[8];
        sprintf(sz, "%d%%", soc);
        screenString_(soc >= 100 ? 6 : 10, 4, sz, 0xffff, 1);
    }
}

void screenWait(uint8_t level, uint8_t soc, uint8_t temp) {
    screenRect(0, 0, 128, 128, BACKGROUNDCOLOR);
    screenRect(0, 0, 128, 16, BACKGROUNDCOLOR2);
    screenRect(0, 112, 128, 16, BACKGROUNDCOLOR2);
    char sz[12];
    sprintf(sz, "T:%d C", temp);
    screenString_(50, 4, "v2.0", 0xffff, 1);
    screenString_(88, 4, sz, 0xffff, 1);
    screenRect(113, 4, 2, 2, 0xffff);
    screenString(40, "Please", 0xffff, 2);
    screenString(65, "wait...", 0xffff, 2);
    screenBattery(level, soc);
    screenString(116, "Firmware:" VERSIONLINE, 0xffff, 1);
}
