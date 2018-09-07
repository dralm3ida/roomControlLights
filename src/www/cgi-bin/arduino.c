#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../../arduino/serial-comm/commands.h"

LPCSTR portname = "COM4";
DWORD  accessdirection =GENERIC_READ | GENERIC_WRITE;

int main (int argc, char ** argv)
{
   char  command[50] = "all";
   char  *pathinfo = NULL;
   int   res = 0;

   printf("Content-type: application/json\n");
   printf("Cache: no-cache\n");
   printf("\n");


   if ( NULL != (pathinfo = getenv("PATH_INFO")) )
   {
      char  *s = pathinfo, *r = NULL;

      if ( '/' == *s ){ s++; }
      r = s + 1;
      while ( ('/' != *r) && (0 != *r) ){ r++; }
      if ( *r )
      {
         s = r; r++;
         while ( ('/' != *r) && (0 != *r) ){ r++; }
         if ( (r - s) > 1 ){ memcpy(command, s + 1, r - s); }
      }
   }


HANDLE hSerial = CreateFile(portname,
    accessdirection,
    0,
    0,
    OPEN_EXISTING,
    0,
    0);
if (hSerial == INVALID_HANDLE_VALUE) {
   printf("failed opening %s\n", portname);
}

DCB dcbSerialParams = {0};
dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
if (!GetCommState(hSerial, &dcbSerialParams)) {
   printf("could not get the state of the comport %s\n", portname);
}
dcbSerialParams.BaudRate=9600;
dcbSerialParams.ByteSize=8;
dcbSerialParams.StopBits=ONESTOPBIT;
dcbSerialParams.Parity=NOPARITY;
if(!SetCommState(hSerial, &dcbSerialParams)){
   printf("analyse error %s\n", portname);
     //analyse error
}


 COMMTIMEOUTS timeouts={0};
timeouts.ReadIntervalTimeout=50;
timeouts.ReadTotalTimeoutConstant=50;
timeouts.ReadTotalTimeoutMultiplier=10;
timeouts.WriteTotalTimeoutConstant=50;
timeouts.WriteTotalTimeoutMultiplier=10;
if(!SetCommTimeouts(hSerial, &timeouts)){
   printf("handle error %s\n", portname);
    //handle error
}

   if ( 0 == res )
   {
      char  request = 0, response[50];
      char  isVoiceCommand = 0;
      char  ultrasound[8] = {0};
      char  temperature = 0;
      DWORD dwBytesRead = 0;

      if ( 0 ){
      }else if ( 0 == strncmp("voice:", command, 6) ){
         if ( 0 ){
         }else if ( 0 == strcmp("lightson", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSON;
         }else if ( 0 == strcmp("lightsoff", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSOFF;
         }else if ( 0 == strcmp("lightsongroup1", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSON1;
         }else if ( 0 == strcmp("lightsoffgroup1", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSOFF1;
         }else if ( 0 == strcmp("lightsofgroup1", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSOFF1;
         }else if ( 0 == strcmp("lightsongroup2", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSON2;
         }else if ( 0 == strcmp("lightsoffgroup2", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSOFF2;
         }else if ( 0 == strcmp("lightsofgroup2", command + 6) ){
            request = SERIAL_COMMAND_VOICE_LIGHTSOFF2;
         }else{
            res = -1;
         }
         isVoiceCommand = 1;
      }else if ( 0 == strcmp("ultrasound", command) ){
         request = SERIAL_COMMAND_ULTRASOUND;
      }else if ( 0 == strcmp("light", command) ){
         request = SERIAL_COMMAND_LIGHT;
      }else if ( 0 == strcmp("pir", command) ){
         request = SERIAL_COMMAND_PIR;
      }

      if ( !WriteFile(hSerial, &request, 1, &dwBytesRead, NULL) )
      {
         res = -1;
         printf("write error %s\n", portname);
      }
      else
      {
         dwBytesRead = 0;
         if ( !ReadFile(hSerial, response, 50, &dwBytesRead, NULL) )
         {
            res = -1;
            printf("read error %s\n", portname);
         }
      }
      if ( (0 == res) && (0 == isVoiceCommand) )
      {
         int   i = 0, j = 0;

         printf("{ \"serial\": \"%s\", \"command\": \"%s\", \"bytes\": %u, \"request\": %u", portname, command, dwBytesRead, request);
         if ( (1 == request) || (0 == request) )
         {
            printf("\n, \"ultrasound\": [");
            for ( j = 0; (i < 8) && (i < dwBytesRead); ++i, ++j )
            {
               printf("%c%u", (j)?(','):(' '), response[i]);
            }
            printf("]");
         }
         if ( (2 == request) || (0 == request) )
         {
            printf("\n, \"light\": %u", response[i++]);
         }
         if ( (3 == request) || (0 == request) )
         {
            printf("\n, \"pir\": %u", response[i++]);
         }
         printf("\n}");
      }
      if ( (0 == res) && (1 == isVoiceCommand) )
      {
         printf("{ \"serial\": \"%s\", \"command\": \"%s\", \"bytes\": %u, \"request\": %u, \"response\": %u", portname, command, dwBytesRead, request, response[0]);
         printf("\n}");
      }

   }



#if 0
   if ( NULL != (mycom = fopen("\\\\.\\COM4", "r+b")) )
   {
      request[0] = 'a';
      fwrite(request, sizeof(char), 1, mycom);


      fflush(NULL);
      fclose(mycom);
   }
   else
   {
      printf("failed opening COM4\n");
   }
#endif
   //printf("hello world %s\n", getenv("PATH_INFO"));
   CloseHandle(hSerial);
}
