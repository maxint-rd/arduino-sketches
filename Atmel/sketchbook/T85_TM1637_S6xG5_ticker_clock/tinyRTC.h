#include <TinyI2CMaster.h>      // ATtiny44A: SCL=4, SDA=6, NOTE: commented out  #define TWI_FAST_MODE
//#include <Wire.h>     // Using wire is too large on ATtiny44 600 bytes more flash and 40 more bytes RAM

// utility functions obtained from RTC by Makuna
uint8_t BcdToUint8(uint8_t val)
{
    return val - 6 * (val >> 4);
}

uint8_t Uint8ToBcd(uint8_t val)
{
    return val + 6 * (val / 10);
}


// Digital clock **********************************************
// See http://www.technoblogy.com/show?24R7
const int RTCaddress = 0x68;

void beginRTC()
{
  TinyI2C.init();
//  Wire.begin();        // join i2c bus (address optional for master)
  //setRTC(0x12, 0x34);      // Set the time to 12:34
  // todo: set to compiled time if compiled is larger than now __TIME__
}

void setRTC(int hr, int min)
{
  TinyI2C.start(RTCaddress, 0);
  TinyI2C.write(0);     // write register 0
  TinyI2C.write(0);     // write sec
  TinyI2C.write(min);   // write min
  TinyI2C.write(hr);    // write hr
  TinyI2C.stop();
}

int _secs;
int _mins;
int _hrs;

void getRTC(void)
{
  // Read the time from the RTC
  TinyI2C.start(RTCaddress, 0);
  TinyI2C.write(0);     // write register 0
  TinyI2C.restart(RTCaddress, 3);  // read 3 bytes
  _secs = TinyI2C.read();     // read sec
  _mins = TinyI2C.read();     // read min
  _hrs = TinyI2C.read();     // read hr
  TinyI2C.stop();
/*
  Wire.beginTransmission(RTCaddress); // transmit to device #8
  Wire.write(0);     // write register 0
  Wire.endTransmission();    // stop transmitting
  Wire.requestFrom(0x68, 3);    // request 3 bytes from slave device #x68
  _secs = Wire.read();     // read sec
  _mins = Wire.read();     // read min
  _hrs = Wire.read();     // read hr
*/
/*
  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
*/
}

