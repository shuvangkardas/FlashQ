#include "MemQ.h"
//#include "FlashMemory.h"
//#include "RingEEPROM.h"

uint8_t pageBuf[256];

MemQ::MemQ(uint32_t startAddr, uint32_t endAddr)
{
  _startAddr = startAddr;
  _endAddr = endAddr;
}

void MemQ::attachFlash(Flash *flashObj, void **dataPtr, uint16_t dataSize, uint16_t totalBuf)
{
  _flashObj = flashObj;
  _dataPtr = dataPtr;

  _dataSize = dataSize;
  _totalBuf = totalBuf;
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

void *MemQ::read(void *buf, uint8_t n)
{
  if (ringBuffer.tailAddr < ringBuffer.headAddr)
  {
    Serial.print(F("<--------Reading Payload addr : "));
    Serial.print(ringBuffer.tailAddr);Serial.println(F("-------->"));
    
    uint16_t totalbyte = _dataSize*n;
    _flashObj -> read(ringBuffer.tailAddr, buf, totalbyte);
    ringBuffer.tailAddr += totalbyte;
    //Save eeprom buffer

    
    return buf;
  }
  else
  {
    return NULL;
    Serial.println(F("No new data to read"));
  }
}

void MemQ::saveLoop()
{
  if (*_dataPtr != NULL)
  {
     Serial.print(F("<--------Writing Payload addr : "));
     Serial.print(ringBuffer.headAddr);Serial.println(F("-------->"));
    //Save data
    uint16_t totalbyte = _dataSize * _totalBuf;
    _flashObj -> write(ringBuffer.headAddr, *_dataPtr, totalbyte);
#if defined(DEBUG_ON)
    //print page for debug
    _flashObj -> dumpPage(ringBuffer.headAddr >> 8, pageBuf);
    _flashObj -> dumpPage((ringBuffer.headAddr >> 8) + 1, pageBuf);
#endif
    //increment head pointer
    ringBuffer.headAddr += totalbyte;
    //update ring buffer
    _ringEepObj -> savePacket((byte*)&ringBuffer);
    //null pagePtr to avoid overwrite
    *_dataPtr = NULL;
  }
  else
  {
    //    Serial.println(F("Ptr is null"));
  }
}
void MemQ::reset()
{
  ringBuffer.headAddr = _startAddr;
  ringBuffer.tailAddr = _startAddr;
  _ringEepObj -> _clrStatusBuf();
  _ringEepObj -> savePacket((byte*)&ringBuffer);
  _flashObj -> eraseChip();
}


//page_t *MemQ::readPage(page_t *page)
//{
//  if (ringBuffer.tailPage < ringBuffer.headPage)
//  {
//    if (ringBuffer.tailPage > _endPage)
//    {
//      Serial.println(F("Head Buffer Reset"));
//      ringBuffer.tailPage = _startPage;
//    }
//    Serial.print(F("<----------Reading Page : "));
//    Serial.print(ringBuffer.tailPage); Serial.println(F("-------------->"));
//    page_t *p = (page_t*)_flashObj -> _readPage(ringBuffer.tailPage,(byte*) page);
//    //print read page
//    _flashObj -> printPageBytes((byte*)p);
//    ringBuffer.tailPage++;
//    return p;
//  }
//  else
//  {
//    Serial.println(F("No New Data to Read"));
//    return NULL;
//  }
//
//}
//
//void MemQ::savePageLoop()
//{
//  if (*_pagePtr != NULL)
//  {
//    page_t *page = *_pagePtr;
//    Serial.println(F("Ptr is not null"));
//    for (byte i = 0; i < _totalPage; i++)
//    {
//      if (ringBuffer.headPage > _endPage)
//      {
//        Serial.println(F("Head Buffer Reset"));
//        ringBuffer.headPage = _startPage;
//      }
//      Serial.print(F("<-------------Writing Page : "));
//      Serial.print(ringBuffer.headPage);
//      Serial.println(F("------------>"));
//#if defined(DEBUG_ON)
//      //print Buffers before write)
//      _flashObj -> printPageBytes((uint8_t*)page);
//#endif
//      //Erage Page before writing
//      //Write Flash Pages
//      _flashObj -> _writePage(ringBuffer.headPage, (uint8_t*)page);
//#if defined(DEBUG_ON)
//      //read and print page
//      _flashObj -> printPage(ringBuffer.headPage);
//#endif
//      //increment page address to nex page
//      ringBuffer.headPage++;
//      //point page to next page data
//      page++;
//    }
//    //update ring buffer
//    _ringEepObj -> savePacket((byte*)&ringBuffer);
//    //null pagePtr to avoid overwrite
//    *_pagePtr = NULL;
//  }
//  else
//  {
//    Serial.println(F("Ptr is NULL"));
//
//  }
//}
