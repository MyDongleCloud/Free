/*
     VDD 1|‾‾‾‾‾‾‾|8 GND
     PA6 2|      |7 PA3 CLK
     PA7 3|      |6 PA0 NRST/UPDI/CS
MOSI PA1 4|______|5 PA2 MISO // WD
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#define PA0_CS PIN0_bm
#define PA1_MOSI PIN1_bm
#define PA2_WD PIN2_bm
#define PA3_CLK PIN3_bm
#define PA6_LCD PIN6_bm
#define PA7_INT PIN7_bm

void GPIOWD(int v) {
	if (v)
		PORTA.OUTSET = PA2_WD;
	else
		PORTA.OUTCLR = PA2_WD;
}

void SPI(uint8_t l, uint8_t a, uint8_t b) {
	PORTA.OUTCLR = PA0_CS;
	SPI0.DATA = a;
	while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0) ;
	if (l == 2) {
		SPI0.DATA = b;
		while ((SPI0.INTFLAGS & SPI_RXCIF_bm) == 0) ;
	}
	PORTA.OUTSET = PA0_CS;
}

void WAIT_100MS() {
	_delay_ms(100);
}

#include "screenSimple.h"

uint8_t loop = 1;
ISR(RTC_PIT_vect) {
	loop = 0;
	RTC.PITINTFLAGS = RTC_PI_bm;
}

int main() {
	PORTA.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN1CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN2CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN3CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN6CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTA.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc;

	PORTA.DIRSET = PA1_MOSI | PA3_CLK | PA0_CS;
	SPI0.CTRLA |= SPI_MASTER_bm; // Select master mode
	SPI0.CTRLA |= SPI_PRESC_DIV4_gc; // Select Prescaler of 2
	SPI0.CTRLA |= SPI_CLK2X_bm; // Enable double clock speed
	SPI0.CTRLB |= SPI_SSD_bm; // Disable slave select
	SPI0.CTRLB |= SPI_MODE_3_gc; // Select SPI Mode 3
	SPI0.CTRLA |= SPI_ENABLE_bm; // Enable SPI

	PORTA.DIRSET |= PA2_WD | PA6_LCD;

	PORTA.DIRSET &= ~PA7_INT;
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;				// 1024 Hz from OSCULP32K
	RTC.CTRLA = RTC_RTCEN_bm;					// enable RTC
	RTC.PITINTCTRL = RTC_PI_bm;					// enable periodic interrupt
	RTC.PITCTRLA = RTC_PERIOD_CYC16384_gc | RTC_PITEN_bm;	
    cli();
    sei();

	screenInit_();
	while (loop)
		screenRect_(0, 0, 128, 128, 64938);
	return 0;
}
