/*
 * Maxint R&D
 * D1 Mini multi-I/O devboard
 * 
 * Development board for D1 mini featuring multiple I/O interfaces
 * 
 * Board layout:
 *   /----------------------------------------------------\
 *  / PWM1               Proto area 2              PWM2    \
 *  |                                                       \
 *  | PWR                                              I2C  |
 *  |                                                       |
 *  |       I/O                                       5V    |
 *  | SW1   GV70          BUZ                               | 
 *  |                 LDR                                   |
 *  |      D1 PINS               7-seg                      |
 *  |      D1 PINS               4-dig                      |
 *  |                 I2C SER                               |
 *  |   Proto area 1                        Proto area 3    |
 *  |                                                       |
 *  |      D1 PINS                                          |
 *  |      D1 PINS                                          |
 *  |                                                       |
 *  | SW2    I2C                  SW3                       |
 *  |        I2C                                       5V   |
 *  |                                                       |
 *  | PWR                                 RGB2          I2C |
 *  |                    Proto area 4                 RGB1  /
 *  \ PWM4                                         PWM3    / 
 *   \----------------------------------------------------/
 *   
 * FEATURES/PINOUT
 * 
 * Pin  Function                 ESP-8266 Pin  Device
 * TX   TXD                      TXD           BT-TX
 * RX   RXD                      RXD           BT-RX
 * A0   Analog input, max 3.3V   A0            LDR
 * D0   IO, wakeup               GPIO16
 * D1   IO, SCL                  GPIO5         SCL
 * D2   IO, SDA                  GPIO4         SDA
 * D3   IO, 10k Pull-up          GPIO0         SW2 (low for UART flash boot)
 * D4   IO, 10k Pull-up, LED     GPIO2         SW1 (high for normal boot)
 * D5   IO, SCK                  GPIO14        TM1637-DIO
 * D6   IO, MISO                 GPIO12        TM1637-CLK
 * D7   IO, MOSI                 GPIO13
 * D8   IO, 10k Pull-down, SS    GPIO15        BUZ (low for normal boot)
 * G    Ground                   GND
 * 5V   5V                       -
 * 3V3  3.3V                     3.3V
 * RST  Reset                    RST
 * 
 * Note: On the D1 mini you indeed need to watch out how pins are used.
 * D3 and D4 have pull-ups. These are fine to use for a pushbutton, but cannot be used for devices
 * that pull the pin low. D8 has a pull down. It is okay when used to drive a buzzer through a transistor.
 *       
 * On-board I2C DEVICES
 * PCA9685 - 0x40/0x70 - 16-channel, 12-bit PWM Fm+ I2C-bus LED controller (address 0x70 for call all, A0-A5 for 0x40-0x7F)
 * PCF8575 - 0x20 - 16-BIT I2C AND SMBus I/O Expander (address 0x20, A0-A2 for 0x20-0x27)
 * 
 * On-board TM1637 LED&Key
 * 4x7SEG A-H, DIG1-4
 * RGB1 A-C, DIG5
 * RGB2 A-C, DIG6
 * SW3 Key 8-12 on K1 (0-4 on K2)
 * 
 * I2C scanner code from  from https://playground.arduino.cc/Main/I2cScanner
 * 
 */


// The onboard PCA9685 chip has 16 PWM pins, which can be used to drive LEDs and servos directly. Motors can be driven via a motor driver board
// PCA9685 - 0x40/0x70 - 16-channel, 12-bit PWM Fm+ I2C-bus LED controller (address 0x70 for call all, A0-A5 for 0x40-0x7F)
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(&Wire, 0x40);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(); // called this way, it uses the default address 0x40

// The onboard PCF8574 chip has 16 digital I/O pins, which can be used for sensors, LEDs, buttons and other I/O devices
// PCF8575 - 0x20 - 16-BIT I2C AND SMBus I/O Expander (address 0x20, A0-A2 for 0x20-0x27)
#include <mxUnifiedPCF8574.h>
mxUnifiedPCF8575 unio = mxUnifiedPCF8575(0x20);     // use the PCF875 I2C output expander on address 0x20


// The buzzer is connected via a transistor to D8
// The MmlMusicPWM library uses a timer to play music without interruptions
#define PIN_BUZZER D8
#include <MmlMusicPWM.h>
MmlMusicPWM music(PIN_BUZZER);

// A light sensitive resistor is connected to A0
#define LDR A0

// The TM11637 chip is connected via a DIO and a CLK line
// It drives a 4 x 7-segment display and separate RGB leds and can also scan for button presses
#define PIN_DIO D5
#define PIN_CLK D6

