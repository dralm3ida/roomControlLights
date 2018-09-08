// ---------------------------------------------------------------------------
// Example NewPing library sketch that pings 3 sensors 20 times a second.
// ---------------------------------------------------------------------------

#include <NewPing.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <stdint.h>


#include "Timer.h"
#define SONAR_NUM 5    // Number of sensors.
#define MAX_DISTANCE 250 // Maximum distance (in cm) to ping.

//LDR sensor
const uint8_t LDRpin = A11;
uint16_t sensorValueLDR = 0;  // variable to store the value coming from the sensor

NewPing sonar[SONAR_NUM] = {   // Sensor object array.
  NewPing(41, 43, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping. //trig -> echo
  NewPing(37, 39, MAX_DISTANCE),
  NewPing(33, 35, MAX_DISTANCE),
  NewPing(29, 31, MAX_DISTANCE),
  NewPing(35, 27, MAX_DISTANCE)

};

//
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 2, 2);
IPAddress gateway(192, 168, 2, 1);
IPAddress submask(255, 255, 255, 0);
IPAddress ipDest(192, 168, 2, 254);
uint16_t localPort = 8888;      // local port to listen on
uint16_t destPort = 8888;      // local port to listen on

//class
Timer time;
EthernetUDP Udp;

bool newFilteredMeasurements = false;
void setup() {
  Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.

  //ethernet
  Ethernet.init(10);  // Most Arduino shields
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);

  //config timer for send message
   time.every(1000, takeSendingMessage, &newFilteredMeasurements );
  Serial.println("Start");
}

void loop() {
   time.update();
  delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  sensorValueLDR = ldrSensor();
  if (newFilteredMeasurements)
  {

    Udp.beginPacket(ipDest, destPort);
    // Print the measurements nicely
    Serial.print("S");
    Udp.write("S");
    for (int i = 0; i < SONAR_NUM; i++) {
      Serial.print(",");
      Serial.print(sonar[i].ping_cm());
      Udp.write(sonar[i].ping_cm());
    }

    Udp.write(sensorValueLDR);
    Udp.endPacket();

    Serial.print(",");
    Serial.print(sensorValueLDR);
    Serial.println(",");
    newFilteredMeasurements = false; //flag de nova lentura
  }
}

//**********************************************************************************************************
/*
   read and filter of ldr
   kalman filter
*/
uint16_t ldrSensor() {

  return map(analogRead( LDRpin ), 0, 1023, 0, 255);
}

//**********************************************************************************************************
///*
//   envio de informação
//*/
//
void takeSendingMessage()
{
  newFilteredMeasurements = true;
}
