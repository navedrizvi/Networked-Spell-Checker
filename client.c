#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utility.c"
#include "main.h"

int main(int argc, char *argv[])
{
  int socket_fd;
  struct sockaddr_in servaddr;
  char *host;
  int port_number;

  if (argc < 2)
  {
    fprintf(stderr, "usage: %s host [server_port_n]\n", argv[0]);
    exit(1);
  }
  host = argv[1];

  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    unix_error("Socket Creation Failed");
  }
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(host);

  //Set port for server if specified
  if (argv[2] != NULL)
  {
    port_number = atoi(argv[2]);
  }
  else
  {
    port_number = DEFAULT_PORT;
  }

  servaddr.sin_port = htons(port_number);

  //Call connect to establish connection with server (servaddr)
  if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
  {
    unix_error("Connection with the server failed..");
  }
  printf("Connected to server..\n");
  printf("Response from server:\n");

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
    printf("Response form server : %s", buff);
  }
}
// #include <stdio.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <string.h>
// #include <arpa/inet.h>
// #include <stdlib.h>
// #include <fcntl.h>  // for open
// #include <unistd.h> // for close
// #include <pthread.h>
// void *cientThread(void *arg)
// {
//   printf("In thread\n");
//   char message[1000];
//   char buffer[1024];
//   int clientSocket;
//   struct sockaddr_in serverAddr;
//   socklen_t addr_size;
//   // Create the socket.
//   clientSocket = socket(PF_INET, SOCK_STREAM, 0);
//   //Configure settings of the server address
//   // Address family is Internet
//   serverAddr.sin_family = AF_INET;
//   //Set port number, using htons function
//   serverAddr.sin_port = htons(8080);
//   //Set IP address to localhost
//   serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
//   memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
//   //Connect the socket to the server using the address
//   addr_size = sizeof serverAddr;
//   connect(clientSocket, (struct sockaddr *)&serverAddr, addr_size);
//   strcpy(message, "Hello");
//   if (send(clientSocket, message, strlen(message), 0) < 0)
//   {
//     printf("Send failed\n");
//   }
//   //Read the message from the server into the buffer
//   if (recv(clientSocket, buffer, 1024, 0) < 0)
//   {
//     printf("Receive failed\n");
//   }
//   //Print the received message
//   printf("Data received: %s\n", buffer);
//   close(clientSocket);
//   pthread_exit(NULL);
// }
// int main()
// {
//   int i = 0;
//   pthread_t tid[51];
//   while (i < 50)
//   {
//     if (pthread_create(&tid[i], NULL, cientThread, NULL) != 0)
//       printf("Failed to create thread\n");
//     i++;
//   }
//   sleep(20);
//   i = 0;
//   while (i < 50)
//   {
//     pthread_join(tid[i++], NULL);
//     printf("%d:\n", i);
//   }
//   return 0;
// }