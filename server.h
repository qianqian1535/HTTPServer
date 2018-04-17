#ifndef SERVER_H
#define SERVER_H


#define MAXBUFF  1024
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
    {"mp4","application/octet-stream"},
    {0,0}
};

void error(const char *msg);
int connection(int fd, char *root);
void send_new(int fd, char *msg, int len);

#endif
