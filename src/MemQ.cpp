#include "MemQ.h"

#define SECTOR_SIZE 4096UL
#if defined(DEBUG_ON) //print page for debug
uint8_t pageBuf[256];
#endif

MemQ::MemQ(uint16_t startSector, uint16_t endSector)
{
  _startAddr = startSector;
  _startAddr = (_startAddr<<12);
  // _startAddr = (uint32_t)(startSector << 12);

  // _endAddr = (uint32_t)(endSector << 12) + SECTOR_SIZE;
  // _endAddr = (uint32_t)(endSector << 12);
  _endAddr = endSector;
  _endAddr = (_endAddr<<12) + SECTOR_SIZE - 1;

  // _startSector = startSector;
  // _endSector = endSector;
}

void MemQ::attachFlash(Flash *flashObj, uint8_t **dataPtr, uint8_t packetSz, uint8_t totalPacket)
// void MemQ::attachFlash(Flash *flashObj, void **dataPtr, uint16_t dataSize, uint16_t totalBuf)
{
  _flashObj = flashObj;
  _dataPtr = dataPtr;

  _dataSize = packetSz;
  _totalBuf = totalPacket;
  _totalByteToSave = packetSz * totalPacket;

  Serial.print(F("end sec : "));Serial.println(_endAddr);
  uint16_t leftByte = (_endAddr - _startAddr)%_totalByteToSave;
  Serial.print(F("Left Byte : ")); Serial.println(leftByte);
  _endAddr = _endAddr - leftByte; //Adjusting last endAddr
  Serial.print(F("Last Addr: ")); Serial.println(_endAddr);
  //begin flash
  _flashObj->begin();
  _maxMemchangeCounter = _totalBuf * 2; //Eeprom will save after this number of flash activity

  Serial.print(F("Flash Start Addr : "));
  Serial.println(_startAddr);
  Serial.print(F("Flash End Addr : "));
  Serial.println(_endAddr);
}

void MemQ::attachEEPRom(RingEEPROM *ringEepRomPtr, uint8_t ringSz)
{
  _ringEepObj = ringEepRomPtr;
  _ringEepObj->begin(ringSz, sizeof(ringBuf_t)); //begin eeprom.
  _ringEepObj->readPacket((byte *)&ringBuffer);  // point last saved address.
}

uint16_t MemQ::getPayloadSz()
{
  return _dataSize;
}

void MemQ::debug(bool onOff)
{
  _debug = onOff;
}

void MemQ::attachSafetyFuncs(func_t enableBus, func_t disableBus)
{
  _enableBus = enableBus;
  _disableBus = disableBus;
}

void MemQ::saveLoop()
{
  //   if (*_dataPtr != NULL)
  //   {

  //     Serial.print(F("<--Flash Write:"));
  //     Serial.print(ringBuffer.headAddr);Serial.println(F("-->"));

  //     uint16_t totalbyte = _dataSize * _totalBuf;

  //     if(_disableBus){ _disableBus();}
  //     _flashObj -> write(ringBuffer.headAddr, *_dataPtr, totalbyte);
  //     if(_enableBus) {  _enableBus();}

  // #if defined(DEBUG_ON)  //print page for debug
  //     if(_debug)
  //     {
  //     	_flashObj -> dumpPage(ringBuffer.headAddr >> 8, pageBuf);
  //     }
  //     // _flashObj -> dumpPage((ringBuffer.headAddr >> 8) + 1, pageBuf);
  // #endif
  //     ringBuffer.headAddr += totalbyte; //increment head pointer
  //     _memChangeCounter += _totalBuf;
  //     *_dataPtr = NULL; //null pagePtr to avoid overwrite
  //   }
  //   else
  //   {
  //     //    Serial.println(F("Ptr is null"));
  //   }

  // //EEEPROM store Data after these activity
  // if (_memChangeCounter >= _maxMemchangeCounter)
  // {
  //   Serial.println(F("<==Updating EEPROM==>"));
  //   _ringEepObj -> savePacket((byte*)&ringBuffer);
  //   _memChangeCounter = 0;
  // }
  saveFast();
  manageMemory();
}

