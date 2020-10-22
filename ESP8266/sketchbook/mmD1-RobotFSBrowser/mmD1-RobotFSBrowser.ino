/*
  FSWebServer - Example WebServer with SPIFFS backend for esp8266
  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WebServer library for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  upload the contents of the data folder with MkSPIFFS Tool ("ESP8266 Sketch Data Upload" in Tools menu in Arduino IDE)
  or you can upload the contents of a folder if you CD in that folder and run the following command:
  for file in `\ls -A1`; do curl -F "file=@$PWD/$file" esp8266fs.local/edit; done

  access the sample web page at http://esp8266fs.local
  edit the page by going to http://esp8266fs.local/edit
*/

/*
 * Virtual joystick funcionality:
 *    https://automatedhome.party/2017/07/15/wifi-controlled-car-with-a-self-hosted-htmljs-joystick-using-a-wemos-d1-miniesp8266/
 *    based on http://jeromeetienne.github.io/virtualjoystick.js/examples/basic.html
 *    source: https://github.com/jeromeetienne/virtualjoystick.js
 * 
 * Websocket functionality:
 *    https://github.com/Links2004/arduinoWebSockets
 *    https://www.instructables.com/id/WiFi-WebSocket-Remote-Robot/
 *    https://github.com/moononournation/ESPWebSocketRemote
 *    
 * MDNS multicast domain name system:
 *    https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#define DBG_OUTPUT_PORT Serial

#include <WiFiManager.h>

#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-key"
#endif
//const char* ssid = STASSID;
//const char* password = STAPSK;
const char* host = "esp8266fs";     // mDNS name: <host>.local (not supported on Android!)

ESP8266WebServer server(80);

#include <WebSocketsServer.h>
WebSocketsServer webSocket = WebSocketsServer(81);
String fragmentBuffer = "";   // to hold stringfragments for longer Websocket commands

////////////////////////////////////////////////////////
// mx D1 multi-io functions
#include "mxD1_multi.h"


////////////////////////////////////////////////////////
// FSBrowser
#include "FSBrowser.h"


////////////////////////////////////////////////////////
// Motor remote control via virtual joystick or other device
void motorRemote(int x, int y)
{  // engage the motor based on received remote control commands where x, y range between -100 and 100
  x=x*10;
  y=y*10;
  boolean yDir;
  int aSpeed = abs(y);
  int bSpeed = abs(y);

  //set the direction based on y being negative or positive
  if ( y < 0 ){
    yDir = 0; 
  }
  else { 
    yDir = 1;
  }  
  //adjust to speed of each each motor depending on the x-axis
  //it slows down one motor and speeds up the other proportionately 
  //based on the amount of turning
  aSpeed = constrain(aSpeed + x/2, 0, 1023);
  bSpeed = constrain(bSpeed - x/2, 0, 1023);

  DBG_OUTPUT_PORT.printf("Motor remote: %s %d-%d, %d-%d\n", (yDir?"fwd":"bck"), x, aSpeed, y, bSpeed);
  mxD1_multi_motors(yDir, aSpeed, bSpeed);
/*
  //use the speed and direction values to turn the motors
  //if either motor is going in reverse from what is expected,
  //just change the 2 digitalWrite lines for both motors:
  //!ydir would become ydir, and ydir would become !ydir
  digitalWrite(STBY, HIGH);  
  //MotorA
  digitalWrite(AIN1, !yDir);
  digitalWrite(AIN2, yDir);
  analogWrite(PWMA, aSpeed);
  //MotorB
  digitalWrite(BIN1, !yDir);
  digitalWrite(BIN2, yDir);
  analogWrite(PWMB, bSpeed);
*/
}

//////////////////////////////////////////////////////////////////
// handle commands received via javascript Ajax over HTTP
//This function takes the parameters passed in the URL(the x and y coordinates of the joystick)
//and sets the motor speed based on those parameters. 
// format: ./jsData.html?x=12&y=34
void handleJSData()
{
  int x = server.arg(0).toInt();
  int y = server.arg(1).toInt();
  DBG_OUTPUT_PORT.printf("JS: Joy %d, %d\n", x, y);
  motorRemote(x,y);

  //return an HTTP 200
  server.send(200, "text/plain", "");   
}



