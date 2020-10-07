#include "MemQ.h"

Flash flash(5);       //CS PIN = 5
RingEEPROM myeepRom;
page_t *pageData;
MemQ memQ(5,100);
void setup()
{
  Serial.begin(9600);
  memQ.attachFlash(&flash,pageData); 
}

void loop() 
{
  
}
