#include "MemQ.h"
//#include "FlashMemory.h"
//#include "RingEEPROM.h"

MemQ::MemQ(uint32_t startAddr)
{

}

MemQ::MemQ(uint32_t startMem, uint32_t endMem)
{

}

void MemQ::attachFlash(Flash *flashPtr, page_t **pagePtr)
{
_pagePtr = pagePtr;
}

void MemQ::attachEEPRom(RingEEPROM  *ringEepRomPtr, uint8_t ringSz)
{

}
void MemQ::saveLoop()
{
  if(*_pagePtr == NULL)
  {
    Serial.println(F("Ptr is NULL"));
  }
  else
  {
    Serial.println(F("Ptr is not null"));
  }
}

