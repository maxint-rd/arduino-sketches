/*
  TM1637 S6xG5 numbers example to test showing scrolling text on the Maxint TM1637 6x5 LED matrix display.

  This example is for testing the various functions of the LED matrix module.
  Note that while the TM1637 chip supports 8x6 LED segments and 8x2 buttons,
  the module has 6 segments x 5 rows and an interface for 3x1 pushbuttons.

  This example uses the TM1637 class of the TM16xx library. The optional LED matrix classes are
  not used by this example, to allow it to run on memory limited MCUs such as the ATtiny44 and 
  ATtiny85.
  
  It was tested on ATTiny44A (ATtinyCore/LTO enabled) 8MHz in Arduino IDE 1.8.2 with minimal functionality. (4000B flash, 119B RAM)
  It was tested on ATTiny85 (ATtinyCore/LTO enabled) 1 MHz in Arduino IDE 1.8.2 with more functionality. (minimal 4070B flash, 119B RAM)
  Board: Maxint TinyDev T85, programmer: USBasp (ATTinyCore) 
  
  Made by Maxint-RD MMOLE 2019. see GitHub.com/maxint-rd/TM16xx
*/

#include <TM1637.h>
#include <TM16xxButtons.h>
TM1637 module(3, 4);    //  ATtiny85: DIO=3, CLK=4
//TM1637 module(5, 4);  // DIO=5, CLK=4
//TM1637 module(10, 9);    // ATtiny44A: DIN=10, CLK=9 (next to VCC on the Maxint ATtiny44A Develelopment board)

#include "tinyRTC.h"      // ATtiny85: SCL=2, SDA=0. ATtiny44A: SCL=4, SDA=6

#define SMALLFONTS_OPT_UPPER 1     // ;-ucase-Z (set to 0 to save 160 bytes)
#define SMALLFONTS_OPT_LOWER 1     // [-lcase-|
#include "matrix5x6.h"


void Blink(uint8_t nDelay=10)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(nDelay);
  digitalWrite(LED_BUILTIN, LOW);
}

void Flash()
{
  setAllPixels(true);
  delay(100);  
  setAllPixels(false);
  delay(400);  
}

//char tape[] = "00:00 ";
//char tape[] = "TIME 12:34";
char tape[] = "Hallo! TinyDev T85 by MAXINT ";

const int spacer = 1;
const int width = fontwidth + spacer; // The font width is 5 pixels


void setup()
{
  //Blink();
  pinMode(LED_BUILTIN, OUTPUT);     // default pin 8 on ATtiny44/ATtinyCore 

  beginRTC();
  //Blink();
}

void showNum(uint32_t dwNum)
{ // show a number value on the ticker tape
  tape[0]='-';
  for(byte n=5; n>0; n--)
  {
    byte bNum=dwNum%10;
    dwNum=dwNum/10;
    tape[n]='0'+bNum;
  }
  tape[6]='.';
  tape[7]=' ';
}

void showTime()
{ // get the time and put it in the ticker tape (hh:mm:ss)
  getRTC();
  tape[0]='0'+ _hrs / 16;
  tape[1]='0'+ _hrs % 16;
  tape[2]=':';
  tape[3]='0'+ _mins / 16;
  tape[4]='0'+ _mins % 16;
//  tape[5]=':';
//  tape[6]='0'+ _secs / 16;
//  tape[7]='0'+ _secs % 16;
  tape[5]=' ';
  tape[6]=0;
}

uint32_t dwButtons=0;

void showTicker()
{
  // display the ticker text  
  for(int i=0; i<width*strlen(tape) + SIZE_X - 1 - spacer; i++)
  {
    // see if a button was pressed. (Unfortunately there is no room in the ATtiny44 for the TM16xxButtons class)
    dwButtons=module.getButtons();
    if(dwButtons!=0L)
    {
      while(module.getButtons()!=0);    //  wait for release
      break;
    }
      
    fillScreen(false,false);  // clear without sending

    int letter = i/width; /* fontwidth+spacer */
    int x = (SIZE_X - 1) - i % width;

    while(x+width-spacer >= 0 && letter >= 0)
    {
      if(letter<strlen(tape))
        drawChar(tape[letter], x, 0, false);

      letter--;   // also draw previous letter
      x -= width;
    }
    display(); // Send bitmap to display
    delay(70);
  }
}

void handleButtonsT44()
{
  if(dwButtons!=0)
  {
    getRTC();
    _hrs=BcdToUint8(_hrs);
    _mins=BcdToUint8(_mins);
    switch(dwButtons)
    {
      case _BV(8):
        _hrs++;
        break;
      case _BV(9):
        _mins++;
        break;
    }
    setRTC(Uint8ToBcd(_hrs%24), Uint8ToBcd(_mins%60));      // Set the time as specified
  }
}

uint8_t _nMode=0;
void handleButtonsT85()
{
  if(dwButtons!=0)
  {
    //Blink(200);  // cheap waiting for button relesae
    // showNum(dwButtons);
    getRTC();
    _hrs=BcdToUint8(_hrs);
    _mins=BcdToUint8(_mins);
    switch(dwButtons)
    {
      case _BV(8):
        _hrs++;
        break;
      case _BV(9):
        _mins++;
        break;
      case _BV(10):
        Flash();
        _nMode++;
        if(_nMode>3) _nMode=0;
        drawChar('0'+_nMode);
        delay(100);
        if(_nMode==1) strcpy(tape, "1-mar-2019 ");   // TODO: read actual date
        if(_nMode==2) strcpy(tape, "YouTube.com/maxint-rd ");
        if(_nMode==3) strcpy(tape, "Hello world! ");
        break;
    }
    if(_nMode==0)
      setRTC(Uint8ToBcd(_hrs%24), Uint8ToBcd(_mins%60));      // Set the time as specified
  }
}

void loop()
{
  showTicker();  // display the ticker text  
  handleButtonsT85();
  if(_nMode==0)
    showTime();
}
