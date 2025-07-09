#ifdef DeviceFamily_CC13X2
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "macro.h"
#include "sbl.h"
#include "sbl2.h"

#define SBL_MAX_DEVICES 20
#define SBL_DEFAULT_RETRY_COUNT 1
#define SBL_DEFAULT_READ_TIMEOUT 100 // in ms
#define SBL_DEFAULT_WRITE_TIMEOUT 200 // in ms
#define SBL_CC2650_PAGE_ERASE_SIZE (isM4 ? 8192 : 4096)
#define SBL_CC2650_FLASH_START_ADDRESS 0x00000000
#define SBL_CC2650_RAM_START_ADDRESS 0x20000000
#define SBL_CC2650_ACCESS_WIDTH_32B 1
#define SBL_CC2650_ACCESS_WIDTH_8B 0
#define SBL_CC2650_PAGE_ERASE_TIME_MS 20
#define SBL_CC2650_MAX_BYTES_PER_TRANSFER 252
#define SBL_CC2650_MAX_MEMWRITE_BYTES		247
#define SBL_CC2650_MAX_MEMWRITE_WORDS		61
#define SBL_CC2650_MAX_MEMREAD_BYTES		253
#define SBL_CC2650_MAX_MEMREAD_WORDS		63
#define SBL_CC2650_FLASH_SIZE_CFG 0x4003002C
#define SBL_CC2650_RAM_SIZE_CFG 0x40082250
#define SBL_CC2650_BL_CONFIG_PAGE_OFFSET (isM4 ? 0x1FDB : 0xFDB)
#define SBL_CC2650_BL_CONFIG_ENABLED_BM 0xC5
#define SBL_CC2650_BL_WORK_MEMORY_START		0x20000000
#define SBL_CC2650_BL_WORK_MEMORY_END		0x2000016F
#define SBL_CC2650_BL_STACK_MEMORY_START	0x20000FC0
#define SBL_CC2650_BL_STACK_MEMORY_END		0x20000FFF

static int isM4 = -1;

void selectM4(int i) {
	isM4 = i;
	PRINTF("Configured for %s chip\n", isM4 == -1 ? "Unknown" : isM4 == 1 ? "M4" : "M3");
}

typedef enum {
	SBL_SUCCESS = 0,
	SBL_ERROR,
	SBL_ARGUMENT_ERROR,
	SBL_TIMEOUT_ERROR,
	SBL_PORT_ERROR,
	SBL_ENUM_ERROR,
	SBL_UNSUPPORTED_FUNCTION,
} tSblStatus;

enum {
	CMD_PING = 0x20,
	CMD_DOWNLOAD = 0x21,
	CMD_GET_STATUS = 0x23,
	CMD_SEND_DATA = 0x24,
	CMD_RESET = 0x25,
	CMD_SECTOR_ERASE = 0x26,
	CMD_CRC32 = 0x27,
	CMD_GET_CHIP_ID = 0x28,
	CMD_MEMORY_READ = 0x2A,
	CMD_MEMORY_WRITE = 0x2B,
	CMD_BANK_ERASE = 0x2C,
	CMD_SET_CCFG = 0x2D,
};

enum {
	CMD_RET_SUCCESS = 0x40,
	CMD_RET_UNKNOWN_CMD = 0x41,
	CMD_RET_INVALID_CMD = 0x42,
	CMD_RET_INVALID_ADR = 0x43,
	CMD_RET_FLASH_FAIL = 0x44,
};

typedef struct {
	uint32_t startAddr;
	uint32_t byteCount;
	uint32_t startOffset;
	bool	 bExpectAck;
} tTransfer;

uint32_t m_deviceId;
uint32_t m_flashSize = 0;
uint32_t m_ramSize;

static uint32_t readMemory32(uint32_t ui32StartAddress, uint32_t ui32UnitCount, uint32_t *pui32Data);
static uint32_t addressToPage(uint32_t ui32Address);
static bool addressInRam(uint32_t ui32StartAddress, uint32_t ui32ByteCount);
static bool addressInFlash(uint32_t ui32StartAddress, uint32_t ui32ByteCount);

static uint32_t charArrayToUL(unsigned char *pcSrc) {
	uint32_t ui32Val = (unsigned char)pcSrc[3];
	ui32Val += (((unsigned long)pcSrc[2]) & 0xFF) << 8;
	ui32Val += (((unsigned long)pcSrc[1]) & 0xFF) << 16;
	ui32Val += (((unsigned long)pcSrc[0]) & 0xFF) << 24;
	return (ui32Val);
}

static void ulToCharArray(const uint32_t ui32Src, unsigned char *pcDst) {
	// MSB first
	pcDst[0] = (uint8_t)(ui32Src >> 24);
	pcDst[1] = (uint8_t)(ui32Src >> 16);
	pcDst[2] = (uint8_t)(ui32Src >> 8);
	pcDst[3] = (uint8_t)(ui32Src >> 0);
}

static void setProgress(uint32_t ui32Progress) {
	if (ui32Progress != 0 && ui32Progress != 100) {
		PRINTF_("\rProgess %d%%", ui32Progress);
	} else if (ui32Progress == 100)
		PRINTF("\r");
}

static uint32_t getFlashSize() {
	if (noRxCCext)
		return isM4 ? 1024 * 352 : 1024 * 128;
	return m_flashSize;
}

