#ifndef _MEMQ_H_
#define _MEMQ_H_
#include<Arduino.h>

#include "FlashMemory.h"
#include "RingEEPROM.h"

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
  MemQ(uint32_t startAddr);
  MemQ(uint32_t startMem, uint32_t endMem);
  void attachFlash(Flash *flashObj, page_t **pagePtr,uint8_t totalPage);
  void attachEEPRom(RingEEPROM  *ringEepRomPtr, uint8_t ringSz);

  void write(page_t *pageData);
   page_t *read(page_t *buf);
   
  void write(uint8_t *data);
  uint8_t *read(uint8_t *buf);

  void saveLoop();

  private:
  Flash *_flashPtr;
  page_t **_pagePtr;
  uint8_t _totalPage;
  RingEEPROM *_ringEepPtr;
  uint32_t _startAddr;
  uint32_t _endAddr;
};

#endif 