#include <TM1637.h>
#include <TM16xxDisplay.h>
#include <TM16xxButtons.h>
TM1637 module(PIN_DIO, PIN_CLK);
TM16xxDisplay display(&module, 4);    // TM16xx object, 4 digits
TM16xxButtons buttons(&module);

#ifndef DBG_OUTPUT_PORT
#define DBG_OUTPUT_PORT Serial
#endif


//----------------FIX ESP8266 ISSUE WITH TICKER AND DELAY -----------
#define OPT_FIXDELAY 0
#if OPT_FIXDELAY
void msDelay(unsigned int nDuration)
{ // replacement for delay() which in ESP8266 core seems to conflict usage of timer scheduler by Ticker which is used in MmlMusicPWM
  uint32_t ulStart=millis();
  while(millis() < ulStart+nDuration)
    yield();
}

// Redefine delay() which seems to collide with Ticker used in MmlMusicPWM (when playing during delay).
#define delay(x) msDelay(x)
#endif
//-----------------------------------------------------------


void blink(int nTimes=1, int nDelay=100)
{
  pinMode(LED_BUILTIN, OUTPUT);     // D1-MINI: LED_BUILTIN=GPIO2/D4, 10k Pull-up, connected to SW1
  for(int n=0; n<nTimes; n++)
  {
    digitalWrite(LED_BUILTIN, LOW);   // set LOW to switch led on
    delay(nDelay);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(nDelay);
  }
  pinMode(LED_BUILTIN, INPUT);     // D1-MINI: LED_BUILTIN=GPIO2/D4, 10k Pull-up, connected to SW1
}

#define RGB_RED B001
#define RGB_GREEN B010
#define RGB_BLUE B100
#define RGB_YELLOW B011
#define RGB_AQUA B110
#define RGB_MAGENTA B101
#define RGB_WHITE B111
#define RGB_NONE B000

void rgbSet(byte nLed, byte nColor)
{ // Two RGB-LEDs are connected to the TM1637 chip on segments A,B and C of digits 5 and 6
  // Their color can be set using a 3-bit RGB value, eg. rgbSet(1, B010) for green on RGB1 or rgbSet(2, B101) for purple on RGB2
  if(nLed<1 || nLed>2) return;
  module.setSegments(nColor, nLed+3);
}

void playTune(const char *szPlay)
{ // play a tune if nothing else is playing
  if(!music.isPlaying())
    music.play(szPlay);
}

// define some patterns to blink LEDs on the PCF ports P4-P12
// format: speed, count, pattern[x], ...
const byte pcfLoopPattern0[] PROGMEM = {100, 10, 0x80, 0, 0, 0, 0, 0x01, 0, 0, 0, 0};
const byte pcfLoopPattern1[] PROGMEM = {200, 2, 0xAA, 0x55};
const byte pcfLoopPattern2[] PROGMEM = {100, 14,
  0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000,
  0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010};
const byte pcfLoopPattern3[] PROGMEM = {100, 8, 0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000};
const byte pcfLoopPattern4[] PROGMEM = {100, 8, 0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001};
const byte pcfLoopPattern5[] PROGMEM = {100, 4, 0b10000001, 0b01000010, 0b00100100, 0b00011000};
const byte pcfLoopPattern6[] PROGMEM = {100, 4, 0b00011000, 0b00100100, 0b01000010, 0b10000001};

const byte pcfLoopPattern7[] PROGMEM = {100, 8, 0b10000001, 0b01000010, 0b00100100, 0b00011000, 0b00011000, 0b00100100, 0b01000010, 0b10000001};

const byte *pcfLoopPatterns[]={
  pcfLoopPattern0,
  pcfLoopPattern1,
  pcfLoopPattern2,
  pcfLoopPattern3,
  pcfLoopPattern4,
  pcfLoopPattern5,
  pcfLoopPattern6,
  pcfLoopPattern7,
};

byte pcfLoopPatternPlaying=0;
byte pcfLoopPatternPosition=1;
byte pcfLoopPatternSpeed=0;

void pcfLedsShow(byte btLeds)
{
  for (uint8_t pin=4; pin<=11; pin++)
  {
    unio.digitalWrite(pin, btLeds&0x01);
    btLeds>>=1;
  }
}

void pcfLedsSetPattern(byte nPatternNum=0, byte nSpeed=0)
{
  pcfLoopPatternPlaying=nPatternNum;
  pcfLoopPatternSpeed= (nSpeed==0 ? pgm_read_byte(pcfLoopPatterns[pcfLoopPatternPlaying]) : nSpeed);
  byte nCount=pgm_read_byte(pcfLoopPatterns[pcfLoopPatternPlaying]+1);
//  pcfLoopPatternPosition=1;
  DBG_OUTPUT_PORT.printf("Led pattern %d [size %d], speed %d\n", nPatternNum, nCount, pcfLoopPatternSpeed);
}