static uint32_t getRamSize() {
	if (noRxCCext)
		return isM4 ? 1024 * 80 : 1024 * 2;
	return m_ramSize;
}

//-----------------------------------------------------------------------------
/** \brief This function returns a string with the device status name of
 *	 \e ui32Status serial bootloader status value.
 *
 * \param[out] ui32Status
 *	 The serial bootloader status value.
 * \return
 *	 Returns const char * with name of device status.
 */
//-----------------------------------------------------------------------------
static char *getCmdStatusString(uint32_t ui32Status) {
	switch(ui32Status) {
	case CMD_RET_FLASH_FAIL: return "FLASH_FAIL";
	case CMD_RET_INVALID_ADR: return "INVALID_ADR";
	case CMD_RET_INVALID_CMD: return "INVALID_CMD";
	case CMD_RET_SUCCESS:	 return "SUCCESS";
	case CMD_RET_UNKNOWN_CMD: return "UNKNOWN_CMD";
	default: return "Unknown status";
	}
}


//-----------------------------------------------------------------------------
/** \brief This function sends the CC2650 download command and handles the
 *	 device response.
 *
 * \param[in] ui32Address
 *	 The start address in CC2650 flash.
 * \param[in] ui32ByteCount
 *	 The total number of bytes to program on the device.
 *
 * \return
 *	 Returns SBL_SUCCESS if command and response was successful.
 */
//-----------------------------------------------------------------------------
static uint32_t cmdDownload(uint32_t ui32Address, uint32_t ui32Size) {
	int retCode = SBL_SUCCESS;
	bool bSuccess = false;


	//
	// Check input arguments
	//
	if(!addressInFlash(ui32Address, ui32Size)) {
		PRINTF("ARGUMENT_ERROR: Flash download: Address range (0x%08X + %d bytes) is not in device FLASH nor RAM.\n", ui32Address, ui32Size);
		return SBL_ARGUMENT_ERROR;
	}
	if(ui32Size & 0x03) {
		PRINTF("ARGUMENT_ERROR: Flash download: Byte count must be a multiple of 4\n");
		return SBL_ARGUMENT_ERROR;
	}

	//
	// Generate payload
	// - 4B Program address
	// - 4B Program size
	//
	unsigned char pcPayload[8];
	ulToCharArray(ui32Address, &pcPayload[0]);
	ulToCharArray(ui32Size, &pcPayload[4]);

	//
	// Send command
	//
	if((retCode = sendcmd(CMD_DOWNLOAD, pcPayload, 8)) != SBL_SUCCESS) {
		return retCode;
	}

	//
	// Receive command response (ACK/NAK)
	//
	if((retCode = getCmdResponse(&bSuccess, SBL_DEFAULT_RETRY_COUNT, false)) != SBL_SUCCESS) {
		return retCode;
	}

	//
	// Return command response
	//
	return (bSuccess) ? SBL_SUCCESS : SBL_ERROR;
}


//-----------------------------------------------------------------------------
/** \brief This function sends the CC2650 SendData command and handles the
 *	 device response.
 *
 * \param[in] pcData
 *	 Pointer to the data to send.
 * \param[in] ui32ByteCount
 *	 The number of bytes to send.
 *
 * \return
 *	 Returns SBL_SUCCESS if command and response was successful.
 */
