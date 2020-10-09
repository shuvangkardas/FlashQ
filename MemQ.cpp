#include "MemQ.h"
//#include "FlashMemory.h"
//#include "RingEEPROM.h"

MemQ::MemQ(uint32_t startAddr)
{
  _startAddr = startAddr;
}

MemQ::MemQ(uint32_t startMem, uint32_t endMem)
{
  _startAddr = startMem;
  _endAddr = endMem;
}

void MemQ::attachFlash(Flash *flashObj, page_t **pagePtr, uint8_t totalPage)
{
  _flashPtr = flashObj;
  _pagePtr = pagePtr;
  _totalPage = totalPage;
  //begin flash
  _flashPtr -> begin();
  
}
void MemQ::attachEEPRom(RingEEPROM  *ringEepRomPtr, uint8_t ringSz)
{
  _ringEepPtr = ringEepRomPtr;
  _ringEepPtr -> begin(ringSz,sizeof(ringBuf_t));
}
void MemQ::saveLoop()
{
  if (*_pagePtr == NULL)
  {
    Serial.println(F("Ptr is NULL"));
  }
  else
  {
    Serial.println(F("Ptr is not null"));
  }
}