void pcfLedsSetup()
{   // use the 16-bit I/O expander to drive 8 leds on P4-P12
  for (uint8_t n=0; n<8; n++)
  {
    pcfLedsShow(bit(n));
    delay(10);
    pcfLedsShow(0);
  }
  pcfLedsSetPattern(0);
}

unsigned long ulLedsStart=0;
void pcfLedsLoop()
{
  if(millis()>ulLedsStart+pcfLoopPatternSpeed)
  {
    ulLedsStart=millis();
    pcfLoopPatternPosition++;
    if(pcfLoopPatternPosition>pgm_read_byte(pcfLoopPatterns[pcfLoopPatternPlaying]+1))
      pcfLoopPatternPosition=1;
  //byte nCount=pgm_read_byte(pcfLoopPatterns[pcfLoopPatternPlaying]+1);
  //DBG_OUTPUT_PORT.printf("Led pattern%d [size %d]: speed %d, pos %d\n", pcfLoopPatternPlaying, nCount, pcfLoopPatternSpeed, pcfLoopPatternPosition);
    pcfLedsShow(pgm_read_byte(pcfLoopPatterns[pcfLoopPatternPlaying]+pcfLoopPatternPosition+1));
  }
}



//=============================

bool TM1637PrintScrolled(const char *pszText=NULL, bool fRepeatText=false, uint16_t nTimeDelay=333)
{ // print scrolled text, or continue printing the previous text until done
  // return true if still more text to print, or false if done
  static const char *pText=NULL;
  static int nPos=0;
  static long lTime=0;
  static bool fRepeat=false;
  static uint16_t nDelay=false;
  #define NUM_DIGITS 4

  //DBG_OUTPUT_PORT.print("TM1637PrintScrolled Pos:");
  //if(pText!=NULL)
  //   DBG_OUTPUT_PORT.print(nPos);
  if(pszText!=NULL)
  { // init new text
    pText=pszText;
    //nPos=NUM_DIGITS;    // start right
    nPos=0;               // start left
    lTime=0;
    fRepeat=fRepeatText;
    nDelay=nTimeDelay;
    display.clear();
    //display.println(pText);
/*
    DBG_OUTPUT_PORT.print(" - ");
    DBG_OUTPUT_PORT.print(pszText);
    DBG_OUTPUT_PORT.print("[");
    DBG_OUTPUT_PORT.print(strlen(pText));
    DBG_OUTPUT_PORT.print("]");
    DBG_OUTPUT_PORT.println(".");
*/
    return(true);   // still printing
  }
  else
  { // continue showing previous text
    //DBG_OUTPUT_PORT.print("-");
    if(pText==NULL)
       return(false);   // nothing to print

    if(millis()>lTime+nDelay)
    {
/*
      Serial.print("~");
    Serial.print("[");
    Serial.print(nPos, DEC);
    Serial.print("]");
*/
      lTime=millis();
      if(nPos > (0-((int)strlen(pText))-1))   // on ESP8266: size_t is unsigned. Must cast strlen() to int to compare negative values
      { // scroll to next position
        display.setCursor(nPos);
        display.println(pText);
        nPos--;   // cursor position can be negative to scroll text by starting to print it outside of visible area
        //Serial.println(">");
        return(true);   // still printing
      }

      if(nPos == (0-((int)strlen(pText))-1))
      { // At end of string. Repeat or stop.
        display.clear();
        if(fRepeat)
          nPos=NUM_DIGITS;
        else
          pText=NULL;
        //DBG_OUTPUT_PORT.println("!");
        return(false);   // done printing
      }
    }
    return(true);   // not finished printing
  }
}



//=============================
// PCA9685 PWM motor
//=============================

void mxD1_multi_setupPCA(byte nAddress=0x40)
{   // setup the PCA9685 PWM driver
  byte error;

  DBG_OUTPUT_PORT.println(F("setup: PCA9685"));
  Wire.begin();
  Wire.beginTransmission(nAddress);
  error = Wire.endTransmission();

  if(error!=0)
  { // print error
    DBG_OUTPUT_PORT.print("No I2C device found at address 0x");
    if (nAddress<16)
      DBG_OUTPUT_PORT.print("0");
    DBG_OUTPUT_PORT.print(nAddress,HEX);
    DBG_OUTPUT_PORT.println("!");
    return;
  }

  pwm.begin();
  pwm.setPWMFreq(1000);  // use higher pwm frequency for motors (servo's use 60 Hz)

  // set all pwm pins off
  for (uint8_t pin=0; pin<15; pin++)
    pwm.setPWM(pin, 0, 4096);       // turns pin fully off
}

