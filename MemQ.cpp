#include "MemQ.h"
//#include "FlashMemory.h"
//#include "RingEEPROM.h"

MemQ::MemQ(uint32_t startPage)
{
  _startPage = startPage;
}

MemQ::MemQ(uint32_t startPage, uint32_t endPage)
{
  _startPage = startPage;
  _endPage = endPage;
}

void MemQ::attachFlash(Flash *flashObj, page_t **pagePtr, uint8_t totalPage)
{
  _flashObj = flashObj;
  _pagePtr = pagePtr;
  _totalPage = totalPage;
  //begin flash
  _flashObj -> begin();

}
void MemQ::attachEEPRom(RingEEPROM  *ringEepRomPtr, uint8_t ringSz)
{
  _ringEepObj = ringEepRomPtr;
  //begin eeprom.
  _ringEepObj -> begin(ringSz, sizeof(ringBuf_t));
}
void MemQ::savePageLoop()
{
  if (*_pagePtr != NULL)
  {
    page_t *page = *_pagePtr;
    Serial.println(F("Ptr is not null"));
    for (byte i = 0; i < _totalPage; i++)
    {
      Serial.print(F("<-------------Writing Page : "));
      Serial.print(ringBuffer.headPage);
      Serial.println(F("------------>"));
#if defined(DEBUG_ON)
      //print Buffers before write)
      _flashObj -> printPageBytes((uint8_t*)page);
#endif
      //Erage Page before writing
      //Write Flash Pages
      _flashObj -> _writePage(ringBuffer.headPage,(uint8_t*)page);
#if defined(DEBUG_ON)
      //read and print page
      _flashObj -> printPage(ringBuffer.headPage);
#endif
    //increment page address to nex page
    ringBuffer.headPage++;
    //point page to next page data
    page++;
    }
    //update ring buffer
    _ringEepObj -> savePacket((byte*)&ringBuffer);
    //null pagePtr to avoid overwrite
    *_pagePtr = NULL;
  }
  else
  {
    Serial.println(F("Ptr is NULL"));

  }
}
