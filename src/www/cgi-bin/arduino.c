#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "../../arduino/serial-comm/commands.h"

#define SERIAL 0
#include "rpc.c"

#if SERIAL
#  define COMM_OPEN  serial_open
#  define COMM_WRITE serial_write
#  define COMM_READ  serial_read
#  define COMM_CLOSE serial_close
#  define COMM_RSRC  HANDLE
#else
#  define COMM_OPEN  rpc_open
#  define COMM_WRITE rpc_write
#  define COMM_READ  rpc_read
#  define COMM_CLOSE rpc_close
typedef struct
{
   int                  socket;
   struct sockaddr_in   name;
} COMM_RSRC;
#endif


#if SERIAL

COMM_RSRC* serial_open (COMM_RSRC * serial)
{
   LPCSTR         portname = "COM4";
   DWORD          accessdirection = GENERIC_READ | GENERIC_WRITE;
   DCB            dcbSerialParams = {0};
   COMMTIMEOUTS   timeouts = {0};
   HANDLE hSerial = CreateFile(portname
      , accessdirection
      , 0
      , 0
      , OPEN_EXISTING
      , 0
      , 0);
   if ( INVALID_HANDLE_VALUE == hSerial )
   {
      printf("failed opening %s\n", portname);
      return NULL;
   }

   dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
   if ( !GetCommState(hSerial, &dcbSerialParams) )
   {
      printf("could not get the state of the comport %s\n", portname);
      return NULL;
   }
   dcbSerialParams.BaudRate = 9600;
   dcbSerialParams.ByteSize = 8;
   dcbSerialParams.StopBits = ONESTOPBIT;
   dcbSerialParams.Parity = NOPARITY;
   if ( !SetCommState(hSerial, &dcbSerialParams) )
   {
      printf("analyse error %s\n", portname);
      return NULL;
   }

   timeouts.ReadIntervalTimeout=50;
   timeouts.ReadTotalTimeoutConstant=50;
   timeouts.ReadTotalTimeoutMultiplier=10;
   timeouts.WriteTotalTimeoutConstant=50;
   timeouts.WriteTotalTimeoutMultiplier=10;
   if ( !SetCommTimeouts(hSerial, &timeouts) )
   {
      printf("handle error %s\n", portname);
      return NULL;
   }
   *serial = &hSerial;
   return serial;
}// endof ::serial_open

ssize_t serial_write (COMM_RSRC * serial, size_t bytes, unsigned char * payload)
{
   DWORD    dwBytesRead = 0;

   if ( !WriteFile(*serial, payload, bytes, &dwBytesRead, NULL) )
   {
      return -1;
   }
   return dwBytesRead;
}// endof serial_open

ssize_t serial_read (COMM_RSRC * serial, size_t * bytes, unsigned char * payload)
{
   DWORD    dwBytesRead = 0;

   if ( !ReadFile(*serial, payload, *bytes, &dwBytesRead, NULL) )
   {
      return -1;
   }
   return dwBytesRead;
}// endof serial_read

void serial_close (COMM_RSRC * serial)
{
   CloseHandle(*serial);
}// endof serial_close

#else

COMM_RSRC* rpc_open (COMM_RSRC * socket)
{
   memset(socket, 0, sizeof(*socket));
   socket->socket = createSocket(0);
   return socket;
}// endof rpc_open

ssize_t rpc_write (COMM_RSRC * socket, size_t bytes, unsigned char * payload)
{
   if ( requestSocket(socket->socket, (192 << 24 | 168 << 16 | 2 << 8 | 254), 8888, bytes, payload, &socket->name) < 0 )
   {
      return -1;
   }
   return bytes;
}// endof ::rpc_write

ssize_t rpc_read (COMM_RSRC * socket, size_t * bytes, unsigned char * payload)
{
   if ( receiveSocket(socket->socket, bytes, payload, &socket->name) < 0 )
   {
      return -1;
   }
   return *bytes;
}// endof ::rpc_read

