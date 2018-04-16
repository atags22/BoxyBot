#include <WiFi.h>
#include <WiFiUdp.h>

#include "esp32-hal-ledc.h"


//Uncomment for debug information
//#define debugPrints

const char* ssid = "esp32tag";
const char* pass = "ardupass";

// IP address to send UDP data to.
// it can be ip address of the server or
// a network broadcast address
// here is broadcast address
const char * udpAddress = "192.168.4.2";
const int udpPort = 44444;

//create UDP instance
WiFiUDP udp;
int received = 0;



int extractNum(char* buf, int maxSize) {
  //get index of '['
  int openIndex = -1;
  for (int i = 0; i < maxSize; i++) {
    if (buf[i] == '[') {
      openIndex = i;
      break;
    }
    if (buf[i] == '\0') {
#ifdef debugPrints
      Serial.println("No [ found before null terminator");
#endif
      return 0; //Failure
    }
  }
  if (openIndex == -1) {
#ifdef debugPrints
    Serial.println("No [ found in buffer size");
#endif
    return 0; //Failure
  }

  //get index of ']'
  int closeIndex = -1;
  for (int i = openIndex; i < maxSize; i++) {
    if (buf[i] == ']') {

      closeIndex = i;
      break;
    }
    if (buf[i] == '\0') {
#ifdef debugPrints
      Serial.println("No ] found before null terminator");
#endif
      return 0; //Failure
    }
  }
  if (closeIndex == -1) {
#ifdef debugPrints
    Serial.println("No ] found in buffer size");
#endif
    return 0; //Failure
  }


  char strNum[closeIndex - openIndex];
#ifdef debugPrints
  Serial.print("Number size is ");
  Serial.println(closeIndex - openIndex - 1);
#endif
  int copyIndex = 0;
  for (int i = openIndex + 1; i < closeIndex; i++) { //If the code gets here, everything is in range and nothing is null
    strNum[copyIndex++] = buf[i];
  }
  return atoi((char*)strNum);

}

int servoToPwm(int normalRange) {
  int toReturn = 3300 + (17 * normalRange);
  return toReturn;
}
void setup() {
  Serial.begin(115200);

  //Connect to the WiFi network
  WiFi.softAP(ssid, pass);
  Serial.println("");

  // Wait for connection

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //This initializes udp and transfer buffer
  udp.begin(udpPort);

  ledcSetup(1, 50, 16); // channel 1, 50 Hz, 16-bit depth
  ledcAttachPin(22, 1);   // GPIO 22 on channel 1

  ledcSetup(2, 50, 16);
  ledcAttachPin(21, 2);
  //Example LED usage:
  //  while (true) {
  //    for (int i = 1; i < 180 ; i = i + 1) {
  //      ledcWrite(1, servoToPwm(i));       // sweep the servo
  //      delay(100);
  //    }
  //  }
}


void loop() {
  //data will be sent to server
  uint8_t buffer[50] = "hello world";
  //send hello world to server
  //  udp.beginPacket(udpAddress, udpPort);
  //  udp.write(buffer, 11);
  //  udp.endPacket();
  memset(buffer, 0, 50);
  //processing incoming packet, must be called before reading the buffer
  udp.parsePacket();
  //receive response from server, it will be HELLO WORLD
  if (udp.read(buffer, 50) > 0) {

    /***
       Heads up: If the data packet contains both a "Right" command
       and a "Left" command, only the "Right" command will be processed
       as of right now.
    */

    //    Serial.print("Server to client: ");
    if ((char) buffer[0] == 'R') {
      int parsedVal = extractNum((char*)buffer, 20);
      if (parsedVal > 0) {
        Serial.print("R ");
        Serial.println(parsedVal);
        ledcWrite(1, servoToPwm(parsedVal));
      }
    }
    if ((char) buffer[0] == 'L') {
      int parsedVal = extractNum((char*)buffer, 20);
      if (parsedVal > 0) {
        Serial.print("L ");
        Serial.println(parsedVal);
        ledcWrite(2, servoToPwm(parsedVal));
      }
    }


    received += 1;
    if (received % 100 == 0) {
      Serial.print("recv ");
      Serial.println(received);
    }
  }
  //Wait for 1 second
  //delay(1000);
}
