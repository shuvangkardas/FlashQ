#ifndef _MEMQ_H_
#define _MEMQ_H_
#include <Arduino.h>

#if defined(PROD_BUILD)
  #include "../../FlashMemory/src/FlashMemory.h"
  #include "../../RingEEPROM/src/RingEEPROM.h"
#else
  #include "FlashMemory.h"
  #include "RingEEPROM.h"
#endif
#define DEBUG_ON

typedef void (*func_t)(void);

typedef struct ringBuf_t
{
  uint32_t headAddr;
  uint32_t tailAddr;
  // uint32_t erasedSecAddr;
  uint16_t erasedSector;

};

class MemQ
{
public:
  MemQ(uint16_t startSector, uint16_t endSector);
  void attachFlash(Flash *flashObj, uint8_t **dataPtr, uint8_t packetSz, uint8_t totalPacket);
  void attachEEPRom(RingEEPROM *ringEepRomPtr, uint8_t ringSz);
  void attachSafetyFuncs(func_t enableBus, func_t disableBus);
  void saveLoop();
  uint8_t *read(uint8_t *buf, uint8_t n = 1);
  uint16_t getPayloadSz();
  void reset();
  void erase();
  void debug(bool onOff);

  /**************New API*******************/

  uint32_t available();
  void saveFast();
  void manageMemory();

private:
  void _eraseNextSector();

  Flash *_flashObj;
  uint8_t **_dataPtr;
  uint8_t _dataSize;
  uint8_t _totalBuf;
  uint16_t _totalByteToSave;
  uint32_t _startAddr;
  uint32_t _endAddr;

  RingEEPROM *_ringEepObj;
  ringBuf_t ringBuffer;

  func_t _enableBus = NULL;
  func_t _disableBus = NULL;

  uint8_t _memChangeCounter = 0;
  uint8_t _maxMemchangeCounter = 0;
  bool _debug = false;
};

#endif
