/*
     VDD 1|‾‾‾‾‾‾‾|8 GND
WD   PA6 2|      |7 PA3 CLK
CS   PA7 3|      |6 PA0 NRST/UPDI
MOSI PA1 4|______|5 PA2 MISO
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 1000000UL
#include <util/delay.h>

#define PA1_MOSI PIN1_bm
#define PA2_MISO PIN2_bm
#define PA3_CLK PIN3_bm
#define PA6_WD PIN6_bm
#define PA7_CS PIN7_bm

uint8_t loop = 1;

void GPIOWD(int v) {
	if (v)
		PORTA.OUTSET = PA6_WD;
	else
		PORTA.OUTCLR = PA6_WD;
}

void SPI(uint8_t l, uint8_t a, uint8_t b) {
	PORTA.OUTCLR = PA7_CS;
	SPI0.DATA = a;
	while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0)
		;
	if (SPI0.DATA == 0x0)
		loop = 0;
	if (l == 2) {
		SPI0.DATA = b;
		while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0)
			;
	}
	PORTA.OUTSET = PA7_CS;
}

void WAIT_100MS() {
	_delay_ms(100);
}

#include "screenSimple.h"
#define SCREENPIXEL_(a, b, c) screenPixel_(b, a, c)

void progressBar() {
	uint8_t w = 92 + 4;
	uint8_t h = 8 + 4;
	uint8_t x = (128 - w) / 2;
	uint8_t y = (128 - h) / 2;
	uint8_t xx, yy;
	for (xx = 0; xx < w; xx++) {
		SCREENPIXEL_(x + xx, y, 0xffff);
		SCREENPIXEL_(x + xx, y + h, 0xffff);
	}
	for (yy = 0; yy < h; yy++) {
		SCREENPIXEL_(x, y + yy, 0xffff);
		SCREENPIXEL_(x + w - 1, y + yy, 0xffff);
	}
	w -= 4;
	h -= 4;
	x += 2;
	y += 2;
	uint8_t pos;
	while (1)
		for (pos = 0; pos < w; pos++) {
			for (yy = 0; yy <= h; yy++) {
				SCREENPIXEL_((x + pos), y + yy, 0x0);
				uint8_t ppos = (pos + 32) % w;
				SCREENPIXEL_(x + ppos, y + yy, 0xffff);
				if (loop == 0)
					goto end;
			}
			_delay_ms(25);
		}
end:
	return;
}

int main() {
	PORTA.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN3CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc;

	PORTA.DIRSET = (PA1_MOSI | PA3_CLK | PA6_WD | PA7_CS) & ~PA2_MISO;

	SPI0.CTRLB |= SPI_MODE_0_gc; // Select SPI Mode 0
	SPI0.CTRLA |= SPI_MASTER_bm; // Select master mode
	//SPI0.CTRLA |= SPI_PRESC_DIV4_gc; // Select Prescaler of 2
	SPI0.CTRLA |= SPI_CLK2X_bm; // Enable double clock speed
	SPI0.CTRLA |= SPI_ENABLE_bm; // Enable SPI
	SPI0.CTRLB |= SPI_SSD_bm; // Disable slave select

	_delay_ms(1000);
	screenInit_();
	screenRect_(0, 0, 128, 128, 0);
	progressBar();
	SPI0.CTRLA = 0;
	PORTA.DIR = 0;
	return 0;
}
