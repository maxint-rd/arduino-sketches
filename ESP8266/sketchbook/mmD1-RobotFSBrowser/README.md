# D1 Mini multi-I/O devboard
***Development board for D1 mini featuring multiple I/O interfaces***
 
Board layout:
```
   /----------------------------------------------------\
  / PWM1               Proto area 2              PWM2    \
  |                                                       \
  | PWR                                              I2C  |
  |                                                       |
  |       I/O                                       5V    |
  | SW1   GV70          BUZ                               | 
  |                 LDR                                   |
  |      D1 PINS               7-seg                      |
  |      D1 PINS               4-dig                      |
  |                 I2C SER                               |
  |   Proto area 1                        Proto area 3    |
  |                                                       |
  |      D1 PINS                                          |
  |      D1 PINS                                          |
  |                                                       |
  | SW2    I2C                  SW3                       |
  |        I2C                                       5V   |
  |                                                       |
  | PWR                                 RGB2          I2C |
  |                    Proto area 4                 RGB1  /
  \ PWM4                                         PWM3    / 
   \----------------------------------------------------/
```

## FEATURES/PINOUT
 
 Pin | Function                | ESP-8266 Pin | Device
 --- | ----------------------- | ------------ | ------
 TX  | TXD                     | TXD          | BT-TX
 RX  | RXD                     | RXD          | BT-RX
 A0  | Analog input, max 3.3V  | A0           | LDR
 D0  | IO, wakeup              | GPIO16       | 
 D1  | IO, SCL                 | GPIO5        | SCL
 D2  | IO, SDA                 | GPIO4        | SDA
 D3  | IO, 10k Pull-up         | GPIO0        | SW2 (low for UART flash boot)
 D4  | IO, 10k Pull-up, LED    | GPIO2        | SW1 (high for normal boot)
 D5  | IO, SCK                 | GPIO14       | TM1637-DIO
 D6  | IO, MISO                | GPIO12       | TM1637-CLK
 D7  | IO, MOSI                | GPIO13       |
 D8  | IO, 10k Pull-down, SS   | GPIO15       | BUZ (low for normal boot)
 G   | Ground                  | GND          | 
 5V  | 5V                      | -            | 
 3V3 | 3.3V                    | 3.3V         | 
 RST | Reset                   | RST          | 
 
Note: On the D1 mini you indeed need to watch out how pins are used.
D3 and D4 have pull-ups. These are fine to use for a pushbutton, but cannot be used for devices
that pull the pin low. D8 has a pull down. It is okay when used to drive a buzzer through a transistor.
       
## On-board I2C DEVICES
- PCA9685 - 0x40/0x70 - 16-channel, 12-bit PWM Fm+ I2C-bus LED controller (address 0x70 for call all, A0-A5 for 0x40-0x7F)
- PCF8575 - 0x20 - 16-BIT I2C AND SMBus I/O Expander (address 0x20, A0-A2 for 0x20-0x27)
 
## On-board TM1637 LED&Key
- 4x7SEG A-H, DIG1-4
- RGB1 A-C, DIG5
- RGB2 A-C, DIG6
- SW3 Key 8-12 on K1 (0-4 on K2)

## Data upload
FSWebServer - Example WebServer with SPIFFS backend for esp8266 by Hristo Gochkov. 
Upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
Access the sample web page at http://esp8266fs.local
Edit the page by going to http://esp8266fs.local/edit

## Links
**2C scanner code:**
 -    https://playground.arduino.cc/Main/I2cScanner
 
**Virtual joystick funcionality:**
 -    https://automatedhome.party/2017/07/15/wifi-controlled-car-with-a-self-hosted-htmljs-joystick-using-a-wemos-d1-miniesp8266/
 -    based on http://jeromeetienne.github.io/virtualjoystick.js/examples/basic.html
 -    source: https://github.com/jeromeetienne/virtualjoystick.js
 
**Websocket functionality:**
 -    https://github.com/Links2004/arduinoWebSockets
 -    https://www.instructables.com/id/WiFi-WebSocket-Remote-Robot/
 -    https://github.com/moononournation/ESPWebSocketRemote
 
**MDNS multicast domain name system:**
 -    https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html


## Disclaimer
- All code on this GitHub account, including this library is provided to you on an as-is basis without guarantees and with all liability dismissed. It may be used at your own risk. Unfortunately I have no means to provide support.
