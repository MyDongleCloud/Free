#include <stdlib.h>
#include <avr/io.h>
#include <stdint.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void spi_init_master() {
	PORTA.DIRSET = PIN1_bm|PIN3_bm|PIN4_bm; // Set data direction for MOSI, SCK and SS pins
	SPI0.CTRLA |= SPI_MASTER_bm; // Select master mode
	SPI0.CTRLA |= SPI_PRESC_DIV4_gc; // Select Prescaler of 2
	SPI0.CTRLA |= SPI_CLK2X_bm; // Enable double clock speed
	SPI0.CTRLB |= SPI_SSD_bm; // Disable slave select
	SPI0.CTRLB |= SPI_MODE_3_gc; // Select SPI Mode 3
	SPI0.CTRLA |= SPI_ENABLE_bm; // Enable SPI
}

void GPIOWD(int v) {
	if (v)
		;
	else
		;
}

void SPI(uint8_t l, uint8_t a, uint8_t b) {
	PORTA.OUTCLR = PIN4_bm; //set SS low
	SPI0.DATA = a;
	while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0) ;
	if (l == 2) {
		SPI0.DATA = b;
		while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0) ;
	}
	PORTA.OUTSET = PIN4_bm; //set SS high
}

void WAIT_100MS() {
	_delay_ms(100);
}

#include "screenSimple.h"

int main() {
	spi_init_master();
	screenInit_();
	screenRect_(0, 0, 128, 128, 64938);
	return 0;
}
