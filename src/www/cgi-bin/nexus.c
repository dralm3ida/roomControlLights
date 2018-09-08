#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#  include <winsock.h>
//#  include <WS2tcpip.h>
#  include <windows.h>
#else
#  include <sys/socket.h>
#  include <sys/un.h>
#endif
	 
#define SERVER  "/tmp/serversocket"
#define MAXMSG  512

int
make_named_socket (const char *filename, int port)
{
  struct sockaddr_in name;
  int sock;
  size_t size;

  /* Create the socket. */
  sock = socket (AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
    {
      perror ("socket");
      exit (EXIT_FAILURE);
    }

  /* Bind a name to the socket. */
  name.sin_family = AF_INET;
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

  if (bind (sock, (struct sockaddr *) &name, sizeof(name)) < 0)
    {
      perror ("bind");
      printf("%d\n", WSAGetLastError());
      exit (EXIT_FAILURE);
    }

  return sock;
}

int main (void)
{
   int sock;
   char message[MAXMSG];
   struct sockaddr_in name;
   int size;
   int nbytes;
   WORD wVersionRequested;
    WSADATA wsaData; 

   wVersionRequested = MAKEWORD(2, 2);
   WSAStartup(wVersionRequested, &wsaData);
   /* Remove the filename first, it's ok if the call fails */
   //unlink (SERVER);

   /* Make the socket, then loop endlessly. */
   sock = make_named_socket (SERVER, 8888);
   while (1)
   {
      /* Wait for a datagram. */
      size = sizeof (name);
      nbytes = recvfrom (sock, message, MAXMSG, 0, (struct sockaddr *)&name, &size);
      if (nbytes < 0)
      {
         perror ("recfrom (server)");
         exit (EXIT_FAILURE);
      } 
      /* Give a diagnostic message. */
      fprintf (stderr, "Server: got message: %s\n", message);
      /* Bounce the message back to the sender. */
      nbytes = sendto (sock, message, nbytes, 0, (struct sockaddr *) & name, size);
      if (nbytes < 0)
      {
         perror ("sendto (server)");
         exit (EXIT_FAILURE);
      }
   }
   WSACleanup();
}
