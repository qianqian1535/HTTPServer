/*Name: Qianqian Zheng
Student #: 813288
gitlab login id: q.zheng11@student.unimelb.edu.au
*/
#ifndef SERVER_H
#define SERVER_H


#define MAXBUFF  2048  //max buffer size for sending file and reading requests
#define MAXCLIENT 10 // maximum number of client connections to queue
#define UNKNOWN -1 //for indicating unknown length for send_new()

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
    {"mp4","application/octet-stream"}, //large file to test multithreading
    {0,0}
};

/*for handling errors*/
void error(const char *msg);

/* This function parses the HTTP requests, arrange resource locations,
check for supported media types, serves files in a web root,
sends the HTTP error codes.*/
int connection(int fd, char *root);

/*use a loop for the write&send,
because not all of the data may be written in one call*/
void send_new(int fd, char *msg, int len);

#endif
