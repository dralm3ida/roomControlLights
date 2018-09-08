#include <Ethernet.h>
#include <EthernetUdp.h>
#include "commands.h"

#define DEBUG         Serial.print
#define DEBUGLN       Serial.println

#define PIN_LIGHTS_GROUP1   2
#define PIN_LIGHTS_GROUP2   3

#define ULTRASOUND_DELTA_THRESHOLD 128

unsigned char g_ultraSoundValues[] = {0, 0, 0, 0, 0, 0, 0, 0}; // 8 sensors, each one with range 0-256
unsigned char g_ultraSoundLastValues[] = {100, 20, 30, 48, 50, 68, 170, 89}; // last values
unsigned char g_lightSensor = 1;
unsigned char g_pirSensor = 1;
unsigned char g_light_one_state = LOW;
unsigned char g_light_two_state = LOW;
byte g_lights_mode = SERIAL_COMMAND_VOICE_AUTOMODEOFF;
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

void turnLightsOn (int group){
   switch (group){
      case 1:
         digitalWrite(PIN_LIGHTS_GROUP1, g_light_one_state = HIGH);
         break;
      case 2:
         digitalWrite(PIN_LIGHTS_GROUP2, g_light_two_state = HIGH);      
         break;
      default:
         digitalWrite(PIN_LIGHTS_GROUP1, g_light_one_state = HIGH);
         digitalWrite(PIN_LIGHTS_GROUP2, g_light_two_state = HIGH);
   }
}

void turnLightsOff (int group)
{
   switch (group){
      case 1:
         digitalWrite(PIN_LIGHTS_GROUP1, g_light_one_state = LOW);
         break;
      case 2:
         digitalWrite(PIN_LIGHTS_GROUP2, g_light_two_state = LOW);
         break;
      default:
         digitalWrite(PIN_LIGHTS_GROUP1, g_light_one_state = LOW);
         digitalWrite(PIN_LIGHTS_GROUP2, g_light_two_state = LOW);
   }
}

int handleLightStatus()
{
   if ( SERIAL_COMMAND_VOICE_AUTOMODEOFF == g_lights_mode ){ return 0; }

   int i = 0, leni = 0;
   int sensorValue = 0;
   int deltaValue = 0;



   for (i = 0, leni = 8; i < leni ; ++i){
      sensorValue = g_ultraSoundValues[i];
      deltaValue =  g_ultraSoundValues[i] - g_ultraSoundLastValues[i];
      if ( deltaValue >= ULTRASOUND_DELTA_THRESHOLD ){
         turnLightsOn(0);
      }
   }
   return 0;
}

void doCommandVoice (int command)
{
  switch ( command )
  {
    case SERIAL_COMMAND_VOICE_LIGHTSON:
      turnLightsOn(0);
      DEBUGLN("{}VOICE: command lights on");
      break;
    case SERIAL_COMMAND_VOICE_LIGHTSOFF:
      turnLightsOff(0);
      DEBUGLN("{}VOICE: command lights off");
      break;
    case SERIAL_COMMAND_VOICE_LIGHTSON1:
      turnLightsOn(1);
      DEBUGLN("{}VOICE: command lights on group 1");
      break;
    case SERIAL_COMMAND_VOICE_LIGHTSOFF1:
      turnLightsOff(1);
      DEBUGLN("{}VOICE: command lights off group 1");
      break;
    case SERIAL_COMMAND_VOICE_LIGHTSON2:
      turnLightsOn(2);
      DEBUGLN("{}VOICE: command lights on group 2");
      break;
    case SERIAL_COMMAND_VOICE_LIGHTSOFF2:
      turnLightsOff(2);
      DEBUGLN("{}VOICE: command lights off group 2");
      break;
  }  
}


void loop() {
  int packetSize = Udp.parsePacket();
  int incomingByte = 0;

  /*Serial.print("serial ");
  Serial.print(Serial.available());
  Serial.print(", ultrasound ");
  Serial.print(packetSize);
  Serial.println("");
  */
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
  else if ( packetSize > 0 )
  {
    byte  response[20] = {0};
    int   i = 0, none = 1;
    
    packetSize = Udp.read(response, 20);
    switch ( response[0] )
    {
      case 'A':
        g_lights_mode = SERIAL_COMMAND_VOICE_AUTOMODEON;
        DEBUGLN("{}AUTO: enable automatic mode");
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, 10);
        Udp.endPacket();        
        break;
      case 'M':
        g_lights_mode = SERIAL_COMMAND_VOICE_AUTOMODEOFF;
        DEBUGLN("{}AUTO: disable automatic mode");
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, 10);
        Udp.endPacket();
        break;
      case 'S':
        Serial.print("Update sensors: ");
        if ( packetSize >= 7 )
        {
          memcpy(g_ultraSoundValues, response + 1, 5);
          for ( i = 1; i < 7; ++i )
          {
            Serial.print(response[i], DEC);
            Serial.print(" ");
          }
        }
        if ( packetSize > 7 )
        {
          g_lightSensor = response[9];
          Serial.print(g_lightSensor, DEC);
        }
        Serial.print("\n");
        handleLightStatus();
        none = 0;
        break;
      case 'V':
        doCommandVoice(response[1]);
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, 10);
        Udp.endPacket();
        break;
      case 'G':
        IPAddress remoteAddr = Udp.remoteIP();
        int       remotePort = Udp.remotePort();

        DEBUG("{}GET: received request from ");
        DEBUG(Udp.remoteIP());
        DEBUG(":");
        DEBUG(Udp.remotePort());
        response[0] = 10;
        memcpy(response + 1, g_ultraSoundValues, 8);
        response[9] = g_lightSensor;
        response[10] = g_lights_mode;
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, 11);
        Udp.endPacket();
        DEBUGLN("... return 11 bytes");
        none = 0;
        break;
    }
  }
  Serial.flush();
  delay(10);
}
