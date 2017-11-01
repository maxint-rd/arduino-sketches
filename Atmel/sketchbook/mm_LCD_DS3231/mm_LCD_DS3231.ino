/*
 * mm_LCD_DS3231 - Simple clock using the LCD and the DS3231 RTC.
 * 
 * This sketch is for a simple LCD clock. It simply displays time, date and temperature.
 * The time, date and temperature are obtained from a DS3231 RTC module.
 * They are displayed on an e.dentifier2 LCD panel connected via a 74HC595 shift register.
 * NOTE: The e.dentifier2 LCD requires 3.3V, DO NOT USE 5V
 * 
 * This sketch uses the following libraries:
 *  - Rtc by Makuna           - https://github.com/Makuna/Rtc
 *  - mxUnifiedIO             - https://github.com/maxint-rd/mxUnifiedIO
 *  - mxUnified74HC595        - https://github.com/maxint-rd/mxUnified74HC595
 *  - mxUnifiedLcdEdentifier2 - https://github.com/maxint-rd/mxUnifiedLcdEdentifier2
 *  - Adafruit GFX (v1.2.2)   - https://github.com/adafruit/Adafruit-GFX-Library
 *
 * This sketch is based on the DS3231_Simple example of the Rtc library, to 
 * which parts from the HC595_edent2_text_test example were added.
 * 
 */

/*
 * RTC includes and object initialisation.
 * 
 * RTC CONNECTIONS:
 * - DS3231 SDA --> SDA (Uno: SDA=A4, SCL=A5)
 * - DS3231 SCL --> SCL
 * - DS3231 VCC --> 3.3v
 * - DS3231 GND --> GND
 */

/* for software wire use below --
#include <SoftwareWire.h>  // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>

SoftwareWire myWire(SDA, SCL);
RtcDS3231<SoftwareWire> Rtc(myWire);
-- for software wire use above */

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */

/*
 * LCD includes and object initialisation
 * 
 * 74HC595 CONNECTIONS:
 * SPI hardware pins to 74HC595 shift register. (Uno: SS=10, MOSI=11, SCLK=13)
 * - HC595 pin 16 VCC --> 3.3v
 * - HC595 pin 14 serial (SER) to SPI-MOSI
 * - HC595 pin 13 enable (OE) --> GND
 * - HC595 pin 12 latch (RCLK) to SPI-SS
 * - HC595 pin 11 clock (SRCLK) to SPI-SCK
 * - HC595 pin 10 clear (SRCLR) --> VCC 3.3v
 * - HC595 pin 8 GND --> GND
 * 
 * E.DENTIFIER2 LCD CONNECTIONS:
 * - D0-D7 pins 7-14 --> HC595 Qa (pin 15) and Qb to Qh (pin 1-7)
 * - 3V VCC POWER+ pins 1,2,5 --> VCC 3.3v
 * - WR_CLK  Write clock pins 3,4 --> Arduino pin 8
 * - DC  Data/command pin 6 --> Arduino pin 9
 * - GND POWER- pin 15 --> GND
 */
#include <mxUnified74HC595.h>
#include <mxUnifiedLcdEdentifier2.h>

// include the fonts we will use to display text
#include <Fonts/FreeSansOblique12pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>

// declare the objects used for the LCD
mxUnified74HC595 unio = mxUnified74HC595();                  // use hardware SPI pins, no cascading (requires additional pins for DC and CLK
mxUnifiedLcdEdentifier2 display = mxUnifiedLcdEdentifier2(&unio, 8, 9);         // e.dentifier2 LCD: datapins P0-P7, DC=D8, CLK=D9 (DC and CLK are MCU pins)


void setup () 
{
  //
  //--------SERIAL SETUP ------------
  //
  Serial.begin(115200);
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  //
  //--------RTC SETUP ------------
  //
  // if you are using ESP-01 then uncomment the line below to reset the pins to
  // the available pins for SDA, SCL
  // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println();

  if (!Rtc.IsDateTimeValid()) 
  {
      // Common causes:
      //    1) first time you ran and the device wasn't running yet
      //    2) the battery on the device is low or even missing
      Serial.println("RTC lost confidence in the DateTime!");

      // following line sets the RTC to the date & time this sketch was compiled
      // it will also reset the valid flag internally unless the Rtc device is
      // having an issue
      Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();    // TODO: adjust for DST
  if (now < compiled) 
  {
      Serial.println("RTC is older than compile time!  (Updating DateTime)");
      Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) 
  {
      Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled) 
  {
      Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

  //
  //--------LCD SETUP ------------
  //
  unio.begin();     // start using the mxUnified74HC595 shift register
  unio.setBitOrder(LSBFIRST);        // change bitOrder to match pin layout of 595 shiftregister with e.dentifier2 display (LSB next to GND is Qh of 595)
  display.clearDisplay();   // clear buffer to skip splash-screen
  display.begin();  // regular begin() using default settings

  // show intro text using another font
  display.setRotation(2);  // rotate 180 degrees counter clockwise, can also use values of 2 and 3 to go further.
  display.clearDisplay();
  display.setFont(&FreeSerif9pt7b);
  display.setTextColor(BLACK);
  display.setCursor(0,14);    // different origin-system on fonts in rotated display
  display.print("e.dentifier2");
  display.setFont(&FreeSansOblique12pt7b);
  display.setCursor(0,34);    // different origin-system on fonts in rotated display
  display.print("Clock");
  display.display();
  delay(1000);
}

bool fBlink=false;
void loop () 
{
  char szTime[25];

  // check to see if time is still valid
  if (!Rtc.IsDateTimeValid()) 
  { // Time is invalid. Common causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
  }
  
  // get the date/time
  RtcDateTime now = Rtc.GetDateTime();

  // print the time
  sprintf(szTime, "%02u:%02u:%02u", now.Hour(), now.Minute(), now.Second() );
  Serial.print(szTime);
  display.clearDisplay();
  display.setFont(&FreeSansOblique12pt7b);
  display.setCursor(0,34);    // different origin-system on fonts in rotated display
  display.print(szTime);

  // print the date
  sprintf(szTime, "%02u-%02u", now.Day(), now.Month(), now.Year() );
  Serial.print("   -   ");
  Serial.print(szTime);
  display.setFont();
  display.setCursor(70,2);
  display.print(szTime);

  // print the temperature
  RtcTemperature temp = Rtc.GetTemperature();
  Serial.print("   -   ");
  Serial.print(temp.AsFloat());
  Serial.println("C");
  display.setFont(&FreeSerif9pt7b);
  display.setCursor(0,14);    // different origin-system on fonts in rotated display
  display.print(temp.AsFloat(),1);
  display.print("'C");

  // show a blinking dot on the display behind the C
  fBlink=!fBlink;
  if(fBlink)
    display.println(".");

  // update display multiple times per second to ensure no seconds are skipped on the display
  display.display();
  delay(250);
}

