#ifndef SBL2_H
#define SBL2_H

//Global functions
void selectM4(int i);
uint32_t CCreset();
int calcCrcLikeChip(const unsigned char *pData, unsigned long ulByteCount);
void CCdumpMetadata();
void CCreadMemory();
void CCreadMemoryFromFile(char *path);
uint32_t CCeraseFlashAll();
uint32_t CCwriteFirmware(char *path, char *path3, int notAll, void (*progresscallback)(int percent));

#endif
