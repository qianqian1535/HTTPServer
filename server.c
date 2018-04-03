/*Qianqian Zheng
 */
/*some code is taken from server.c from workshop 5 and
   https://dzone.com/articles/web-server-c*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include "server.h"
#include <fcntl.h>
#include <errno.h>

#define MAXBUFF  1025
#define MAXCLIENT 10
#define UNKNOWN -1
void error(const char *msg);
int connection(int fd, char *root);
void send_new(int fd, char *msg, int len);

int  main(int argc, char const *argv[]) {
        if (argc <3 ) {
                fprintf(stderr, "ERROR, no port provided\n");
                exit(1);
        }
        int portno;
        char root[100];
        portno = atoi(argv[1]);
        strcpy(root,argv[2]);

        struct sockaddr_in serv_addr;
        //create socket
        int listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (listenfd < 0)
        {
                error("ERROR opening socket");
        }
        memset(&serv_addr, '0', sizeof(serv_addr)); //initialise server address
        serv_addr.sin_family = AF_INET; //Type of address –internet IP
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Listen on ANY IP Addr
        serv_addr.sin_port = htons(portno); //Listen on port 5000

        bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        listen(listenfd, MAXCLIENT); // maximum number of client connections to queue
        while (1) {
                int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
                if (connfd < 0) {
                        error("ERROR on accept");
                }else{
                        connection(connfd, root);
                        break;
                }
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
   This function parses the HTTP requests,
   arrange resource locations,
   check for supported media types,
   serves files in a web root,
   sends the HTTP error codes.
 */
int connection(int fd, char *root) {
        char buffer[MAXBUFF];

        memset(buffer, '0', sizeof(buffer)); //initialise buffer

        int read_new = read(fd,buffer, MAXBUFF-1);
        if (read_new <= 0) {
                printf("Recieve Failed\n");
        }
        printf("%s\n", buffer);
        // Check if it is a get request
        if (strncmp(buffer, "GET ", 4) == 0) {
                //modify get request to get file path
                char* ptr = buffer + 4;
                char* path = strtok(ptr, "\n"); //get first line
                strcat(root,path);
                //read file from system
                int openfd = open(root, O_RDONLY);
                //send 404 response upon open failure
                if (openfd == -1) {
                        printf("404 File not found Error\n");
                        send_new(fd, "HTTP/1.1 404 Not Found\r\n", UNKNOWN);
                        send_new(fd, "Server : Web Server in C\r\n\r\n",UNKNOWN);
                        send_new(fd, "<html><head><title>404 Not Found</head></title>",UNKNOWN);
                        send_new(fd, "<body><p>404 Not Found: The requested resource could not be found!</p></body></html>",UNKNOWN);

                } else{
                        char* extension;
                        extension= strtok(root,".");
                        /* walk through other tokens */
                        while( extension != NULL) {
                                extension = strtok(NULL, ".");
                        }

                        //serve file, response headers: Http Status, Content-type
                        send_new(fd, "HTTP/1.0 200 OK\r\n",UNKNOWN);
                        send_new(fd,strcat("Content-Type: ", extension),UNKNOWN);
                        send_new(fd,"\r\n\r\n",UNKNOWN);
                        while (1) {
                                char *sendbuffer = calloc(MAXBUFF,sizeof(char));
                                // Read data into buffer.  We may not have enough to fill up buffer, so we
                                // store how many bytes were actually read in bytes_read.
                                int bytes_read = read(openfd, sendbuffer, sizeof(sendbuffer));
                                if (bytes_read == 0) {
                                        // We're done reading from the file
                                        break;
                                }else if (bytes_read < 0) {
                                        // handle errors
                                }
                                void* p =sendbuffer;
                                send_new(fd, p, bytes_read);
                                free(sendbuffer);
                        }
                }
                close(openfd);
        }
        shutdown(fd, SHUT_RDWR);
}

/*
   use a loop for the write, because not all of the data may be written in one call
 */
void send_new(int fd, char *msg, int len) {
        if(len == UNKNOWN) {
                len = strlen(msg);
        }
        while (len > 0) {
                int bytes_written = write(fd, msg, len);
                if (bytes_written <= 0) {
                        // handle errors
                        printf("Error in send\n");
                }
                len -= bytes_written;
                msg += bytes_written;
        }
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
