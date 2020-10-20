#ifndef _MEMQ_H_
#define _MEMQ_H_
#include<Arduino.h>
#include "FlashMemory.h"
#include "RingEEPROM.h"

#define DEBUG_ON

typedef struct ringBuf_t
{
  uint32_t headAddr;
  uint32_t tailAddr;
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

  uint8_t _memChangeCounter = 0;
  uint8_t _maxMemchangeCounter = 0;
  
  RingEEPROM *_ringEepObj;
  ringBuf_t ringBuffer;
  
};

#endif 