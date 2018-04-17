/*Name: Qianqian Zheng
Student #: 813288
gitlab login id: q.zheng11@student.unimelb.edu.au
*/
/*parts of the code here is modified based on server.c from workshop 5 and
example server from https://dzone.com/articles/web-server-c*/
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
#include <fcntl.h>
#include <errno.h>

#include "server.h"

int  main(int argc, char const *argv[]) {

    if (argc <3 ) {
        error("no port/path");
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

    listen(listenfd, MAXCLIENT);
    int pid;
    while (1) {
        int connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

        if (connfd < 0) {
            error("ERROR on accept.");
        }
        pid = fork();
        if (pid < 0) {
            error("ERROR on fork");
        }else if (pid == 0){

            connection(connfd, root);
        }

        close(connfd);
    }
    close(listenfd);

    return 0;
}
/*for handling errors*/
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
    char *buffer = calloc(MAXBUFF,sizeof(char));
    int read_new = read(fd,buffer, MAXBUFF-1);
    if (read_new <= 0) {
        printf("Recieve Failed\n");
    }

    //printf("buffer: %s\n", buffer);
    // Check if it is a get request
    if (strncmp(buffer, "GET ", 4) != 0) {
        printf("wrong path %s\n", buffer);
    }else{
        //modify get request to get file path
        char* ptr = buffer + 4;

        char* file = strtok(strtok(ptr, "\n")," "); //get first line

        //serve index.html if route is empty
        if(ptr[strlen(file) - 1] == '/'){
            strcat(file, "index.html");
        }
        //string for actual path to open file
        char *path = malloc(strlen(root)+strlen(file)+1);
        strcpy(path, root);
        strcat(path, file);

        //read file from system
        int openfd = open(path, O_RDONLY);
        free(path);
        //send 404 response upon open failure
        if (openfd == -1) {
            printf("404 File not found Error\n");
            send_new(fd, "HTTP/1.0 404", UNKNOWN);

        } else{
            char* s = strchr(ptr, '.');
            int i;
            for (i = 0; extensions[i].ext != NULL; i++) {
                if (strcmp(s + 1, extensions[i].ext) == 0) {
                    //serve file, response headers: Http Status, Content-type
                    send_new(fd, "HTTP/1.0 200 OK\r\n",UNKNOWN);
                    char *mime_type=  extensions[i].mediatype;

                    send_new(fd,"Content-Type: ",UNKNOWN);
                    send_new(fd,mime_type, UNKNOWN);
                    send_new(fd,"\r\n\r\n",UNKNOWN);
                    while (1) {
                        char *sendbuffer = calloc(MAXBUFF,sizeof(char));
                        // Read data into buffer, store how many bytes were
                        //actually read in bytes_read.
                        int bytes_read = read(openfd, sendbuffer, MAXBUFF);
                        if (bytes_read == 0) {
                            //  done reading from the file
                            break;
                        }else if (bytes_read < 0) {
                            printf("error reading %s\n",extensions[i].mediatype);
                            break;
                        }
                        //send file
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