void rpc_close (COMM_RSRC * socket)
{
   closeSocket(socket->socket);
   return;
}// endof rpc_close
#endif

int main (int argc, char ** argv)
{
   char        command[50] = "all";
   char        *pathinfo = NULL;
   COMM_RSRC   rpcHandle;
   int         res = 0;

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

   if ( !COMM_OPEN(&rpcHandle) )
   {
      printf("Failed creating comm resource\n");
      res = -1;
      return 1;
   }

   if ( 0 == res )
   {
      unsigned char  request[2] = {0}, response[50];
      char     isVoiceCommand = 0;
      char     ultrasound[8] = {0};
      char     temperature = 0;
      size_t   dwBytesRead = 0;

      if ( 0 ){
      }else if ( 0 == strncmp("voice:", command, 6) ){
         request[0] = 'V';
         if ( 0 ){
         }else if ( 0 == strcmp("wakeup", command + 6) ){
            request[0] = 'A';
         }else if ( 0 == strcmp("shutdown", command + 6) ){
            request[0] = 'M';
         }else if ( 0 == strcmp("lightson", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSON;
         }else if ( 0 == strcmp("lightsoff", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSOFF;
         }else if ( 0 == strcmp("lightsongroup1", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSON1;
         }else if ( 0 == strcmp("lightsoffgroup1", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSOFF1;
         }else if ( 0 == strcmp("lightsofgroup1", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSOFF1;
         }else if ( 0 == strcmp("lightsongroup2", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSON2;
         }else if ( 0 == strcmp("lightsoffgroup2", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSOFF2;
         }else if ( 0 == strcmp("lightsofgroup2", command + 6) ){
            request[1] = SERIAL_COMMAND_VOICE_LIGHTSOFF2;
         }else{
            res = -1;
         }
         isVoiceCommand = 1;
      }else if ( 0 == strcmp("ultrasound", command) ){
         request[0] = SERIAL_COMMAND_ULTRASOUND;
      }else if ( 0 == strcmp("light", command) ){
         request[0] = SERIAL_COMMAND_LIGHT;
      }else if ( 0 == strcmp("pir", command) ){
         request[0] = SERIAL_COMMAND_PIR;
      }else{
         request[0] = 'G';
      }

      //printf("Send command[%u]\n", request);
      if ( COMM_WRITE(&rpcHandle, 2, request) < 0 )
      {
         printf("Failed sending comm request\n");
         res = -1;
         return 1;
      }

      Sleep(100);
      dwBytesRead = sizeof(response);
      if ( COMM_READ(&rpcHandle, &dwBytesRead, response) < 0 )
      {
         printf("Failed receiving comm response\n");
         res = -1;
         return 1;
      }
      if ( (0 == res) && (0 == isVoiceCommand) )
      {
         int   i = 0, j = 0, k = 1;

         printf("{ \"command\": \"%s\", \"bytes\": %u, \"request\": \"%c\"", command, dwBytesRead, request);
         if ( (1 == request[0]) || ('G' == request[0]) )
         {
            printf("\n, \"ultrasound\": [");
            for ( j = 0; (i < 8) && (i < dwBytesRead); ++i, ++j )
            {
               printf("%c%u", (j)?(','):(' '), response[k++]);
            }
            printf("]");
         }
         if ( (2 == request[0]) || ('G' == request[0]) )
         {
            printf("\n, \"light\": %u", response[k++]);
         }
         if ( (3 == request[0]) || ('G' == request[0]) )
         {
            printf("\n, \"pir\": %u", response[k++]);
         }
         printf("\n}");
      }
      if ( (0 == res) && (1 == isVoiceCommand) )
      {
         printf("{ \"command\": \"%s\", \"bytes\": %u, \"request\": \"%c\", \"response\": %u", command, dwBytesRead, request, response[0]);
         printf("\n}");
      }

   }

   COMM_CLOSE(&rpcHandle);
}
