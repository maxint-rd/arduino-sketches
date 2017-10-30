# mm_LCD_DS3231
Simple clock using the LCD and the DS3231 RTC.

This sketch is for a simple LCD clock. It simply displays time, date and temperature. The time, date and temperature are obtained from a DS3231 RTC module.They are displayed on an e.dentifier2 LCD panel connected via a 74HC595 shift register.
NOTE: The e.dentifier2 LCD requires 3.3V, DO NOT USE 5V
 
 This sketch uses the following libraries:
  - Rtc by Makuna           - https://github.com/Makuna/Rtc
  - mxUnifiedIO             - https://github.com/maxint-rd/mxUnifiedIO
  - mxUnified74HC595        - https://github.com/maxint-rd/mxUnified74HC595
  - mxUnifiedLcdEdentifier2 - https://github.com/maxint-rd/mxUnifiedLcdEdentifier2
  - Adafruit GFX (v1.2.2)   - https://github.com/adafruit/Adafruit-GFX-Library

This sketch is based on the DS3231_Simple example of the Rtc library, to which parts from the HC595_edent2_text_test example were added.
