#ifndef CLICORN_NET
#define CLICORN_NET

#include "cJSON.h"
/*
Init curl
*/
void netInit(void);

/*
Cleanup curl
*/
void netClean(void);

/*
Download a file with a new thread started.

Callback with 0 if successful, or other err value defined in curl.h
*/
void downloadFile(char *url, char *savePath, int timeout, void (*fn)(int status, char *seq, void *arg), char *seq, void *arg);

typedef struct
{
    int statusCode;
    void *body;
    size_t bodyLength;
} Response;

/*
Perform a GET request.
*/
void netGet(char *url, char *headers, int timeout, void (*fn)(Response *response, char *seq, void *arg), char *seq, void *arg);

/*
Perform a POST request.
*/
void netPost(char *url, char *headers, char *body, int timeout, void (*fn)(Response *response, char *seq, void *arg), char *seq, void *arg);

#endif
