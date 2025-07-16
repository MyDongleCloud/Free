#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/mman.h>
#include "macro.h"
#include "sbl.h"
#include "sbl2.h"

//defines
#ifndef B1500000
#define B1500000 B230400
#endif
#define MAXRETRIES 256
#define NACK 0x33
#define ACK 0xcc
#define COMMAND_RESET 0x25
#define COMMAND_GET_ID 0x28

//Global variable
int CCflashInProgress = 0;

//Private variables
static int fdUART = 0;
static struct termios ttyOrig;

//Private basic functions
static int set_interface_attribs_(int fd, int speed) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
		PRINTF("error %d from tcgetattr\n", errno);
		return -1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY | INLCR | ICRNL | IGNBRK);
	tty.c_cflag |= (CLOCAL | CREAD);
	tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		PRINTF("error %d from tcsetattr\n", errno);
		return -1;
	}
	return 0;
}

int openUART(char *sz, int speed) {
	int fd = open(sz, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd <= 0) {
		PRINTF("Error opening UART %s\n", sz);
	} else {
		tcgetattr(fd, &ttyOrig);
		set_interface_attribs_(fd, speed);
		//PRINTF("UART %s opened at %d\n", sz, speed);
	}
	return fd;
}

static void sblInit_(char *sz) {
	fdUART = openUART(sz, B1500000);
}

static void writeValue(const char *path, const char *v) {
	int fd = open(path, O_WRONLY | O_CREAT, 0644);
	if (fd >= 0) {
		write(fd, v, strlen(v));
		close(fd);
	}
}

static int sblRead_(unsigned char *a, int l) {
	int re = l;
	int tries = 3;
	while (re > 0 && tries-- > 0) {
		int ret = read(fdUART, a + l - re, re);
		if (ret > 0)
			re -= ret;
		usleep(10);
	}
	return re != 0;
}

static void sblWrite_(unsigned char *b, int l) {
	write(fdUART, b, l);
}

static unsigned char sblChecksum(uint32_t ui32Cmd, const char *pcSendData, uint32_t ui32SendLen) {
	unsigned char c = ui32Cmd;
	int i;
	for(i = 0; i < ui32SendLen; i++)
		c += pcSendData[i];
	return c;
}

static void CCComReset() {
#if 0
	int fdMem = open("/dev/mem", O_RDWR | O_SYNC);
	void *mapid, *virtid;
#define ADDRESS_ID 0xC001B000
	mapid = mmap(0, MAP32_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fdMem, ADDRESS_ID & ~MAP32_MASK);
	virtid = mapid + (ADDRESS_ID & MAP32_MASK);
	unsigned int val = REG32(virtid + 0x0);
	unsigned int val4 = REG32(virtid + 0x04);
	PRINTF("CC GPIO: val4(type) initial 0x%08x\n", val4);
	PRINTF("CC GPIO: val0(value) initial 0x%08x\n", val);
	//devmem2 0xC001B004 b 0x10
	//devmem2 0xC001B000 b 0x0
	REG32(virtid + 0x04) = val4 | 0x10;
	REG32(virtid + 0x00) = (val & 0xFFFFFFFE);
	PRINTF("CC GPIO: val4(type) changing 0x%08x\n", REG32(virtid + 0x4));
	PRINTF("CC GPIO: val0(value) changing 0x%08x\n", REG32(virtid + 0x0));

	int fd = open(CC_RESET, O_WRONLY | O_CREAT, 0644);
	char one[2] = "1";
	if (fd >= 0) {
		write(fd, one, 1);
		close(fd);
	}

	REG32(virtid + 0x04) = val4;
	REG32(virtid + 0x0) = val;
	PRINTF("CC GPIO: val4(type) finish 0x%08x\n", REG32(virtid + 0x4));
	PRINTF("CC GPIO: val0(value) finish 0x%08x\n", REG32(virtid + 0x0));
	munmap(mapid, MAP32_SIZE);
	if (fdMem)
		close(fdMem);
#endif
}

//Public functions
bool CCinitCommunication(char *szUart, int resetViaGPIO) {
	if (resetViaGPIO)
		CCComReset();
	sblInit_(szUart);
	usleep(10 * 1000);
	PRINTF("sendCmd Dummy ");
	unsigned char setup[2] = {0x55, 0x55};
	sblWrite_(setup, 2);
	bool bAck;
	uint32_t ret = getCmdResponse(&bAck, MAXRETRIES, false);
	PRINTF("%s\n", ret == 1 ? "Nothing" : bAck ? "OK" : "Error");
	return bAck;
}

void CCuninitCommunication() {
	if (fdUART > 0) {
		tcsetattr(fdUART, TCSANOW, &ttyOrig);
		close(fdUART);
	}
}

bool CCFlash(char *szUart, char *szFirmware, char *szFirmware3, int resetViaGPIO) {
	CCflashInProgress = 1;
	bool ret = false;
	if (CCinitCommunication(szUart, resetViaGPIO)) {
		usleep(1000 * 100);
		CCwriteFirmware(szFirmware, szFirmware3, 1, NULL);
		CCreset();
		ret = true;
	}
	CCuninitCommunication();
	CCflashInProgress = 0;
	return ret;
}

uint32_t getCmdResponse(bool *bAck, uint32_t ui32MaxRetries, bool bQuiet) {
	*bAck = false;
	unsigned char *a = malloc(2);
	if (sblRead_(a, 2))
		return 1;
	if (a[1] != ACK)
		PRINTF(" 0x%x ", a[1]);
	*bAck = a[1] == ACK ? true : false;
	return 0;
}

uint32_t sendCmdResponse(bool bAck) {
	unsigned char r[] = {0, 0};
	r[1] = bAck ? ACK : NACK;
	sblWrite_(r, 2);
	return 0;
}

uint32_t getResponseData(unsigned char *pcData, uint32_t ui32MaxLen, uint32_t ui32MaxRetries) {
	unsigned char *res = malloc(2 + ui32MaxLen);
	if (sblRead_(res, 2 + ui32MaxLen))
		return 1;
	unsigned char *s = res;
	unsigned char *c = res + 1;
	unsigned char *a = res + 2;
	memcpy(pcData, a, ui32MaxLen);
	if (c[0] != sblChecksum(0, a, ui32MaxLen))
		PRINTF("getResponseData size:%d==%d+2 checksum:0x%x==0x%x\n", s[0], ui32MaxLen, c[0], sblChecksum(0, a, ui32MaxLen));
	return c[0] != sblChecksum(0, a, ui32MaxLen);
}

uint32_t sendcmd(uint32_t ui32Cmd, unsigned char *pcSendData, uint32_t ui32SendLen) {
	//PRINTF("sendCmd 0x%x\n", ui32Cmd);
	unsigned char *b = malloc(3 + ui32SendLen);
	b[0] = 3 + ui32SendLen;
	b[1] = sblChecksum(ui32Cmd, pcSendData, ui32SendLen);
	b[2] = ui32Cmd;
	if (ui32SendLen > 0)
		memcpy(b + 3, pcSendData, ui32SendLen);
	sblWrite_(b, b[0]);
	free(b);
	return 0;
}
