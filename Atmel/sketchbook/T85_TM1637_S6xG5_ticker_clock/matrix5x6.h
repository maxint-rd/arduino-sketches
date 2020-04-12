#include "smallfonts5x6.h"

#define S6xG5_OPT_MIRROR_SEG true

/*   6x5 number codes */
/*
#define zero_code                  0b00001100010010010010010010001100
#define one_code                   0b00000100001100000100000100001110
#define two_code                   0b00001100010010000100001000011110
#define three_code                 0b00001100010010000100010010001100
#define four_code                  0b00010010010010011110000010000010
#define five_code                  0b00011110010000011100000010011100
#define six_code                   0b00000100001000011100010010001100
#define seven_code                 0b00011110000010000100001000010000
#define eight_code                 0b00001100010010001100010010001100
#define nine_code                  0b00001100010010001110000010001100
const uint32_t arrCodes[] PROGMEM={zero_code,one_code,two_code,three_code,four_code,five_code,six_code,seven_code,eight_code, nine_code};
*/

/*   

6x5 Byte codes
==========
?!!!? - large box
@^R^@ - small box
@^^^@ - small square
@@L@@ - dash
R@!RL - smiley
@@!RL - smile
@@LR! - sad
*/

/*
void setDisplayToBitmapString(char *szBmp)
{   // set the display using a string or an array of bytes
  for(int nPos=0; nPos<5; nPos++)
  {
    module.setSegments(szBmp[nPos], nPos);
  }
}
*/



void setDisplayToBitmap32(uint32_t dwBitmap, bool fReverse=S6xG5_OPT_MIRROR_SEG)
{ // set the display to a bitmap that is packed into a 32 bit value as 5 6-bit values 
  for(int nPos=0; nPos<5; nPos++)   // 5 grid x egments
  {
    byte bColumn=0x3F & (dwBitmap>>(6*(nPos)));   // use only six bits of the code
    module.setSegments(bColumn, fReverse?4-nPos:nPos);
  }
}

uint32_t dwMemoryBitmap=0L;
void display()
{ // update the display by sending the memory bitmap to the display
  setDisplayToBitmap32(dwMemoryBitmap);
}

#define SIZE_X 5
#define SIZE_Y 6
#define SIZE_MATRIX SIZE_X*SIZE_Y

void setAllPixels(bool fOn=true, bool fDisplayNow=true)
{ // set all pixels in the matrix
  dwMemoryBitmap= (fOn ? 0x3FFFFFFF:0x0);
  if(fDisplayNow) display();
}
#define fillScreen setAllPixels

void setPixel(int8_t x, int8_t y, bool fOn=true, bool fDisplayNow=true)
{ // set a pixel in the memory bitmap
  if(x<0 || x >= SIZE_X || y<0 || y >= SIZE_Y)
    return;

  //uint32_t nBits=(1L << (x*SIZE_Y + y));        // use grid-rows for x, segments for y, origin grid=1, seg=1
  uint32_t nBits=(1L << (SIZE_MATRIX-1 - (x)*SIZE_Y - y));        // use grid-rows for x, segments for y, origin grid=5, seg=6
  if(fOn)
    dwMemoryBitmap |= nBits;
  else
    dwMemoryBitmap &= ~nBits;

  if(fDisplayNow) display();
}

const int fontwidth = 5;
const int fontheight = 6;
void drawChar(unsigned char c, int8_t x=0, int8_t y=0, bool fDisplayNow=true)
{ // Draw a character at the specified position by updating the memory bitmap
  // (Loosely based on Adafruit_GFX  drawChar function)

  // clip characters that are entirely outside of the matrix
  if((x >= SIZE_X)            || // Clip right matrix:SIZE_X
     (y >= SIZE_Y)           || // Clip bottom matrix:SIZE_Y
     ((x + fontwidth  - 1) < 0) || // Clip left (fontwidth=5)
     ((y + fontheight  - 1) < 0))   // Clip top  (fontheight=6)
      return;

  for(int8_t i=0; i<5; i++ )
  { // Char bitmap = 5 columns
    uint8_t line = pgm_read_byte(&Small_Fonts5x6[(c-32) * 5 + i]);
    for(int8_t j=0; j<8; j++, line >>= 1)
      setPixel(x+i, y+j, line & 1, false);
  }

  if(fDisplayNow) display();
}

/*
void setDisplayToNum(byte bNum)
{   // set the display to an array of numbers using a string or an array of codes
  setDisplayToBitmap32(pgm_read_dword(arrCodes+bNum), true);    // use reversed positioning when S1,G1 is on top-right 
}

void setDisplayToChar(char c)
{   // set the display to an array of numbers using a string or an array of codes
  //setDisplayToBitmapString(Small_Fonts5x6 + (c*5));
  c=c-32; // space #32 =0
  for(int nPos=0; nPos<5; nPos++)
  {
    module.setSegments(pgm_read_byte(Small_Fonts5x6 + (c*5) + nPos), nPos);
  }
}
*/