//-----------------------------------------------------------------------------
static uint32_t cmdSendData(unsigned char *pcData, uint32_t ui32ByteCount) {
	uint32_t retCode = SBL_SUCCESS;
	bool bSuccess = false;

	//
	// Check input arguments
	//
	if(ui32ByteCount > SBL_CC2650_MAX_BYTES_PER_TRANSFER) {
		PRINTF("ERROR: Error: Byte count (%d) exceeds maximum transfer size %d.\n", ui32ByteCount, SBL_CC2650_MAX_BYTES_PER_TRANSFER);
		return SBL_ERROR;
	}

	//
	// Send command
	//
	if((retCode = sendcmd(CMD_SEND_DATA, pcData, ui32ByteCount)) != SBL_SUCCESS) {
		return retCode;
	}

	//
	// Receive command response (ACK/NAK)
	//
	if((retCode = getCmdResponse(&bSuccess, 3, false)) != SBL_SUCCESS) {
		return retCode;
	}
	if(!bSuccess) {
		return SBL_ERROR;
	}

	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief This function sends ping command to device.
 *
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
uint32_t ping() {
	int retCode = SBL_SUCCESS;
	bool bResponse = false;

	//
	// Send command
	//
	if((retCode = sendcmd(CMD_PING, NULL, 0)) != SBL_SUCCESS) {
		return retCode;
	}

	//
	// Get response
	//
	if((retCode = getCmdResponse(&bResponse, SBL_DEFAULT_RETRY_COUNT, false)) != SBL_SUCCESS) {
		return retCode;
	}

	return (bResponse) ? SBL_SUCCESS : SBL_ERROR;
}


//-----------------------------------------------------------------------------
/** \brief This function gets status from device.
 *
 * \param[out] pStatus
 *	 Pointer to where status is stored.
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
static uint32_t readStatus(uint32_t *pui32Status) {
	uint32_t retCode = SBL_SUCCESS;
	bool bSuccess = false;

	//
	// Send command
	//
	if((retCode = sendcmd(CMD_GET_STATUS, NULL, 0)) != SBL_SUCCESS) {
		return retCode;
	}

	//
	// Receive command response
	//
	if((retCode = getCmdResponse(&bSuccess, SBL_DEFAULT_RETRY_COUNT, false)) != SBL_SUCCESS) {
		return retCode;
	}

	if(!bSuccess) {
		return SBL_ERROR;
	}

	//
	// Receive command response data
	//
	unsigned char status = 0;
	uint32_t ui32NumBytes = 1;
	if((retCode = getResponseData(&status, ui32NumBytes, SBL_DEFAULT_RETRY_COUNT)) != SBL_SUCCESS) {
		//
		// Respond with NAK
		//
		sendCmdResponse(false);
		return retCode;
	}

	//
	// Respond with ACK
	//
	sendCmdResponse(true);

	*pui32Status = status;
	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief This function reads device ID.
 *
 * \param[out] pui32DeviceId
 *	 Pointer to where device ID is stored.
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
static uint32_t readDeviceId(uint32_t *pui32DeviceId) {
	int retCode = SBL_SUCCESS;
	bool bSuccess = false;

	//
	// Send command
	//
	if((retCode = sendcmd(CMD_GET_CHIP_ID, NULL, 0)) != SBL_SUCCESS) {
		return retCode;
	}

	//
	// Receive command response (ACK/NAK)
	//
	if((retCode = getCmdResponse(&bSuccess, SBL_DEFAULT_RETRY_COUNT, false)) != SBL_SUCCESS) {
		return retCode;
	}
	if(!bSuccess) {
		return SBL_ERROR;
	}

	//
	// Receive response data
	//
	unsigned char pId[4];
	memset(pId, 0, 4);
	uint32_t numBytes = 4;
	if((retCode = getResponseData(pId, numBytes, SBL_DEFAULT_RETRY_COUNT)) != SBL_SUCCESS) {
		//
		// Respond with NAK
		//
		sendCmdResponse(false);
		return retCode;
	}

	if(numBytes != 4) {
			//
			// Respond with NAK
			//
			sendCmdResponse(false);
			PRINTF("ERROR: Didn't receive 4 B.\n");
			return SBL_ERROR;
		}

	//
	// Respond with ACK
	//
	sendCmdResponse(true);

	//
	// Store retrieved ID and report success
	//
	*pui32DeviceId = charArrayToUL(pId);
	m_deviceId = *pui32DeviceId;

	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief This function reads device FLASH size in bytes.
 *
 * \param[out] pui32FlashSize
 *	 Pointer to where FLASH size is stored.
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
static uint32_t readFlashSize(uint32_t *pui32FlashSize) {
	uint32_t retCode = SBL_SUCCESS;

	//
	// Read CC2650 DIECFG0 (contains FLASH size information)
	//
	uint32_t addr = SBL_CC2650_FLASH_SIZE_CFG;
	uint32_t value;
	if((retCode = readMemory32(addr, 1, &value)) != SBL_SUCCESS) {
		PRINTF("ERROR (((tSblStatus)retCode): Failed to read device FLASH size");
		return retCode;
	}
	//
	// Calculate flash size (The number of flash sectors are at bits [7:0])
	//
	value &= 0xFF;
	*pui32FlashSize = value*SBL_CC2650_PAGE_ERASE_SIZE;

	m_flashSize = *pui32FlashSize;

	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief This function reads device RAM size in bytes.
 *
 * \param[out] pui32RamSize
 *	 Pointer to where RAM size is stored.
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
static uint32_t readRamSize(uint32_t *pui32RamSize) {
	int retCode = SBL_SUCCESS;

	//
	// Read CC2650 DIECFG0 (contains RAM size information
	//
	uint32_t addr = SBL_CC2650_RAM_SIZE_CFG;
	uint32_t value;
	if((retCode = readMemory32(addr, 1, &value)) != SBL_SUCCESS) {
		PRINTF("ERROR(retCode): Failed to read device RAM size");
		return retCode;
	}

	//
	// Calculate RAM size in bytes (Ram size bits are at bits [1:0])
	//
	value &= 0x03;
	switch(value) {
	case 3: *pui32RamSize = isM4 ? 0x14000 : 0x5000; break;	// 80~20 KB
	case 2: *pui32RamSize = isM4 ? 0x10000 : 0x4000; break;	// 64~16 KB
	case 1: *pui32RamSize = isM4 ? 0xC000 : 0x2800; break;	// 48~10 KB
	case 0:
	default:*pui32RamSize = isM4 ? 0x8000 : 0x1000; break;	// 32~4 KB
	}

	//
	// Save RAM size internally
	//
	m_ramSize = *pui32RamSize;

	return retCode;
}

//-----------------------------------------------------------------------------
/** \brief This function reset the device. Communication to the device must be 
 *	 reinitialized after calling this function.
 *
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
uint32_t CCreset() {
	int retCode = SBL_SUCCESS;
	bool bSuccess = false;

	//
	// Send command
	//
	if((retCode = sendcmd(CMD_RESET, NULL, 0)) != SBL_SUCCESS) {
		return retCode;
	}

	//
	// Receive command response (ACK/NAK)
	//
	if((retCode = getCmdResponse(&bSuccess, SBL_DEFAULT_RETRY_COUNT, false)) != SBL_SUCCESS) {
		return retCode;
	}
	if(!bSuccess) {
		PRINTF("ERROR: Reset command NAKed by device.\n");
		return SBL_ERROR;
	}

	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief This function erases device flash pages. Starting page is the page 
 *	 that includes the address in \e startAddress. Ending page is the page 
 *	 that includes the address <startAddress + byteCount>. CC13/CC26xx erase 
 *	 size is 4KB.
 *
 * \param[in] ui32StartAddress
 *	 The start address in flash.
 * \param[in] ui32ByteCount
 *	 The number of bytes to erase.
 *
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
static uint32_t CCeraseFlashRange(uint32_t ui32StartAddress, uint32_t ui32ByteCount) {
	uint32_t retCode = SBL_SUCCESS;
	bool bSuccess = false;
	unsigned char pcPayload[4];
	uint32_t devStatus;

	//
	// Calculate retry count
	//
	uint32_t ui32PageCount = ui32ByteCount / SBL_CC2650_PAGE_ERASE_SIZE;
	if( ui32ByteCount % SBL_CC2650_PAGE_ERASE_SIZE) ui32PageCount ++;
	setProgress( 0 );
	uint32_t i;
	for(i = 0; i < ui32PageCount; i++) {

		//
		// Build payload
		// - 4B address (MSB first)
		//
		ulToCharArray(ui32StartAddress + i * SBL_CC2650_PAGE_ERASE_SIZE, &pcPayload[0]);

		//
		// Send command
		//
		if((retCode = sendcmd(CMD_SECTOR_ERASE, pcPayload, 4)) != SBL_SUCCESS) {
			return retCode;
		}

		//
		// Receive command response (ACK/NAK)
		//
		if((retCode = getCmdResponse(&bSuccess, SBL_DEFAULT_RETRY_COUNT, false)) != SBL_SUCCESS) {
			return retCode;
		}
		if(!bSuccess) {
			return SBL_ERROR;
		}

		//
		// Check device status (Flash failed if page(s) locked)
		//
		readStatus(&devStatus);
		if(devStatus != CMD_RET_SUCCESS) {
			PRINTF("ERROR: Flash erase failed. (Status 0x%02X = '%s'). Flash pages may be locked.\n", devStatus, getCmdStatusString(devStatus));
			return SBL_ERROR;
		}

		setProgress( 100*(i+1)/ui32PageCount );
	}
 

	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief This function reads \e ui32UnitCount (32 bit) words of data from 
 *	 device. Destination array is 32 bit wide. The start address must be 4 
 *	 byte aligned.
 *
 * \param[in] ui32StartAddress
 *	 Start address in device (must be 4 byte aligned).
 * \param[in] ui32UnitCount
 *	 Number of data words to read.
 * \param[out] pcData
 *	 Pointer to where read data is stored.
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
static uint32_t readMemory32(uint32_t ui32StartAddress, uint32_t ui32UnitCount, uint32_t *pui32Data) {
	int retCode = SBL_SUCCESS;
	bool bSuccess = false;

	//
	// Check input arguments
	//
	if((ui32StartAddress & 0x03)) {
		PRINTF("ARGUMENT_ERROR: readMemory32(): Start address (0x%08X) must be a multiple of 4.\n", ui32StartAddress);
		return SBL_ARGUMENT_ERROR;
	}

	//
	// Set progress
	//
	setProgress(0);

	unsigned char pcPayload[6];
	uint32_t responseData[SBL_CC2650_MAX_MEMREAD_WORDS];
	uint32_t chunkCount = ui32UnitCount / SBL_CC2650_MAX_MEMREAD_WORDS;
	if(ui32UnitCount % SBL_CC2650_MAX_MEMREAD_WORDS) chunkCount++;
	uint32_t remainingCount = ui32UnitCount;

	uint32_t i;
	for(i = 0; i < chunkCount; i++) {
		uint32_t dataOffset = (i * SBL_CC2650_MAX_MEMREAD_WORDS);
		uint32_t chunkStart = ui32StartAddress + dataOffset;
		uint32_t chunkSize = MIN2(remainingCount, SBL_CC2650_MAX_MEMREAD_WORDS);
		remainingCount -= chunkSize;

		//
		// Build payload
		// - 4B address (MSB first)
		// - 1B access width
		// - 1B Number of accesses (in words)
		//
		ulToCharArray(chunkStart, &pcPayload[0]);
		pcPayload[4] = SBL_CC2650_ACCESS_WIDTH_32B;
		pcPayload[5] = chunkSize;
		//
		// Set progress
		//
		setProgress(((i * 100) / chunkCount));

		//
		// Send command
		//
		if((retCode = sendcmd(CMD_MEMORY_READ, pcPayload, 6)) != SBL_SUCCESS) {
			return retCode;
		}

		//
		// Receive command response (ACK/NAK)
		//
		if((retCode = getCmdResponse(&bSuccess, SBL_DEFAULT_RETRY_COUNT, false)) != SBL_SUCCESS) {
			return retCode;
		}
		if(!bSuccess) {
			return SBL_ERROR;
		}

		//
		// Receive 4B response
		//
		uint32_t expectedBytes = chunkSize * 4;
		uint32_t recvBytes = expectedBytes;
		if((retCode = getResponseData((unsigned char *)responseData, recvBytes, SBL_DEFAULT_RETRY_COUNT)) != SBL_SUCCESS) {
			//
			// Respond with NAK
			//
			sendCmdResponse(false);
			return retCode;
		}

		if(recvBytes != expectedBytes) {
			//
			// Respond with NAK
			//
			sendCmdResponse(false);
			PRINTF("ERROR: Didn't receive 4 B.\n");
			return SBL_ERROR;
		}

		memcpy(&pui32Data[dataOffset], responseData, expectedBytes);
		//delete [] responseData;
		//
		// Respond with ACK
		//
		sendCmdResponse(true);
	}

	//
	// Set progress
	//
	setProgress(100);

	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief Calculate CRC over \e byteCount bytes, starting at address
 *	 \e startAddress.
 *
 * \param[in] ui32StartAddress
 *	 Start address in device.
 * \param[in] ui32ByteCount
 *	 Number of bytes to calculate CRC32 over.
 * \param[out] pui32Crc
 *	 Pointer to where checksum from device is stored.
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
static uint32_t calculateCrc32(uint32_t ui32StartAddress, uint32_t ui32ByteCount, uint32_t *pui32Crc) {
	uint32_t retCode = SBL_SUCCESS;
	bool bSuccess = false;
	unsigned char pcPayload[12];
	uint32_t ui32RecvCount = 0;

	//
	// Check input arguments
	//
	if(!addressInFlash(ui32StartAddress, ui32ByteCount) &&
	 !addressInRam(ui32StartAddress, ui32ByteCount)) {
		PRINTF("ARGUMENT_ERROR: Specified address range (0x%08X + %d bytes) is not in device FLASH nor RAM.\n", ui32StartAddress, ui32ByteCount);
		return SBL_ARGUMENT_ERROR;
	}

	//
	// Set progress
	//
	setProgress(0);

	//
	// Build payload
	// - 4B address (MSB first)
	// - 4B byte count(MSB first)
	//
	ulToCharArray(ui32StartAddress, &pcPayload[0]);
	ulToCharArray(ui32ByteCount, &pcPayload[4]);
	pcPayload[8] = 0x00;
	pcPayload[9] = 0x00;
	pcPayload[10] = 0x00;
	pcPayload[11] = 0x00;
	//
	// Send command
	//
	if((retCode = sendcmd(CMD_CRC32, pcPayload, 12)) != SBL_SUCCESS) {
		return retCode;
	}

#ifdef DeviceFamily_CC13X2
    Task_sleep(2 * _100MS);
#endif
	//
	// Receive command response (ACK/NAK)
	//
	if((retCode = getCmdResponse(&bSuccess, 5, false)) != SBL_SUCCESS) {
		return retCode;
	}
	if(!bSuccess) {
		PRINTF("ERROR: Device NAKed CRC32 command.\n");
		return SBL_ERROR;
	}

	//
	// Get data response
	//
	ui32RecvCount = 4;
	if((retCode = getResponseData(pcPayload, ui32RecvCount, SBL_DEFAULT_RETRY_COUNT)) != SBL_SUCCESS) {
		sendCmdResponse(false);
		return retCode;
	}
	*pui32Crc = charArrayToUL(pcPayload);

	//
	// Send ACK/NAK to command
	//
	bool bAck = (ui32RecvCount == 4) ? true : false;
	sendCmdResponse(bAck);

	//
	// Set progress
	//
	setProgress(100);

	return SBL_SUCCESS;
}



//-----------------------------------------------------------------------------
/** \brief Write \e unitCount words of data to device FLASH. Source array is
 *	 8 bit wide. Parameters \e startAddress and \e unitCount must be a
 *	 a multiple of 4. This function does not erase the flash before writing 
 *	 data, this must be done using e.g. eraseFlashRange().
 *
 * \param[in] ui32StartAddress
 *	 Start address in device. Must be a multiple of 4.
 * \param[in] ui32ByteCount
 *	 Number of bytes to program. Must be a multiple of 4.
 * \param[in] pcData
 *	 Pointer to source data.
 * \return
 *	 Returns SBL_SUCCESS, ...
 */
//-----------------------------------------------------------------------------
uint32_t writeFlashRange(uint32_t ui32StartAddress, uint32_t ui32ByteCount, unsigned char *pcData, void (*progresscallback)(int percent)) {
	uint32_t devStatus = CMD_RET_UNKNOWN_CMD;
	uint32_t retCode = SBL_SUCCESS;
	uint32_t bytesLeft, dataIdx, bytesInTransfer;
	uint32_t transferNumber = 1;
	bool bIsRetry = false;
	bool bBlToBeDisabled = false;
	tTransfer pvTransfer[2];
	uint32_t ui32TotChunks = (ui32ByteCount / SBL_CC2650_MAX_BYTES_PER_TRANSFER);
	if(ui32ByteCount % SBL_CC2650_MAX_BYTES_PER_TRANSFER) ui32TotChunks++;
	uint32_t ui32CurrChunk = 0;

	//
	// Calculate BL configuration address (depends on flash size)
	//
	uint32_t ui32BlCfgAddr = SBL_CC2650_FLASH_START_ADDRESS + getFlashSize() - SBL_CC2650_PAGE_ERASE_SIZE + SBL_CC2650_BL_CONFIG_PAGE_OFFSET;

	//
	// Calculate BL configuration buffer index
	//
	uint32_t ui32BlCfgDataIdx = ui32BlCfgAddr - ui32StartAddress;

	//
	// Is BL configuration part of buffer?
	//
	if(ui32BlCfgDataIdx <= ui32ByteCount) {
		if(((pcData[ui32BlCfgDataIdx]) & 0xFF) != SBL_CC2650_BL_CONFIG_ENABLED_BM) {
			bBlToBeDisabled = false;
			PRINTF("ERROR: Warning: CC2650 bootloader will be disabled.\n");
			return SBL_ERROR;
		}
	}

	if(bBlToBeDisabled) {
		//
		// Main transfer (before lock bit)
		//
		pvTransfer[0].bExpectAck = true;
		pvTransfer[0].startAddr = ui32StartAddress;
		pvTransfer[0].byteCount = (ui32BlCfgAddr - ui32StartAddress) & (~0x03);
		pvTransfer[0].startOffset = 0;

		//
		// The transfer locking the backdoor
		//
		pvTransfer[1].bExpectAck = false;
		pvTransfer[1].startAddr = ui32BlCfgAddr - (ui32BlCfgAddr % 4);
		pvTransfer[1].byteCount = ui32ByteCount - pvTransfer[0].byteCount;
		pvTransfer[1].startOffset = ui32BlCfgDataIdx - (ui32BlCfgDataIdx % 4);

	}
	else {
		pvTransfer[0].bExpectAck = true;
		pvTransfer[0].byteCount = ui32ByteCount;
		pvTransfer[0].startAddr = ui32StartAddress;
		pvTransfer[0].startOffset = 0;
	}

	//
	// For each transfer
	//
	uint32_t i;
	for(i = 0; i < (bBlToBeDisabled ? 2 : 1); i++) {
		//
		// Sanity check
		//
		if(pvTransfer[i].byteCount == 0) {
			continue;
		}

		//
		// Set progress
		//
		setProgress(addressToPage(pvTransfer[i].startAddr));

		//
		// Send download command
		//
		if((retCode = cmdDownload(pvTransfer[i].startAddr, 
								 pvTransfer[i].byteCount)) != SBL_SUCCESS) {
			return retCode;
		}

		//
		// Check status after download command
		//
		retCode = readStatus(&devStatus);
		if(retCode != SBL_SUCCESS) {
			PRINTF("ERROR(retCode): Error during download initialization. Failed to read device status after sending download command.\n");
			return retCode;
		}
		if(devStatus != CMD_RET_SUCCESS) {
			PRINTF("ERROR: Error during download initialization. Device returned status %d (%s).\n", devStatus, getCmdStatusString(devStatus));
			return SBL_ERROR;
		}

		//
		// Send data in chunks
		//
		bytesLeft = pvTransfer[i].byteCount;
		dataIdx = pvTransfer[i].startOffset;
		while(bytesLeft) {
			//
			// Set progress
			//
			//setProgress(addressToPage(ui32StartAddress + dataIdx));
			if (progresscallback)
				progresscallback( ((100*(++ui32CurrChunk))/ui32TotChunks) );
			else
				setProgress( ((100*(++ui32CurrChunk))/ui32TotChunks) );

			//
			// Limit transfer count
			//
			bytesInTransfer = MIN2(SBL_CC2650_MAX_BYTES_PER_TRANSFER, bytesLeft);

			//
			// Send Data command
			//
			if(retCode = cmdSendData(&pcData[dataIdx], bytesInTransfer) != SBL_SUCCESS) {
				PRINTF("ERROR(retCode): Error during flash download. \n- Start address 0x%08X (page %d). \n- Tried to transfer %d bytes. \n- This was transfer %d.\n", (ui32StartAddress+dataIdx), addressToPage(ui32StartAddress+dataIdx), bytesInTransfer, (transferNumber));
				return retCode;
			}

			if(pvTransfer[i].bExpectAck) {
				//
				// Check status after send data command
				//
				devStatus = 0;
				retCode = readStatus(&devStatus);
				if(retCode != SBL_SUCCESS) {
					PRINTF("ERROR(retCode): Error during flash download. Failed to read device status.\n- Start address 0x%08X (page %d). \n- Tried to transfer %d bytes. \n- This was transfer %d in chunk %d.\n", (ui32StartAddress+dataIdx), addressToPage(ui32StartAddress + dataIdx), (bytesInTransfer), (transferNumber), (i));
					return retCode;
				}
				if(devStatus != CMD_RET_SUCCESS) {
					PRINTF("SUCCESS: Device returned status %s\n", getCmdStatusString(devStatus));
					if(bIsRetry) {
						//
						// We have failed a second time. Aborting.
						PRINTF("ERROR: Error retrying flash download.\n- Start address 0x%08X (page %d). \n- Tried to transfer %d bytes. \n- This was transfer %d in chunk %d.\n", (ui32StartAddress+dataIdx), addressToPage(ui32StartAddress + dataIdx), (bytesInTransfer), (transferNumber), (i));
						return SBL_ERROR;
					}

					//
					// Retry to send data one more time.
					//
					bIsRetry = true;
					continue;
				}
			}
			else {
				//
				// We're locking device and will lose access
				//
				PRINTF("PROBLEM PROBLEM\n");
			}

			//
			// Update index and bytesLeft
			//
			bytesLeft -= bytesInTransfer;
			dataIdx += bytesInTransfer;
			transferNumber++;
			bIsRetry = false;
		}
	}

	return SBL_SUCCESS;
}


//-----------------------------------------------------------------------------
/** \brief This function returns the page within which address \e ui32Address
 *	 is located.
 *
 * \param[in] ui32Address
 *	 The address.
 *
 * \return
 *	 Returns the flash page within which an address is located.
 */
//-----------------------------------------------------------------------------
static uint32_t addressToPage(uint32_t ui32Address) {
	return ((ui32Address - SBL_CC2650_FLASH_START_ADDRESS) / SBL_CC2650_PAGE_ERASE_SIZE);
}


//-----------------------------------------------------------------------------
/** \brief This function checks if the specified \e ui32StartAddress (and range)
 *	 is located within the device RAM area.
 *
 * \param[in] ui32StartAddress
 *	 The start address of the range
 * \param[in] pui32Bytecount
 *	 (Optional) The number of bytes in the range.
 * \return
 *	 Returns true if the address/range is within the device RAM.
 */
//-----------------------------------------------------------------------------
static bool addressInRam(uint32_t ui32StartAddress, uint32_t ui32ByteCount/* = 1*/) {
	uint32_t ui32EndAddr = ui32StartAddress + ui32ByteCount;

	if(ui32StartAddress < SBL_CC2650_RAM_START_ADDRESS) {
		return false;
	}
	if(ui32EndAddr > (SBL_CC2650_RAM_START_ADDRESS + getRamSize())) {
		return false;
	}
	return true;
}


//-----------------------------------------------------------------------------
/** \brief This function checks if the specified \e ui32StartAddress (and range)
 *	 is located within the device FLASH area.
 *
 * \param[in] ui32StartAddress
 *	 The start address of the range
 * \param[in] pui32Bytecount
 *	 (Optional) The number of bytes in the range.
 *
 * \return
 *	 Returns true if the address/range is within the device RAM.
 */
//-----------------------------------------------------------------------------
static bool addressInFlash(uint32_t ui32StartAddress, uint32_t ui32ByteCount/* = 1*/) {
	uint32_t ui32EndAddr = ui32StartAddress + ui32ByteCount;

	if(ui32EndAddr > (SBL_CC2650_FLASH_START_ADDRESS + getFlashSize())) {
		return false;
	}

	return true;
}

static void CCautoSelect() {
	uint32_t a;
	readDeviceId(&a);
	isM4 = a == 0x30828000 || a == 0x30029000;//0x20018000 for cc1310, 0x30828000 for CC1312R, 0x00828000 for CC1312R7, 0x30029000 for CC2642R
	PRINTF("Autoselect isM4:%d\n", isM4);
}

int calcCrcLikeChip(const unsigned char *pData, unsigned long ulByteCount) {
    unsigned long d, ind;
    unsigned long acc = 0xFFFFFFFF;
    const unsigned long ulCrcRand32Lut[] =
    {
        0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 
        0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C, 
        0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C, 
        0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C
    };

    while ( ulByteCount-- )
    {
        d = *pData++;
        ind = (acc & 0x0F) ^ (d & 0x0F);
        acc = (acc >> 4) ^ ulCrcRand32Lut[ind];
        ind = (acc & 0x0F) ^ (d >> 4);
        acc = (acc >> 4) ^ ulCrcRand32Lut[ind];
    }

    return (acc ^ 0xFFFFFFFF);
}

void CCdumpMetadata() {
	if (isM4 == -1)
		CCautoSelect();
	uint32_t a;
	readDeviceId(&a);
	PRINTF("ID: 0x%08x\n", a);
	readFlashSize(&a);
	PRINTF("Flash: %dkB\n", a / 1024);
	readRamSize(&a);
	PRINTF("Ram: %dkB\n", a / 1024);
	readStatus(&a);
	PRINTF("Status: 0x%x\n", a);
}

uint32_t CCeraseFlashAll() {
	if (isM4 == -1)
		CCautoSelect();
	PRINTF("Erasing size %ukiB\n", m_flashSize / 1024);
	if (m_flashSize != 0)
		return CCeraseFlashRange(0, m_flashSize);
    uint32_t a;
	if (readFlashSize(&a) == SBL_SUCCESS)
		return CCeraseFlashRange(0, m_flashSize);
	return CCeraseFlashRange(0, isM4 ? 1024 * 352 : 1024 * 128);
}

#ifdef DeviceFamily_CC13X2
#include <ti/drivers/NVS.h>
extern NVS_Handle nvsHandle;
unsigned char ss[8192];
#endif
uint32_t CCwriteFirmware(char *path, char *path3, int notAll, void (*progresscallback)(int percent)) {
#ifdef DeviceFamily_CC13X2
    if (isM4 == -1)
        CCautoSelect();

    uint32_t a;
    if (readDeviceId(&a) != SBL_SUCCESS) return 1;
    if (readFlashSize(&a) != SBL_SUCCESS) return 1;
    if (readRamSize(&a) != SBL_SUCCESS) return 1;
    if (CCeraseFlashAll() != SBL_SUCCESS) return 1;
    PRINTF("Erase done\n");
    int size = isM4 ? 1024 * 352 : 1024 * 128;
    NVS_read(nvsHandle, 0, ss, 512);

    int todo = size / 252 - 1;
    todo *= 252;
    if (writeFlashRange(todo, size - todo, ss + 4, NULL) != SBL_SUCCESS) return 1;
    todo = ss[3] + (ss[2] << 8) + (ss[1] << 16) + (ss[0] << 24);

    int done = 0;
    int nvspage = 1;
    while (done < todo) {
        NVS_read(nvsHandle, nvspage * 8192, ss, 8192);
        nvspage++;
        int tt = MIN2(8192, todo - done);
        if (writeFlashRange(done, tt, ss, NULL) != SBL_SUCCESS) return 1;
        done += tt;
        progresscallback(done * 100 / todo);
    }
    PRINTF("Write done\n");

    calculateCrc32(0, size, &a);
    PRINTF("CRC %x==%x\n", a, CRCLINE);
    return a != CRCLINE;
#else
	if (isM4 == -1)
		CCautoSelect();
	PRINTF("Writing firmware %s\n", isM4 ? path3 : path);
	if (strlen(path3) != 0 && notAll && isM4 == 0)
		notAll = 0;
	FILE *fp = fopen(isM4 ? path3 : path, "rb");
	if (fp) {
		int size = isM4 ? 1024 * 352 : 1024 * 128;
		int todo = isM4 ? 1024 * 352 : 1024 * 128;
		int end = isM4 ? 0x4A000 : 0x1FF00;
		unsigned char *pucData = malloc(size);
		fread(pucData, size, 1, fp);
		fclose(fp);
		if (notAll) {
			unsigned char *FFs = malloc(252);
			memset(FFs, 0xFF, 252);
			int i;
			for (i = end / 252 - 1; i >= 0; i--)
				if (memcmp(pucData + i * 252, FFs, 252) != 0) {
					todo = (i + 1) * 252;
					PRINTF("Flashing only %d%% (len:%d)\n", todo * 100 / size, todo);
					break;
				}
			if (i == -1) {
				todo = 0;
				PRINTF("Flashing nothing\n");
			}
		}
		uint32_t a;
		if (!noRxCCext) {
			if (readDeviceId(&a) != SBL_SUCCESS) return 1;
			if (readFlashSize(&a) != SBL_SUCCESS) return 1;
			if (readRamSize(&a) != SBL_SUCCESS) return 1;
		}
		if (CCeraseFlashAll() != SBL_SUCCESS) return 1;
		PRINTF("Erase done\n");

		struct timespec tsa;
		clock_gettime(CLOCK_REALTIME, &tsa);
		if (writeFlashRange(0, todo, pucData, progresscallback) != SBL_SUCCESS) return 1;
		struct timespec tsb;
		clock_gettime(CLOCK_REALTIME, &tsb);
		PRINTF("Main write done (%ds)\n", (int)(tsb.tv_sec - tsa.tv_sec));
		if (notAll) {
			PRINTF("Flashing header\n");
			todo = end / 252 - 1;
			todo *= 252;
			if (writeFlashRange(todo + m_flashSize - size, size - todo, pucData + todo, NULL) != SBL_SUCCESS) return 1;
		}
		if (!noRxCCext) {
			calculateCrc32(0, m_flashSize, &a);
			PRINTF("CRC %x==%x\n", a, calcCrcLikeChip(pucData, size));
			if (a != calcCrcLikeChip(pucData, size) && m_flashSize == size) return 1;
		}
		free(pucData);
	}
	return 0;
#endif
}

char *st[] = { "EXT_LF_CLK", "MODE_CONF_1", "SIZE_AND_DIS_FLAGS", "MODE_CONF", "VOLT_LOAD_0", "VOLT_LOAD_1", "RTC_OFFSET", "FREQ_OFFSET", "IEEE_MAC_0", "IEEE_MAC_1", "IEEE_BLE_0", "IEEE_BLE_1", "BL_CONFIG", "ERASE_CONF", "CCFG_TI_OPTIONS", "CCFG_TAP_DAP_0", "CCFG_TAP_DAP_1", "IMAGE_VALID_CONF", "CCFG_PROT_31_0", "CCFG_PROT_63_32", "CCFG_PROT_95_64", "CCFG_PROT_127_96", ""};
void CCreadMemory() {
	if (isM4 == -1)
		CCautoSelect();
	uint32_t a;
	readDeviceId(&a);
	readFlashSize(&a);
	readRamSize(&a);
	uint32_t *pui32Data = malloc(32 * 4);
	memset(pui32Data, 0, 32 * 4);
	readMemory32(0, 8, pui32Data);
	for (a = 0; a < 8; a++)
		PRINTF("0x%x: %08x\n", a, pui32Data[a]);

	memset(pui32Data, 0, 32 * 4);
	uint32_t s = 0xFA8;
	readMemory32((isM4 ? 0x57000 : 0x1F000) + s, 22, pui32Data);
	for (a = 0; a < 22; a++)
		PRINTF("0x%x (%s): %02x %02x %02x %02x\n", s + a * 4, st[a], (pui32Data[a] >> 24) & 0xFF, (pui32Data[a] >> 16) & 0xFF, (pui32Data[a] >> 8) & 0xFF, (pui32Data[a] >> 0) & 0xFF);
	free(pui32Data);
}

void CCreadMemoryFromFile(char *path) {
	if (isM4 == -1)
		CCautoSelect();
	FILE *fp = fopen(path, "rb");
	if (fp) {
		uint32_t *pui32Data = malloc(32 * 4);
		memset(pui32Data, 0, 32 * 4);
		uint32_t s = 0xFA8;
		fseek(fp, (isM4 ? 0x57000 : 0x1F000) + s, SEEK_SET);
		fread(pui32Data, 22 * 4, 1, fp);
		int a;
		for (a = 0; a < 22; a++)
			PRINTF("0x%x (%s): %02x %02x %02x %02x\n", s + a * 4, st[a], (pui32Data[a] >> 24) & 0xFF, (pui32Data[a] >> 16) & 0xFF, (pui32Data[a] >> 8) & 0xFF, (pui32Data[a] >> 0) & 0xFF);
		free(pui32Data);
	}
}
