#ifndef SCREENBACKEND_H
#define SCREENBACKEND_H

//Global functions
void screenInit_();
void screenRect_(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void screenPixel_(uint8_t x, uint8_t y, uint16_t color);

#endif
