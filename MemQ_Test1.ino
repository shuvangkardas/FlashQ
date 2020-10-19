#include "MemQ.h"

#define TOTAL_BUFFER 10
#define TOTAL_FLASH_BUF (TOTAL_BUFFER/2)

typedef struct payload_t
{
  byte var1;
  byte var2;
  byte var3;
  byte var4;
  byte var5;
};

Flash flash(8);       //CS PIN = 5
RingEEPROM myeepRom(0x10);

payload_t payload[TOTAL_BUFFER];
payload_t *flashPtr;
payload_t *sensorPtr;

payload_t readPayload;

MemQ memQ(100, 1000);
uint8_t index = 0;

void setup()
{
  Serial.begin(9600);
  memQ.attachFlash(&flash, &flashPtr, sizeof(payload_t),TOTAL_FLASH_BUF);
  memQ.attachEEPRom(&myeepRom, 4);
//  memQ.reset();

  sensorPtr = &payload[0];
  flashPtr = NULL;
  
  Serial.println(F("Setup done"));
}

void loop()
{
  if(Serial.available())
  {
    byte x = Serial.read();
    populatePayload(&sensorPtr[index]);
    index++;
    if(index == 5)
    {
        flashPtr = &sensorPtr[0];
    }
    else if( index == 10)
    {
       flashPtr = &sensorPtr[5];
       index = 0;
    }
  }
  memQ.saveLoop();
  payload_t *Ptr = memQ.read(&readPayload);
  if(Ptr)
  {
    printPayload(Ptr);
  }
}


void populatePayload(payload_t *p)
{
  Serial.print(F("Populating Data : "));Serial.println(index);
//  p -> var1 = 1;
//  p -> var2 = 2;
//  p -> var3 = 3;
//  p -> var4 = 4;
//  p -> var5 = 5;
  p -> var1 = random(1,100);
  p -> var2 = random(1,100);
  p -> var3 = random(1,100);
  p -> var4 = random(1,100);
  p -> var5 = random(1,100);
  
}

void printPayload(payload_t *p)
{
  Serial.print(F("Payload : "));
  Serial.print(p -> var1); Serial.print(" ");
  Serial.print(p -> var2); Serial.print(" ");
  Serial.print(p -> var3); Serial.print(" ");
  Serial.print(p -> var4); Serial.print(" ");
  Serial.print(p -> var5); Serial.println(" ");
}

uint8_t *generateData(uint8_t *buffer)
{ 
  uint8_t *ptr = buffer;
  uint8_t value = 0;
  for (int i = 0 ; i < 256; i++)
  {
    ptr[i] = value;
    //    ptr[i] = 0xFF;
    value++;
  }
  return buffer;
}
