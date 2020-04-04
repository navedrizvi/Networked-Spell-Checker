#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "main.h"

int main(int argc, char *argv[])
{
  int socket_fd;
  struct sockaddr_in servaddr;
  char *host;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s host\n", argv[0]);
    exit(1);
  }
  host = argv[1];
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1)
  {
    printf("Socket creation failed...\n");
    exit(0);
  }
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(host);
  servaddr.sin_port = htons(DEFAULT_PORT);

  //Call connect to establish connection with server (servaddr)
  if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
  {
    printf("Connection with the server failed...\n");
    exit(0);
  }
  else
    printf("Connected to server..\n");

  char message[MAX_LINE];
  read(socket_fd, message, sizeof(message));
  printf("%s", message);
  handle_client_request_response(socket_fd);
  close(socket_fd);
}

void handle_client_request_response(int socket_fd)
{
  char buff[MAX_LINE];
  int n;
  while (1)
  {
    //Get client input
    bzero(buff, sizeof(buff));
    printf("Enter word to spell check: ");
    n = 0;
    while ((buff[n++] = getchar()) != '\n')
      ;
    //Send client input
    write(socket_fd, buff, sizeof(buff));
    if ((int)buff[0] == 27)
    {
      printf("Client exiting...\n");
      break;
    }

    bzero(buff, sizeof(buff));

    //Read response from server
    read(socket_fd, buff, sizeof(buff));
    printf("Resposne form server : %s", buff);
  }
}
