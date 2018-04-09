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

typedef struct {
    char *ext;
    char *mediatype;
} extn;

//Possible media types
extn extensions[] ={
    {"jpg","image/jpeg"},
    {"js",  "text/javascript"  },
    {"css", "text/css" },
    {"html","text/html" },
    {0,0}
};

void error(const char *msg);
int connection(int fd, char *root);
void send_new(int fd, char *msg, int len);

int  main(int argc, char const *argv[]) {
    if (argc <3 ) {
        error("no port");
    }
    int portno, pid;
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
            /*pid = fork();
            if (pid < 0) {
                error("ERROR on fork");
            }else if (pid == 0)*/ {
                close(listenfd);
                printf("doing the connection() function \n");
                connection(connfd, root);
                //exit(0);
            }
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
    char *buffer = calloc(MAXBUFF,sizeof(char));;


    int read_new = read(fd,buffer, MAXBUFF-1);
    if (read_new <= 0) {
        printf("Recieve Failed\n");
    }

    printf("buffer: %s\n", buffer);
    // Check if it is a get request
    if (strncmp(buffer, "GET ", 4) != 0) {
        printf("wrong path %s\n", buffer);
    }else{
        //modify get request to get file path
        char* ptr = buffer + 4;
        char* path = strtok(strtok(ptr, "\n")," "); //get first line
        strcat(root,path);
        printf("root after strcat: %s\n",root );
        //read file from system
        int openfd = open(root, O_RDONLY);
        //send 404 response upon open failure
        if (openfd == -1) {
            printf("404 File not found Error\n");
            send_new(fd, "HTTP/1.0 404", UNKNOWN);

        } else{
            char* s = strchr(ptr, '.');
            printf("extention: %s\n", s);
            int i;
            for (i = 0; extensions[i].ext != NULL; i++) {
                if (strcmp(s + 1, extensions[i].ext) == 0) {
                    //serve file, response headers: Http Status, Content-type
                    send_new(fd, "HTTP/1.0 200 OK",UNKNOWN);
                    char * mime_type= strcat("Content-Type: ", extensions[i].mediatype);
                    printf("mime string %s\n",mime_type );
                    send_new(fd,mime_type,UNKNOWN);
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
                            printf("error reading %s\n",extensions[i].mediatype);
                            break;
                        }
                        //void* p = sendbuffer;
                        send_new(fd, sendbuffer, bytes_read);
                        free(sendbuffer);
                    }
                    break;
                }
            }

        }
        close(openfd);
        shutdown(fd, SHUT_RDWR);
    }
    free(buffer);
    return 0;
}
/*
use a loop for the write, because not all of the data may be written in one call
*/
void send_new(int fd, char *msg, int len) {

    if(len == UNKNOWN) {
        len = strlen(msg);
    }printf("sending %d bytes message\n", len);
    while (len > 0) {
        int bytes_written = write(fd, msg, len);
        if (bytes_written <= 0) {
            // handle errors
            printf("Error in send\n");
        }else{
            printf("Sent %d bytes\nmessage is %s\n", bytes_written, msg);
        }
        len -= bytes_written;
        msg += bytes_written;
    }
    /*
// send fixed-length data to pre-pend variable-length field with the latter's size
send(fd, &msg, sizeof(len), 0);
// send the variable-length field.
send(fd, msg, len, 0);*/
    /*if (send(fd, msg, len, 0) == -1) {
            printf("Error in send\n");
        }
        else{
            printf("Sent message is %s\n",  msg);
        }*/
    /**/
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
