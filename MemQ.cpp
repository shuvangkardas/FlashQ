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
  // point last saved address.
  _ringEepObj -> readPacket((byte*)&ringBuffer);
}

void MemQ::reset()
{
  ringBuffer.headPage = _startPage;
  ringBuffer.tailPage = _startPage;
  _ringEepObj -> _clrStatusBuf();
  _ringEepObj -> savePacket((byte*)&ringBuffer);
  _flashObj -> eraseChipData();

}
page_t *MemQ::readPage(page_t *page)
{
  if (ringBuffer.tailPage < ringBuffer.headPage)
  {
    if (ringBuffer.tailPage > _endPage)
    {
      Serial.println(F("Head Buffer Reset"));
      ringBuffer.tailPage = _startPage;
    }
    Serial.print(F("<----------Reading Page : "));
    Serial.print(ringBuffer.tailPage); Serial.println(F("-------------->"));
    page_t *p = (page_t*)_flashObj -> _readPage(ringBuffer.tailPage,(byte*) page);
    //print read page
    _flashObj -> printPageBytes((byte*)p);
    ringBuffer.tailPage++;
    return p;
  }
  else
  {
    Serial.println(F("No New Data to Read"));
    return NULL;
  }

}

void MemQ::savePageLoop()
{
  if (*_pagePtr != NULL)
  {
    page_t *page = *_pagePtr;
    Serial.println(F("Ptr is not null"));
    for (byte i = 0; i < _totalPage; i++)
    {
      if (ringBuffer.headPage > _endPage)
      {
        Serial.println(F("Head Buffer Reset"));
        ringBuffer.headPage = _startPage;
      }
      Serial.print(F("<-------------Writing Page : "));
      Serial.print(ringBuffer.headPage);
      Serial.println(F("------------>"));
#if defined(DEBUG_ON)
      //print Buffers before write)
      _flashObj -> printPageBytes((uint8_t*)page);
#endif
      //Erage Page before writing
      //Write Flash Pages
      _flashObj -> _writePage(ringBuffer.headPage, (uint8_t*)page);
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