//////////////////////////////////////////////////////////////////
// Handle commands received via websockets (much faster than via HTTP Ajax)
// [@]. Motor control - format: @x,y
//
//
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch(type) {
    case WStype_DISCONNECTED:
      DBG_OUTPUT_PORT.printf("WS: [%u] Disconnected!\n", num);
      // TODO: set all robot actions off
      motorRemote(0,0);
      break;

    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      DBG_OUTPUT_PORT.printf("WS: [%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

      // send message to client
      webSocket.sendTXT(num, "Connected");
      display.println("CONN");
      //TM1637PrintScrolled("Connected ");
      playTune("T240 L8 O7 C r r C r16 E r16 F");
    }
      break;

    case WStype_TEXT:
      DBG_OUTPUT_PORT.printf("WS: [%u] get Text: %s\n", num, payload);
      if (payload[0] == '@')
      { // We received a virtual joystick command. Format: @x,y
        // parse payload to find x,y
        int x = atoi((const char *)payload+1);
        char * pComma=strchr((const char *)payload, ',');
        int y = pComma!=NULL ? atoi((const char *)pComma+1) : 0;
        DBG_OUTPUT_PORT.printf("WS: Joy %d, %d\n", x, y);
        motorRemote(x,y);
      }
      break;

    case WStype_BIN:
      DBG_OUTPUT_PORT.printf("WS: [%u] get binary length: %u\n", num, length);
      hexdump(payload, length);
      break;

    // Fragmentation / continuation opcode handling
    // case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT_TEXT_START:
      fragmentBuffer = (char*)payload;
      DBG_OUTPUT_PORT.printf("WS: [%u] get start start of Textfragment: %s\n", num, payload);
      break;
    case WStype_FRAGMENT:
      fragmentBuffer += (char*)payload;
      DBG_OUTPUT_PORT.printf("WS: [%u] get Textfragment : %s\n", num, payload);
      break;
    case WStype_FRAGMENT_FIN:
      fragmentBuffer += (char*)payload;
      DBG_OUTPUT_PORT.printf("WS: [%u] get end of Textfragment: %s\n", num, payload);
      DBG_OUTPUT_PORT.printf("WS: [%u] full frame: %s\n", num, fragmentBuffer.c_str());
      break;
  }

}


void configModeCallback (WiFiManager *myWiFiManager)
{
  DBG_OUTPUT_PORT.println(F("Entered config mode"));
  DBG_OUTPUT_PORT.println(WiFi.softAPIP());
  DBG_OUTPUT_PORT.println(myWiFiManager->getConfigPortalSSID());
  display.println("Conf");
}




void setup(void)
{
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.print("mxD1-Robot FS/WS/vJoy.\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);   // enable printf and debug output from WiFi libraries

  mxD1_multi_setup();
  
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }


  //WIFI INIT
/*
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  }
*/

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  DBG_OUTPUT_PORT.printf("Starting WiFiManager for %s\n", host);
  display.println("IP=");  
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  //wifiManager.autoConnect(MDNS_NAME);
  wifiManager.setDebugOutput(false);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect(host);
  //or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print(F("Connected! IP address: "));
  IPAddress ipAddress = WiFi.localIP();
  DBG_OUTPUT_PORT.println(ipAddress);
  display.println(ipAddress[3]);



/*
  if(MDNS.begin(host))
    DBG_OUTPUT_PORT.printf("MDNS responder started for %s\n", host);
  else
    DBG_OUTPUT_PORT.printf("MDNS responder NOT started for %\n", host);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");
*/

  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  //set the static pages on SPIFFS for the html and js
  //server.serveStatic("/", SPIFFS, "/joystick.html"); 
  //server.serveStatic("/virtualjoystick.js", SPIFFS, "/virtualjoystick.js");
  //call handleJSData function when this URL is accessed by the js in the html file
  server.on("/jsData.html", handleJSData); 

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  DBG_OUTPUT_PORT.println(F("HTTP server started"));

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  DBG_OUTPUT_PORT.println(F("Websocket server started"));

/*
  if (MDNS.begin(MDNS_NAME)) {
    USE_SERIAL.println("MDNS responder started");
  }
*/

  // start Multicast domain name system for <host>.local (not supported on Android)
  if(MDNS.begin(host))
    DBG_OUTPUT_PORT.printf("MDNS responder started for %s\n", host);
  else
    DBG_OUTPUT_PORT.printf("MDNS responder NOT started for %\n", host);

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  DBG_OUTPUT_PORT.print(F("Open http://"));
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(F(".local/edit to see the file browser"));

  //display.println("done");
  rgbSet(1, RGB_NONE);
  rgbSet(2, RGB_NONE);
}

void loop(void)
{
  server.handleClient();
  MDNS.update();
  webSocket.loop();
  mxD1_multi_loop();
}
