// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <errno.h>
// #include <netdb.h>
// #include <sys/types.h>
// #include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>

// #define PORT "80"
// #define EXIT_GETADDRINFO_FAILURE 1
// #define EXIT_CONNECT_FAILURE 2

// #define MAX_LINE 128
// #define CONNECT_STR_MAX 256

// ssize_t readLine(int fd, void *buffer, size_t n);
// int getclientsd(char *, char *);

// int main(int argc, char *argv[])
// {
//   size_t bytes_read;
//   int sd; /* socket descriptor */
//   char line[MAX_LINE];
//   int status;
//   char *host;
//   char connect_str[CONNECT_STR_MAX];

//   if (argc != 2)
//   {
//     fprintf(stderr, "usage: %s host\n", argv[0]);
//     exit(1);
//   }

//   host = argv[1];
//   sd = getclientsd(host, PORT);
//   //Make get http request
//   sprintf(connect_str, "GET / HTTP/1.1\r\n\r\n");
//   write(sd, connect_str, strlen(connect_str));
//   memset(line, 0, MAX_LINE);
//   while ((readLine(sd, line, MAX_LINE - 1)) > 0)
//   {
//     printf("%s", line);
//   }

//   close(sd);
//   return 0;
// }

// int getclientsd(char *host, char *port)
// {
//   int status;
//   int sd; /* socket descriptor */
//   struct addrinfo hints, *p, *servinfo;
//   memset(&hints, 0, sizeof hints);
//   hints.ai_family = AF_INET;
//   hints.ai_socktype = SOCK_STREAM;

//   if ((status = getaddrinfo(host, port, &hints, &servinfo)) != 0)
//   {
//     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
//     return EXIT_GETADDRINFO_FAILURE;
//   }

//   for (p = servinfo; p != NULL; p = p->ai_next)
//   {
//     if ((sd = socket(p->ai_family, p->ai_socktype,
//                      p->ai_protocol)) == -1)
//     {
//       continue;
//     }

//     //Call connect to establish connection with server at socket p->ai_addr (which is listening), with sd being client's socket descriptor
//     //Returns 0 on successful connection
//     if (connect(sd, p->ai_addr, p->ai_addrlen) == -1)
//     {
//       close(sd);
//       continue;
//     }
//     break;
//   }

//   if (!p)
//   {
//     fprintf(stderr, "failed to connect to %s\n", host);
//     exit(EXIT_CONNECT_FAILURE);
//   }

//   freeaddrinfo(servinfo);
//   return sd;
// }

// /* FROM KERRISK

//    Read characters from 'fd' until a newline is encountered. If a newline
//    character is not encountered in the first (n - 1) bytes, then the excess
//    characters are discarded. The returned string placed in 'buf' is
//    null-terminated and includes the newline character if it was read in the
//    first (n - 1) bytes. The function return value is the number of bytes
//    placed in buffer (which includes the newline character if encountered,
//    but excludes the terminating null byte). */
// ssize_t readLine(int fd, void *buffer, size_t n)
// {
//   ssize_t numRead; /* # of bytes fetched by last read() */
//   size_t totRead;  /* Total bytes read so far */
//   char *buf;
//   char ch;

//   if (n <= 0 || buffer == NULL)
//   {
//     errno = EINVAL;
//     return -1;
//   }

//   buf = buffer; /* No pointer arithmetic on "void *" */

//   totRead = 0;
//   for (;;)
//   {
//     numRead = read(fd, &ch, 1);

//     if (numRead == -1)
//     {
//       if (errno == EINTR) /* Interrupted --> restart read() */
//         continue;
//       else
//         return -1; /* Some other error */
//     }
//     else if (numRead == 0)
//     {                   /* EOF */
//       if (totRead == 0) /* No bytes read; return 0 */
//         return 0;
//       else /* Some bytes read; add '\0' */
//         break;
//     }
//     else
//     { /* 'numRead' must be 1 if we get here */
//       if (totRead < n - 1)
//       { /* Discard > (n - 1) bytes */
//         totRead++;
//         *buf++ = ch;
//       }

//       if (ch == '\n')
//         break;
//     }
//   }

//   *buf = '\0';
//   return totRead;
// }
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80
#define PORT 22222
#define SA struct sockaddr

void func(int sockfd)
{
  char buff[MAX];
  int n;
  for (;;)
  {
    bzero(buff, sizeof(buff));
    printf("Enter the string : ");
    n = 0;
    while ((buff[n++] = getchar()) != '\n')
      ;
    write(sockfd, buff, sizeof(buff));
    bzero(buff, sizeof(buff));
    read(sockfd, buff, sizeof(buff));
    printf("From Server : %s", buff);
    if ((strncmp(buff, "exit", 4)) == 0)
    {
      printf("Client Exit...\n");
      break;
    }
  }
}

int main(int argc, char *argv[])
{
  int sockfd, connfd;
  struct sockaddr_in servaddr, cli;
  char *host;

  if (argc != 2)
  {
    fprintf(stderr, "usage: %s host\n", argv[0]);
    exit(1);
  }
  host = argv[1];
  // socket create and varification
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    printf("socket creation failed...\n");
    exit(0);
  }
  else
    printf("Socket successfully created..\n");
  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = inet_addr(host);
  servaddr.sin_port = htons(PORT);

  // connect the client socket to server socket
  if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
  {
    printf("connection with the server failed...\n");
    exit(0);
  }
  else
    printf("connected to the server..\n");

  // function for chat
  func(sockfd);

  // close the socket
  close(sockfd);
}