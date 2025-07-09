#ifndef SBL_H
#define SBL_H

#define bool char
#define true 1
#define false 0

//Global variable
extern int noRxCCext;
extern int CCflashInProgress;

//Global functions
bool CCinitCommunication(char *szUart, int resetViaGPIO);
void CCuninitCommunication();
bool CCFlash(char *szUart, char *szFirmware, char *szFirmware3, int resetViaGPIO);
uint32_t getCmdResponse(bool *bAck, uint32_t ui32MaxRetries, bool bQuiet);
uint32_t sendCmdResponse(bool bAck);
uint32_t getResponseData(unsigned char *pcData, uint32_t ui32MaxLen, uint32_t ui32MaxRetries);
uint32_t sendcmd(uint32_t ui32Cmd, unsigned char *pcSendData, uint32_t ui32SendLen);

#endif