void MemQ::saveFast()
{
  if (*_dataPtr != NULL)
  {
    if (_disableBus)
    {
      _disableBus();
    }
    _flashObj->write(ringBuffer.headAddr, *_dataPtr, _totalByteToSave);
    if (_enableBus)
    {
      _enableBus();
    }

    if (_debug)
    {
      Serial.print(F("<--Flash Write:"));
      Serial.print(ringBuffer.headAddr);
      Serial.println(F("-->"));
      // _flashObj->dumpPage(ringBuffer.headAddr >> 8, pageBuf);
    }

    ringBuffer.headAddr += _totalByteToSave; //increment head pointer
    if (ringBuffer.headAddr >= _endAddr)
    {
      ringBuffer.headAddr = _startAddr;
      
      // reset(); //resets device as sector erase is not working
      Serial.print(F(">>>Erasing Sector : "));
      Serial.println(ringBuffer.headAddr>>12);
     
      _flashObj -> eraseSector(ringBuffer.headAddr);
      ringBuffer.erasedSector = ringBuffer.headAddr >> 12; //byte addr to sector addr

       _ringEepObj->savePacket((byte *)&ringBuffer);

      uint32_t curPage = ((uint32_t)ringBuffer.erasedSector)<<4;
      _flashObj->dumpPage(curPage, pageBuf);
      // for (uint32_t i = curPage; i < curPage + 16; i++ )
      // {
      //   _flashObj->dumpPage(i, pageBuf);
      // }
    }
    _memChangeCounter += _totalBuf;
    *_dataPtr = NULL; //null pagePtr to avoid overwrite
  }
}

uint32_t MemQ::available()
{
  return abs(ringBuffer.headAddr - ringBuffer.tailAddr);
}

uint8_t *MemQ::read(uint8_t *buf, uint8_t n)
{
  if (ringBuffer.tailAddr == ringBuffer.headAddr)
  {
    // if(_debug){Serial.println(F("No new data to read"));}
    return NULL;
  }
  else
  {
    if (_debug)
    {
      Serial.print(F("<--Flash Read:"));
      Serial.print(ringBuffer.tailAddr);
      Serial.println(F("-->"));
    }

    uint16_t totalbyte = _dataSize;
    if (n > 1)
    {
      totalbyte = _dataSize * n;
    }

    if (_disableBus)
    {
      _disableBus();
    }
    _flashObj->read(ringBuffer.tailAddr, (uint8_t *)buf, totalbyte);
    if (_enableBus)
    {
      _enableBus();
    }

    ringBuffer.tailAddr += totalbyte;
    if (ringBuffer.tailAddr >= _endAddr)
    {
      ringBuffer.tailAddr = _startAddr;
    }
    _memChangeCounter += n;
    return buf;
  }
}

// uint16_t _sectorErased = ringBuffer.headAddr >> 12;

void MemQ::manageMemory()
{
  //EEEPROM store Data after these activity
  if (_memChangeCounter >= _maxMemchangeCounter)
  {
    Serial.println(F("<==Updating FLash Pointer==>"));
    _eraseNextSector();
    _ringEepObj->savePacket((byte *)&ringBuffer);
    _memChangeCounter = 0;
  }

 
}

void MemQ::_eraseNextSector()
{
   uint16_t currentSector = (uint16_t)(ringBuffer.headAddr >> 12);
  // uint16_t erasedSector  = (uint16_t)(ringBuffer.erasedSecAddr >> 12);
  if (currentSector == ringBuffer.erasedSector)
  {
    uint16_t endSector = _endAddr >> 12;
    uint16_t tempSector = ringBuffer.erasedSector + 1;
    if(tempSector <= endSector)
    {
      Serial.print(F(">>>Adv. Erasing Sector : "));
      Serial.println(tempSector);
      uint32_t tempSecAddr = ((uint32_t)tempSector)<<12;
      _flashObj->eraseSector(tempSecAddr); //Erase Next Sector
      ringBuffer.erasedSector = tempSector;
       _ringEepObj->savePacket((byte *)&ringBuffer);
      _flashObj->dumpPage(ringBuffer.erasedSector<<4, pageBuf);
    }
    
  }
}

void MemQ::reset()
{
  ringBuffer.headAddr = _startAddr;
  ringBuffer.tailAddr = _startAddr;
  _ringEepObj->_clrStatusBuf();
  _ringEepObj->savePacket((byte *)&ringBuffer);
  _flashObj->eraseChip();
}

void MemQ::erase()
{
  Serial.println(F("Erasing Flash Buffer"));

  uint16_t sectorStart = _startAddr >> 12;
  uint16_t sectorEnd   = _endAddr >> 12;
  uint32_t sectorAddr;
  for(uint16_t i = sectorStart;i<=sectorEnd ; i++)
  {
    Serial.print(F("-->Erasing Sector : "));
    Serial.println(i);
    sectorAddr = i<<12;
    _flashObj->eraseSector(sectorAddr);
  }

  ringBuffer.headAddr = _startAddr;
  ringBuffer.tailAddr = _startAddr;
  ringBuffer.erasedSector = _endAddr >> 12; //This will prevent erasing secotr after calling erase functions.
  Serial.print(F("ringBuffer.erasedSector : "));Serial.println(ringBuffer.erasedSector);
  _ringEepObj->_clrStatusBuf();
  _ringEepObj->savePacket((byte *)&ringBuffer);
}
