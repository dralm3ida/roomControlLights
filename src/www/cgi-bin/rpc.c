#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#ifdef WIN32
#  include <winsock.h>
//#  include <WS2tcpip.h>
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <sys/un.h>
#  include <netinet/in.h>
#endif

int createSocket (int port){
   int rcp_socket = 0;
   struct sockaddr_in name;
#ifdef WIN32
   WORD wVersionRequested;
    WSADATA wsaData;
    DWORD tv = 0;
#else
   struct timeval    tv = {0};
#endif

#ifdef WIN32
   wVersionRequested = MAKEWORD(2, 2);
   WSAStartup(wVersionRequested, &wsaData);
   /* Remove the filename first, it's ok if the call fails */
   //unlink (SERVER);
#endif


   /* Create the socket. */
   if ( (rcp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
       perror ("socket failed");
       return -1;
   }

   //[#3.4] Set the timeout tick (1s) on this socket when waiting for response

   //[#3.6] Sets timeout period
#ifdef WIN32
   tv = 2000;
   setsockopt(rcp_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#else
   tv.tv_sec  = 2;
   tv.tv_usec = 0;
   setsockopt(rcp_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

   if ( 0 != port ){
      /* Bind a name to the socket. */
      name.sin_family       = AF_INET;
      name.sin_port         = htons(port);
      name.sin_addr.s_addr  = htonl(INADDR_ANY);

      //strncpy (name.sin_path, filename, sizeof (name.sin_path));
      //name.sin_path[sizeof (name.sin_path) - 1] = '\0';

      /* The size of the address is
        the offset of the start of the filename,
        plus its length (not including the terminating null byte).
        Alternatively you can just do:
        size = SUN_LEN (&name);
        */
     //size = (offsetof (struct sockaddr_in, sin_path) + strlen (name.sin_path));

     if (bind (rcp_socket, (struct sockaddr *) &name, sizeof(name)) < 0)
     {
        perror ("bind");
        #ifdef WIN32
           printf("%d\n", WSAGetLastError());
        #endif
           exit (EXIT_FAILURE);
     }
   }
   return rcp_socket;
}

int requestSocket (int socket, int destAddr, int destPort, size_t nBytes, unsigned char *payload, struct sockaddr_in *context){
   int addr_size = 0;
   size_t nbytes = 0;

   memset(context, 0, sizeof(*context));
   context->sin_family       = AF_INET;
   context->sin_port         = htons(destPort);
   context->sin_addr.s_addr  = htonl(destAddr);

   addr_size = sizeof(*context);
   nbytes = sendto (socket, payload, nBytes, 0, (struct sockaddr *)context, addr_size);
   if (nbytes < 0){
      perror ("sendto (server)");
      return -1;
   }
   //printf("send data %u\n", nbytes);
   //fflush(NULL);
   return nBytes;
}

int receiveSocket (int socket, size_t *nBytes, unsigned char *payload, struct sockaddr_in *context){
   int size      = 0, i = 0;
   size_t nbytes = 0;
   int tmout = 0;

   /* Wait for a datagram. */
   //printf("receive data %u\n", *nBytes);
   //fflush(NULL);
   size = sizeof (*context);
   do
   {
      nbytes = recvfrom (socket, payload, *nBytes, 0, (struct sockaddr *)context, &size);
      tmout++;
      //printf("testa %d\n", nbytes);
      //fflush(NULL);
   } while ( ((nbytes < 0) || (nbytes >= *nBytes)) && (tmout < 2) );
   /* Give a diagnostic message. */
   //printf("Server: got message: %u\n", nbytes);
   if ( nbytes ){
      for ( i = 0; i < nbytes; i++ ){
         //printf("%02X", payload[i] & 0xFF);
      }
      *nBytes = nbytes;
   }
}

void closeSocket (int socket)
{
#ifdef WIN32
   WSACleanup();
#endif
}
