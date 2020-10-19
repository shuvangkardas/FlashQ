#ifndef _MEMQ_H_
#define _MEMQ_H_
#include<Arduino.h>
#include "FlashMemory.h"
#include "RingEEPROM.h"

#define PAGE_BYTE 256
#define SECTOR_BYTE 4096
#define SECTOR_PAGE (SECTOR_BYTE/PAGE_BYTE)
 

#define DEBUG_ON

typedef struct ringBuf_t
{
  uint32_t headAddr;
  uint32_t tailAddr;
};

typedef struct page_t 
{
  uint8_t data[256];
};

class MemQ
{
  public:
  MemQ(uint32_t startAddr, uint32_t endAddr);
  void attachFlash(Flash *flashObj, void **dataPtr, uint16_t dataSize, uint16_t totalBuf);
  void attachEEPRom(RingEEPROM  *ringEepRomPtr, uint8_t ringSz);
  
  void saveLoop();
  void *read(void *buf, uint8_t n = 1);

  void reset();
  private:
  Flash *_flashObj;
  uint8_t **_dataPtr;
  uint16_t _dataSize;
  uint16_t _totalBuf;
  uint32_t _startAddr;
  uint32_t _endAddr;
  
  RingEEPROM *_ringEepObj;
  ringBuf_t ringBuffer;
  
};

#endif 
