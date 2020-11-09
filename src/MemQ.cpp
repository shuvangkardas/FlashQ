#include "MemQ.h"

#if defined(DEBUG_ON)  //print page for debug
uint8_t pageBuf[256];
#endif

MemQ::MemQ(uint32_t startAddr, uint32_t endAddr)
{
  _startAddr = startAddr;
  _endAddr = endAddr;
}

void MemQ::attachFlash(Flash *flashObj, void **dataPtr, uint16_t dataSize, uint16_t totalBuf)
{
  _flashObj = flashObj;
  _dataPtr = (uint8_t**)dataPtr;

  _dataSize = dataSize;
  _totalBuf = totalBuf;
  //begin flash
  _flashObj -> begin();
  _maxMemchangeCounter =  _totalBuf*2; //Eeprom will save after this number of flash activity

}
void MemQ::attachEEPRom(RingEEPROM  *ringEepRomPtr, uint8_t ringSz)
{
  _ringEepObj = ringEepRomPtr;
  _ringEepObj -> begin(ringSz, sizeof(ringBuf_t)); //begin eeprom.
  _ringEepObj -> readPacket((byte*)&ringBuffer);   // point last saved address.
}

uint16_t MemQ::getPayloadSz()
{
  return _dataSize;
}

void MemQ::attachSafetyFuncs(func_t enableBus,func_t disableBus)
{
	_enableBus = enableBus;
	_disableBus = disableBus;
}

void *MemQ::read(void *buf, uint8_t n)
{
  if (ringBuffer.tailAddr < ringBuffer.headAddr)
  {
    Serial.print(F("<--------Reading Payload addr : "));
    Serial.print(ringBuffer.tailAddr); Serial.println(F("-------->"));

    uint16_t totalbyte = _dataSize * n;

    if(_disableBus)
    {
    	Serial.println(F("SPI Disabled others"));
    	_disableBus();
    }
    _flashObj -> read(ringBuffer.tailAddr, (uint8_t*)buf, totalbyte);
    if(_enableBus)
    {
    	_enableBus();
    	Serial.println(F("SPI Enabled others"));
    }

    ringBuffer.tailAddr += totalbyte;
    _memChangeCounter += n;
    return buf;
  }
  else
  {
    //    _ringEepObj -> savePacket((byte*)&ringBuffer);
    return NULL;
    Serial.println(F("No new data to read"));
  }
}

void MemQ::saveLoop()
{
  if (*_dataPtr != NULL)
  {
    Serial.print(F("<--------Writing Payload addr : "));
    Serial.print(ringBuffer.headAddr); Serial.println(F("-------->"));

    uint16_t totalbyte = _dataSize * _totalBuf;

    if(_disableBus){ _disableBus();}
    _flashObj -> write(ringBuffer.headAddr, *_dataPtr, totalbyte);
    if(_enableBus) {  _enableBus();}

#if defined(DEBUG_ON)  //print page for debug
    _flashObj -> dumpPage(ringBuffer.headAddr >> 8, pageBuf);
    // _flashObj -> dumpPage((ringBuffer.headAddr >> 8) + 1, pageBuf);
#endif
    ringBuffer.headAddr += totalbyte; //increment head pointer
    _memChangeCounter += _totalBuf;
    *_dataPtr = NULL; //null pagePtr to avoid overwrite
  }
  else
  {
    //    Serial.println(F("Ptr is null"));
  }

  //EEEPROM store Data after these activity
  if (_memChangeCounter >= _maxMemchangeCounter)
  {
    Serial.println(F("<===Updating EEPROM===>"));
    _ringEepObj -> savePacket((byte*)&ringBuffer);
    _memChangeCounter = 0;
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
