#include "MemQ.h"

Flash flash(5);       //CS PIN = 5
RingEEPROM myeepRom;
page_t *pageData = NULL;
page_t page1;
MemQ memQ(5, 100);
void setup()
{
  Serial.begin(9600);
  memQ.attachFlash(&flash, &pageData);
  memQ.attachEEPRom(&myeepRom, 4);
  Serial.println(F("Setup done"));
}

void loop()
{
  pageData = NULL;
  memQ.saveLoop();
  delay(1000);
  pageData = &page1;
  memQ.saveLoop();
  delay(1000);
  
}
