/*
     VDD 1|‾‾‾‾‾‾‾|8 GND
WD   PA6 2|      |7 PA3 CLK
CS   PA7 3|      |6 PA0 NRST/UPDI/CS
MOSI PA1 4|______|5 PA2 MISO // WD
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

void GPIOWD(int v) {
	if (v)
		PORTA.OUTSET = PA6_WD;
	else
		PORTA.OUTCLR = PA6_WD;
}

void SPI(uint8_t l, uint8_t a, uint8_t b) {
	PORTA.OUTCLR = PA7_CS;
	SPI0.DATA = a;
	while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0) ;
	if (l == 2) {
		SPI0.DATA = b;
		while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0) ;
	}
	PORTA.OUTSET = PA7_CS;
}

void WAIT_100MS() {
	_delay_ms(100);
}

#include "screenSimple.h"

uint8_t loop = 1;

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

#if 0
	int i = 100;
	while (i-- != 1) {
		GPIOWD(1);
		PORTA.OUTSET = PA7_CS;
		_delay_ms(10);
		GPIOWD(0);
		PORTA.OUTCLR = PA7_CS;
		_delay_ms(10);
	}
#endif

	_delay_ms(5000);
	screenInit_();

	int count = 0;
	while (loop)
		screenRect_(0, 0, 128, 128, (count++ % 2) ? 64938 : 0);
	return 0;
}