void mxD1_multi_motors(bool fDir, int nSpeedA, int nSpeedB)
{   // drive the motors connected to PWM pins 12-15
  pwm.setPWMFreq(200);  // use lower pwm frequency
  #define MOTOR_MIN 1000     // minimum speed needed to start (L9110 driver)

  if(nSpeedA==0 && nSpeedB==0)
  { // stop
    DBG_OUTPUT_PORT.printf("PCA Motors off!\n");
    pwm.setPWM(12, 0, 4096);         // turns pin fully off
    pwm.setPWM(13, 0, 4096);         // turns pin fully off
    pwm.setPWM(14, 0, 4096);         // turns pin fully off
    pwm.setPWM(15, 0, 4096);         // turns pin fully off
    rgbSet(1, RGB_NONE);
    rgbSet(2, RGB_NONE);
    pcfLedsSetPattern(2);
    display.println("stop");
    //TM1637PrintScrolled("stop");
    return;
  }
//  nSpeedA = constrain(nSpeedA, MOTOR_MIN, 4095);
//  nSpeedB = constrain(nSpeedB, MOTOR_MIN, 4095);

  if(fDir)
  {
    DBG_OUTPUT_PORT.printf("PCA Motors fwd: %d, %d\n", nSpeedA, nSpeedB);
    pwm.setPWM(12, 0, nSpeedA*4);    // turns pin partially on
    pwm.setPWM(13, 0, 4096);         // turns pin fully off
    pwm.setPWM(14, 0, nSpeedB*4);    // turns pin partially on
    pwm.setPWM(15, 0, 4096);         // turns pin fully off
    rgbSet(1, (nSpeedA-nSpeedB)>50 ? RGB_GREEN : RGB_YELLOW);
    rgbSet(2, (nSpeedB-nSpeedA)>50 ? RGB_GREEN : RGB_YELLOW);
    pcfLedsSetPattern((nSpeedA-nSpeedB)>50 ? 3 : ((nSpeedB-nSpeedA)>50 ? 4 : 5), 255-((nSpeedA+nSpeedB)/10)%255);
    display.println("for");
    //TM1637PrintScrolled("for");
  }
  else
  {
    DBG_OUTPUT_PORT.printf("PCA Motors bck: %d, %d\n", nSpeedA, nSpeedB);
    pwm.setPWM(12, 0, 4096);         // turns pin fully off
    pwm.setPWM(13, 0, nSpeedA*4);    // turns pin partially on
    pwm.setPWM(14, 0, 4096);         // turns pin fully off
    pwm.setPWM(15, 0, nSpeedB*4);    // turns pin partially on
    rgbSet(1, (nSpeedA-nSpeedB)>50 ? RGB_RED : RGB_MAGENTA);
    rgbSet(2, (nSpeedB-nSpeedA)>50 ? RGB_RED : RGB_MAGENTA);
    pcfLedsSetPattern((nSpeedA-nSpeedB)>50 ? 3 : ((nSpeedB-nSpeedA)>50 ? 4 : 6), 255-((nSpeedA+nSpeedB)/10)%255);
    display.println("bck");
    //TM1637PrintScrolled("bck");
  }
}



//=============================

void fnClick(byte nButton)
{ // byte nButton is the button-number (first button is number 0)
  DBG_OUTPUT_PORT.print(F("Button "));
  DBG_OUTPUT_PORT.print(nButton);
  DBG_OUTPUT_PORT.println(F(" click."));
}


void mxD1_multi_setup()
{
  blink(3);

  DBG_OUTPUT_PORT.println(F("TM1637 4 x 7-segment Display"));
  display.setIntensity(7);
  display.println("boot");    // F("xyz") macro doesn't work for TM16xxDisplay
  //delay(5000);
  display.setIntensity(1);

  DBG_OUTPUT_PORT.println(F("TM1637 2 x RGB"));
  rgbSet(1, RGB_YELLOW);      // set RGB1 to green
  rgbSet(2, RGB_MAGENTA);      // set RGB2 to purple

  buttons.attachClick(fnClick);

  mxD1_multi_setupPCA();
  pcfLedsSetup();

  playTune("T240 L16 O8 FEDCDEF r1 "); // give a short blurp
}

void mxD1_multi_loop()
{
  //blink(1,10);
  buttons.tick();

  // update the leds on the PCF port
  pcfLedsLoop();


  // print remainder of scrolling text
  bool fPrintingScrolled=TM1637PrintScrolled();


}
