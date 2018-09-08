#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#ifdef WIN32
#  include <winsock.h>
#  include <WS2tcpip.h>
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

   /* Create the socket. */
   if ( (rcp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
       perror ("socket failed");
       exit (EXIT_FAILURE);
   }

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

   context->sin_family       = AF_INET;
   context->sin_port         = htons(destPort);
   context->sin_addr.s_addr  = htonl(destAddr);

   addr_size = sizeof(*context);
   nbytes = sendto (socket, payload, nBytes, 0, (struct sockaddr *)context, addr_size);
   if (nbytes < 0){
      perror ("sendto (server)");
      exit (EXIT_FAILURE);
   }
   fflush(NULL);
}

int receiveSocket (int socket, size_t *nBytes, unsigned char *payload, struct sockaddr_in *context){
   int size      = 0, i = 0;
   size_t nbytes = 0;

   /* Wait for a datagram. */
   size = sizeof (*context);
   nbytes = recvfrom (socket, payload, *nBytes, 0, (struct sockaddr *)context, &size);
   if (nbytes < 0){
      perror ("recfrom (server)");
      exit (EXIT_FAILURE);
   }
   /* Give a diagnostic message. */
   printf("Server: got message: %zu\n", nbytes);
   if ( nbytes ){
      for ( i = 0; i < nbytes; i++ ){
         printf("%02X", payload[i] & 0xFF);
      }
   }
   printf("\n");
   fflush(NULL);
}
