#include "MemQ.h"

#define SECTOR_SIZE 4096UL
#if defined(DEBUG_ON) //print page for debug
uint8_t pageBuf[256];
#endif

MemQ::MemQ(uint16_t startSector, uint16_t endSector)
{
  _startAddr = (uint32_t)(startSector << 12);
  _endAddr = (uint32_t)(endSector << 12) + SECTOR_SIZE-1;
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
      ringBuffer.erasedSecAddr = ringBuffer.headAddr; //byte addr to sector addr
      // reset(); //resets device as sector erase is not working
      Serial.print(F(">>>Erasing Sector : "));
      Serial.println(ringBuffer.erasedSecAddr >> 12);
     
      _flashObj -> eraseSector(ringBuffer.erasedSecAddr);

      uint16_t curPage = ringBuffer.erasedSecAddr >> 8;
      for (uint16_t i = curPage; i < curPage + 16; i++ )
      {
        _flashObj->dumpPage(i, pageBuf);
      }

      // clear first sector
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
    Serial.println(F("<==Updating EEPROM==>"));
    _ringEepObj->savePacket((byte *)&ringBuffer);
    _memChangeCounter = 0;
  }

  uint16_t currentSector = (uint16_t)(ringBuffer.headAddr >> 12);
  uint16_t erasedSector  = (uint16_t)(ringBuffer.erasedSecAddr >> 12);
  if (currentSector == erasedSector)
  {
    uint32_t tempSecAddr = ringBuffer.erasedSecAddr + SECTOR_SIZE;
    // uint16_t endSecotr = _endAddr << 12;
    // if()
    // ringBuffer.erasedSecAddr += SECTOR_SIZE;
    if(_endAddr < tempSecAddr)
    {
      Serial.print(F(">>>Adv. Erasing Sector : "));
      Serial.println(tempSecAddr >> 12);
      _flashObj->eraseSector(tempSecAddr); //Erase Next Sector
      ringBuffer.erasedSecAddr = tempSecAddr;
      _flashObj->dumpPage(ringBuffer.erasedSecAddr>>8, pageBuf);
    }
    
  }
}

void MemQ::reset()
{
  ringBuffer.headAddr = _startAddr;
  ringBuffer.tailAddr = _endAddr;
  _ringEepObj->_clrStatusBuf();
  _ringEepObj->savePacket((byte *)&ringBuffer);
  _flashObj->eraseChip();
}
