static void screenCommand(uint8_t cmd) {
	GPIOWD(0);
	SPI(1, cmd, 0);
	GPIOWD(1);
}

static void screenData(uint8_t data) {
	SPI(1, data, 0);
}

void screenInit_() {
    screenCommand(0x11); //Sleep out
    WAIT_100MS();
    screenCommand(0xB1);
    screenData(0x01);
    screenData(0x2c);
    screenData(0x2d);
    screenCommand(0xB2);
    screenData(0x02);
    screenData(0x35);
    screenData(0x36);
    screenCommand(0xB3);
    screenData(0x01);
    screenData(0x2c);
    screenData(0x2d);
    screenData(0x01);
    screenData(0x2c);
    screenData(0x2d);
    screenCommand(0xB4); //Dot inversion
    screenData(0x03);
    screenCommand(0xC0);
    screenData(0xa2);
    screenData(0x02);
    screenData(0x84);
    screenCommand(0xC1);
    screenData(0XC5);
    screenCommand(0xC2);
    screenData(0x0a);
    screenData(0x00);
    screenCommand(0xC3);
    screenData(0x8a);
    screenData(0x2A);
    screenCommand(0xC4);
    screenData(0x8a);
    screenData(0xEE);
    screenCommand(0xC5); //VCOM
    screenData(0x0e);
    screenCommand(0x36); //MX, MY, RGB mode
    screenData(0xc8); //0xa8 \BA\E1\C6\C1
    screenCommand(0xE0);
    screenData(0x0f);
    screenData(0x1a);
    screenData(0x0f);
    screenData(0x18);
    screenData(0x2f);
    screenData(0x28);
    screenData(0x20);
    screenData(0x22);
    screenData(0x1f);
    screenData(0x1b);
    screenData(0x23);
    screenData(0x37);
    screenData(0x00);
    screenData(0x07);
    screenData(0x02);
    screenData(0x10);
    screenCommand(0xE1);
    screenData(0x0f);
    screenData(0x1b);
    screenData(0x0f);
    screenData(0x17);
    screenData(0x33);
    screenData(0x2c);
    screenData(0x29);
    screenData(0x2e);
    screenData(0x30);
    screenData(0x39);
    screenData(0x3f);
    screenData(0x3B);
    screenData(0x00);
    screenData(0x01);
    screenData(0x04);
    screenData(0x13);
    screenCommand(0x3A); //65k mode
    screenData(0x05);

    //screenRect(0, 0, 128, 128, 0xffff);

    screenCommand(0x29); //Display on
}

void screenArea_(int xs, int xe, int ys, int ye) {
    screenCommand(0x2B);
    screenData(0);
    screenData(xs);
    screenData(0);
    screenData(xe);

    screenCommand(0x2A);
    screenData(0);
    screenData(ys);
    screenData(0);
    screenData(ye);

    screenCommand(0x2C);
}

void screenPixel_(uint8_t x, uint8_t y, uint16_t color) {
    screenArea_(32 + y, 32 + y, x, x);
	SPI(2, color >> 8, color);
}

void screenRect_(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
    screenArea_(32 + y, 32 + y + h - 1, x, x + w - 1);
	uint16_t c;
    for (c = 0; c < w * h; c++)
		SPI(2, color >> 8, color);
}
