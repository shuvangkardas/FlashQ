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
  Serial.println(F("Setup done"));
}

void loop()
{
  //  pageData = NULL;
  Serial.println(F("test 1"));
  pageData = &page1;
  memQ.savePageLoop();
  delay(1000);
  Serial.println(F("test 2"));
  pageData = NULL;
  memQ.savePageLoop();
  delay(1000);
  Serial.println(F("test 3"));
  pageData = &page1;
  memQ.savePageLoop();
  delay(1000);

}
