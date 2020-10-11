#include "MemQ.h"

Flash flash(5);       //CS PIN = 5
RingEEPROM myeepRom(0x10);
page_t *pageData = NULL;
uint8_t totalPageBuf = 1;
page_t page1;
MemQ memQ(5, 100);
void setup()
{
  Serial.begin(9600);
  memQ.attachFlash(&flash, &pageData, totalPageBuf);
  memQ.attachEEPRom(&myeepRom, 4);
//  memQ.reset();
  Serial.println(F("Setup done"));
}

void loop()
{
  //  pageData = NULL;
  Serial.println(F("test 1"));
  pageData = generateData((uint8_t*)&page1);
  Serial.println(F("Data Gen done"));
  memQ.savePageLoop();
  delay(1000);
  Serial.println(F("test 2"));
  pageData = NULL;
  memQ.savePageLoop();
  delay(1000);
  Serial.println(F("test 3"));
  pageData = generateData((uint8_t*)&page1);
  memQ.savePageLoop();
  delay(1000);
}

page_t *generateData(uint8_t *buffer)
{
  uint8_t *ptr = buffer;
  uint8_t value = 0;
  for(int i = 0 ; i<256; i++)
  {
    ptr[i] = value; 
    value++;
  }
  return (page_t*)buffer;
  
}

