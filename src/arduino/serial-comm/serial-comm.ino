#include <Ethernet.h>
#include <EthernetUdp.h>
//#define PX4
#ifdef PX4
#include <Wire.h>
#include <PX4Flow.h>
#endif
#include "commands.h"

#define DEBUG         Serial.print
#define DEBUGLN       Serial.println

#define PIN_LIGHTS_GROUP1   2
#define PIN_LIGHTS_GROUP2   3
#define SCALE_FACTOR 1.5
#define MAX_OUTPUT 255
#define MIN_OUTPUT 0
#define ITERATION 10

#define ULTRASOUND_DELTA_THRESHOLD 100

unsigned char g_ultraSoundValues[] = {0, 0, 0, 0, 0, 0, 0, 0}; // 8 sensors, each one with range 0-256
unsigned char g_ultraSoundLastValues[] = {0, 0, 0, 0, 0, 0, 0, 0}; // last values
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

int iterations = 0;
int motion_intensity_tmp_x = 0;
int motion_intensity_tmp_y = 0;
int motion_intensity = 0;
int number_of_sums = 0;
int flow_x_sum = 0;
int flow_y_sum = 0;
#ifdef PX4
PX4Flow sensor = PX4Flow(); 
#endif

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

#ifdef PX4
  // Initialize the I2C bus
  Wire.begin();
#endif  
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
   int lightsOnOff = 0;

   for (i = 0, leni = 8; i < leni ; ++i){
      sensorValue = g_ultraSoundValues[i];
      deltaValue =  abs(g_ultraSoundValues[i] - g_ultraSoundLastValues[i]);
      if ( (deltaValue >= ULTRASOUND_DELTA_THRESHOLD) && (i != 3) ){
         Serial.print("{}TRIGGER ON sensor ");
         Serial.print(i);
         Serial.print(" delta ");
         Serial.println(deltaValue);
         lightsOnOff = 1;
         // Updates last value
      }
      g_ultraSoundLastValues[i] = g_ultraSoundValues[i];
   }

   if ( 1 == lightsOnOff ){
     turnLightsOn(0);
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
      case 'A': // Modo automatico
        g_lights_mode = SERIAL_COMMAND_VOICE_AUTOMODEON;
        DEBUGLN("{}AUTO: enable automatic mode");
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, 10);
        Udp.endPacket();
        break;
      case 'M': // Modo manual
        g_lights_mode = SERIAL_COMMAND_VOICE_AUTOMODEOFF;
        DEBUGLN("{}AUTO: disable automatic mode");
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, 10);
        Udp.endPacket();
        break;
      case 'S': // Informacao que vem dos ultrasons
        Serial.print("Update sensors:");
        if ( packetSize >= 7 )
        {
          memcpy(g_ultraSoundValues, response + 1, 5);
          Serial.print(" S0:");
          Serial.print(response[1], DEC);
          Serial.print(" S1:");
          Serial.print(response[2], DEC);
          Serial.print(" S2:");
          Serial.print(response[4], DEC);
          Serial.print(" S3:");
          Serial.print(response[3], DEC);
          Serial.print(" S4:");
          Serial.print(response[5], DEC);
          Serial.print(" SL:");
          Serial.print(response[6], DEC);
          /*for ( i = 1; i < 7; ++i )
          {
            Serial.print(response[i], DEC);
            Serial.print(" ");
          }*/
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
      case 'P': // Informacao que vem do optical flow
          Serial.print("Update PIR: ");
          g_pirSensor = (unsigned char)response[1];

          if ( ((HIGH == g_light_one_state) || (HIGH == g_light_two_state)) && (0 == g_pirSensor) ) {
             turnLightsOff(0);
          }
        break;
      case 'V': // Comandos de voz
        doCommandVoice(response[1]);
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write(response, 10);
        Udp.endPacket();
        break;
      case 'G': // Ler todos os valores
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
#ifdef PX4
  if ( 0 )
  {
  sensor.update();
  flow_x_sum = sensor.pixel_flow_x_sum();
  flow_y_sum = sensor.pixel_flow_y_sum();
  DEBUG("{}OPT: read ");
  DEBUG(flow_x_sum);
  DEBUG(" ");
  DEBUG(flow_y_sum);
  DEBUG(" -> motion ");
  if ( (iterations % ITERATION) == 0) 
  {
     motion_intensity += ((motion_intensity_tmp_x + motion_intensity_tmp_y))/(ITERATION);
    if(motion_intensity < MIN_OUTPUT)
    {
      motion_intensity = MIN_OUTPUT;
    }
    else if(motion_intensity > MAX_OUTPUT)
    {
      motion_intensity = MAX_OUTPUT;
    }
   
    /*
     * Reset to avarage varaibles
     */
    motion_intensity_tmp_x = 0;
    motion_intensity_tmp_y = 0;
    number_of_sums = 0;
    iterations = 0;

    //Update 
    g_pirSensor = (motion_intensity > 50)?(1):(0);
    motion_intensity = 0;
    DEBUGLN(motion_intensity);
  }
  else
  {
    /*
     * Sum to avarage
     */
    motion_intensity_tmp_x = motion_intensity_tmp_x + (abs(flow_x_sum)*SCALE_FACTOR); 
    motion_intensity_tmp_y = motion_intensity_tmp_y + (abs(flow_y_sum)*SCALE_FACTOR);
  }
  }
#endif  
  Serial.flush();
  delay(10);
}
