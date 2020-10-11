#ifndef _MEMQ_H_
#define _MEMQ_H_
#include<Arduino.h>

#include "FlashMemory.h"
#include "RingEEPROM.h"

#define DEBUG_ON

typedef struct ringBuf_t
{
  uint32_t headPage;
  uint32_t tailPage;
};

typedef struct page_t 
{
  uint8_t data[256];
};

class MemQ
{
  public:
  MemQ(uint32_t startPage);
  MemQ(uint32_t startPage, uint32_t endPage);
  void attachFlash(Flash *flashObj, page_t **pagePtr,uint8_t totalPage);
  void attachEEPRom(RingEEPROM  *ringEepRomPtr, uint8_t ringSz);

  void writePage(page_t *page);
  page_t *readPage(page_t *page);
   
  void write(uint8_t *data);
  uint8_t *read(uint8_t *data);
  
  void savePageLoop();
  void saveBufferLoop();

  void reset();
  private:
  Flash *_flashObj;
  page_t **_pagePtr;
  uint8_t _totalPage;
  uint32_t _startPage;
  uint32_t _endPage;
  
  RingEEPROM *_ringEepObj;
  ringBuf_t ringBuffer;  
};

#endif 
