#include <Ethernet.h>
#include <EthernetUdp.h>
#include "commands.h"

#define PIN_LIGHTS_GROUP1   2
#define PIN_LIGHTS_GROUP2   3

unsigned char g_ultraSoundValues[] = {100, 20, 30, 48, 50, 68, 170, 89}; // 8 sensors, each one with range 0-256
unsigned char g_lightSensor = 1;
unsigned char g_pirSensor = 1;
unsigned char g_light_one_state = LOW;
unsigned char g_light_two_state = LOW;
byte g_init = 0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};
IPAddress ip(192, 168, 2, 254);
IPAddress gw(192, 168, 2, 1);
IPAddress mk(255, 255, 255, 0);
IPAddress ipUltrasound(192, 168, 2, 2);
unsigned int portUltrasound = 8888;
EthernetUDP Udp;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_LIGHTS_GROUP1, OUTPUT);
  pinMode(PIN_LIGHTS_GROUP2, OUTPUT);
  Serial.begin(9600);
  digitalWrite(PIN_LIGHTS_GROUP1, LOW);
  digitalWrite(PIN_LIGHTS_GROUP2, LOW);

  Ethernet.init(10);
  Ethernet.begin(mac, ip);
  Udp.begin(portUltrasound);
}

void loop() {
  int packetSize = Udp.parsePacket();
  int incomingByte = 0;

  if ( 0 == g_init )
  {
    //Udp.begin(portUltrasound);
    g_init = 1;
  }

  Serial.print("serial ");
  Serial.print(Serial.available());
  Serial.print(", ultrasound ");
  Serial.print(packetSize);
  Serial.println("");
  digitalWrite(LED_BUILTIN, LOW);
  if ( Serial.available() > 0 )
  {
    incomingByte = Serial.read();

    switch ( incomingByte )
    {
      case SERIAL_COMMAND_ULTRASOUND:
        Serial.write(g_ultraSoundValues, sizeof(g_ultraSoundValues));
        break;
      case SERIAL_COMMAND_LIGHT:
        Serial.write(g_lightSensor);
        break;
      case SERIAL_COMMAND_PIR:
        Serial.write(g_pirSensor);
        break;
      case SERIAL_COMMAND_ALL:
        Serial.write(g_ultraSoundValues, sizeof(g_ultraSoundValues));
        Serial.write(g_lightSensor);
        Serial.write(g_pirSensor);
        break;
      case SERIAL_COMMAND_VOICE_LIGHTSON:
        digitalWrite(PIN_LIGHTS_GROUP1, HIGH);
        digitalWrite(PIN_LIGHTS_GROUP2, HIGH);
        Serial.write(incomingByte);
        break;
      case SERIAL_COMMAND_VOICE_LIGHTSOFF:
        digitalWrite(PIN_LIGHTS_GROUP1, LOW);
        digitalWrite(PIN_LIGHTS_GROUP2, LOW);
        Serial.write(incomingByte);
        break;
      case SERIAL_COMMAND_VOICE_LIGHTSON1:
        digitalWrite(PIN_LIGHTS_GROUP1, g_light_one_state = HIGH);
        Serial.write(incomingByte);
        break;
      case SERIAL_COMMAND_VOICE_LIGHTSOFF1:
        digitalWrite(PIN_LIGHTS_GROUP1, g_light_one_state = LOW);
        Serial.write(incomingByte);
        break;
      case SERIAL_COMMAND_VOICE_LIGHTSON2:
        digitalWrite(PIN_LIGHTS_GROUP2, g_light_two_state = HIGH);
        Serial.write(incomingByte);
        break;
      case SERIAL_COMMAND_VOICE_LIGHTSOFF2:
        digitalWrite(PIN_LIGHTS_GROUP2, g_light_two_state = LOW);
        Serial.write(incomingByte);
        break;
    }

/*
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);

*/
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if ( (packetSize >= 8) )
  {
    packetSize = Udp.read(g_ultraSoundValues, 8);
    Serial.print("Contents: ");
    Serial.println(packetSize);
  }
  Serial.flush();
  delay(10);
}
