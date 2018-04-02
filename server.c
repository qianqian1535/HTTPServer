/*Qianqian Zheng
*/
/*some code is taken from server.c from workshop 5*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include "server.h"
#include <fcntl.h>
#include <errno.h>

#define MAXBUFF  1025
#define MAXCLIENT 10
void error(const char *msg);

int  main(int argc, char const *argv[]) {
    if (argc <3 ) {
     fprintf(stderr, "ERROR, no port provided\n");
     exit(1);
    }
    int portno;
    char path_to_dir[100];
    portno = atoi(argv[1]);
    strcpy(path_to_dir,argv[2]);

    char sendBuff[MAXBUFF];
    struct sockaddr_in serv_addr;

    int listenfd = socket(AF_INET, SOCK_STREAM, 0); //create socket


    if (listenfd < 0)
    {
        error("ERROR opening socket");

    }
    memset(&serv_addr, '0', sizeof(serv_addr)); //initialise server address
    memset(sendBuff, '0', sizeof(sendBuff)); //initialise send buffer

    serv_addr.sin_family = AF_INET; //Type of address –internet IP
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Listen on ANY IP Addr
    serv_addr.sin_port = htons(portno); //Listen on port 5000

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, MAXCLIENT); // maximum number of client connections to queue
while (1) {
    int connfd;
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    if (connfd < 0){
     error("ERROR on accept");
 }
    /* Read characters from the connection,
    then process */

    int a = read(connfd,sendBuff, MAXBUFF-1);

    printf("Here is the message: %s\n",sendBuff);

    snprintf(sendBuff, sizeof(sendBuff), "Hello World!");
    a = write(connfd, sendBuff, strlen(sendBuff));
    close(connfd);

}
    close(listenfd);

    return 0;
}
/*for errors*/
void error(const char *msg) {
 perror(msg);
 exit(1);
}

/*
 This function recieves the buffer
 until an "End of line(EOL)" byte is recieved
 */
int recv_new(int fd, char *buffer) {
 char *p = buffer; // Use of a pointer to the buffer rather than dealing with the buffer directly
 int eol_matched = 0; // Use to check whether the recieved byte is matched with the buffer byte or not
 while (recv(fd, p, 1, 0) != 0) // Start receiving 1 byte at a time
 {
  if (*p == EOL[eol_matched]) // if the byte matches with the first eol byte that is '\r'
    {
   ++eol_matched;
   if (eol_matched == EOL_SIZE) // if both the bytes matches with the EOL
   {
    *(p + 1 - EOL_SIZE) = '\0'; // End the string
    return (strlen(buffer)); // Return the bytes recieved
   }
  } else {
   eol_matched = 0;
  }
  p++; // Increment the pointer to receive next byte
 }
 return (0);
}
/*
 This function parses the HTTP requests,
 arrange resource locations,
 check for supported media types,
 serves files in a web root,
 sends the HTTP error codes.
 */
int connection(int fd) {
 char request[500], resource[500], *ptr;
 int fd1, length;
 if (recv_new(fd, request) == 0) {
  printf("Recieve Failed\n");
 }
 printf("%s\n", request);
 // Check for a valid browser request
 ptr = strstr(request, " HTTP/");
 if (ptr == NULL) {
  printf("NOT HTTP !\n");
 } else {
  *ptr = 0;
  ptr = NULL;

  if (strncmp(request, "GET ", 4) == 0) {
   ptr = request + 4;
  }
  if (ptr == NULL) {
   printf("Unknown Request ! \n");
  } else {
   if (ptr[strlen(ptr) - 1] == '/') {
    strcat(ptr, "index.html");
   }
   strcpy(resource, webroot());
   strcat(resource, ptr);
   char* s = strchr(ptr, '.');
   int i;
   for (i = 0; extensions[i].ext != NULL; i++) {
    if (strcmp(s + 1, extensions[i].ext) == 0) {
     fd1 = open(resource, O_RDONLY, 0);
     printf("Opening \"%s\"\n", resource);
     if (fd1 == -1) {
      printf("404 File not found Error\n");
      send_new(fd, "HTTP/1.1 404 Not Found\r\n");
      send_new(fd, "Server : Web Server in C\r\n\r\n");
      send_new(fd, "<html><head><title>404 Not Found</head></title>");
      send_new(fd, "<body><p>404 Not Found: The requested resource could not be found!</p></body></html>");
      //Handling php requests
     } else if (strcmp(extensions[i].ext, "php") == 0) {
      php_cgi(resource, fd);
      sleep(1);
      close(fd);
      exit(1);
     } else {
      printf("200 OK, Content-Type: %s\n\n",
        extensions[i].mediatype);
      send_new(fd, "HTTP/1.1 200 OK\r\n");
      send_new(fd, "Server : Web Server in C\r\n\r\n");
      if (ptr == request + 4) // if it is a GET request
        {
       if ((length = get_file_size(fd1)) == -1)
        printf("Error in getting size !\n");
       size_t total_bytes_sent = 0;
       ssize_t bytes_sent;
       while (total_bytes_sent < length) {
        //Zero copy optimization
        if ((bytes_sent = sendfile(fd, fd1, 0,
          length - total_bytes_sent)) <= 0) {
         if (errno == EINTR || errno == EAGAIN) {
          continue;
         }
         perror("sendfile");
         return -1;
        }
        total_bytes_sent += bytes_sent;
       }

      }
     }
     break;
    }
    int size = sizeof(extensions) / sizeof(extensions[0]);
    if (i == size - 2) {
     printf("415 Unsupported Media Type\n");
     send_new(fd, "HTTP/1.1 415 Unsupported Media Type\r\n");
     send_new(fd, "Server : Web Server in C\r\n\r\n");
     send_new(fd, "<html><head><title>415 Unsupported Media Type</head></title>");
     send_new(fd, "<body><p>415 Unsupported Media Type!</p></body></html>");
    }
   }

   close(fd);
  }
 }
 shutdown(fd, SHUT_RDWR);
}

/*
Creating a thread
int pthread_create(pthread_t *id,
const pthread_attr_t *attr,
void *(func)(void *),
void *arg);
Id of thread itself:
pthread_t pthread_self();
*/
