#include "net.h"
#include <pthread.h>
#include "curl/curl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void netInit(void)
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void netClean(void)
{
    curl_global_cleanup();
}

typedef struct
{
    char *url;
    char *savePath;
    int timeout;
    char *seq;
    void *arg;
    void (*fn)(int status, char *seq, void *arg);
} DownloadMeta;

static size_t writeData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

static void *runDownload(void *meta)
{
    DownloadMeta *m = (DownloadMeta *)meta;
    CURL *curl_handle;
    curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_URL, m->url);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, (long)m->timeout);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeData);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 20L);
    FILE *f = fopen(m->savePath, "wb");
    if (f)
    {
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, f);
        CURLcode c = curl_easy_perform(curl_handle);
        if (c != CURLE_OK)
        {
            m->fn(1, m->seq, m->arg); // 1: Curl Error (RETRY)
        }
        else
        {

            long statusCode;
            curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &statusCode);

            if (statusCode < 200 || statusCode >= 300)
            {

                m->fn(-1, m->seq, m->arg); // -1: Unexpected status code (DO NOT RETRY)
            }
            else
            {
                m->fn(0, m->seq, m->arg); // 0: OK
            }
        }
        fclose(f);
    }
    else
    {
        m->fn(-1, m->seq, m->arg);
    }
    curl_easy_cleanup(curl_handle);
    free(m->url);
    free(m->savePath);
    free(m);

    return NULL;
}

void downloadFile(char *url, char *savePath, int timeout, void (*fn)(int status, char *seq, void *arg), char *seq, void *arg)
{
    DownloadMeta *meta = malloc(sizeof(DownloadMeta));
    meta->savePath = savePath;
    meta->timeout = timeout;
    meta->fn = fn;
    meta->url = url;
    meta->seq = seq;
    meta->arg = arg;
    pthread_t thr;
    pthread_create(&thr, NULL, runDownload, meta);
}

typedef struct
{
    char *url;
    char *headers;
    int timeout;
    char *seq;
    void *arg;
    void (*fn)(Response *response, char *seq, void *arg);
} GetRequest;

typedef struct
{
    size_t offset;
    char *buf;
} Buffer;

static size_t
writeMemory(void *ptr, size_t size, size_t nmemb, void *data)
{
    Buffer *buf = (Buffer *)data;
    memcpy(buf->buf + buf->offset, ptr, size * nmemb);
    buf->offset += size * nmemb;
    return size * nmemb;
}

static void *runGet(void *req)
{
    GetRequest *m = (GetRequest *)req;
    CURL *curl_handle = curl_easy_init();
    cJSON *header = cJSON_Parse(m->headers);
    cJSON *headerItem = header->child;
    struct curl_slist *outHeaders = NULL;
    while (headerItem)
    {
        size_t len = strlen(headerItem->string) + strlen(headerItem->valuestring) + 3;
        char *dat = malloc(len);
        sprintf(dat, "%s: %s", headerItem->string, headerItem->valuestring);
        outHeaders = curl_slist_append(outHeaders, dat);
        free(dat);
        headerItem = headerItem->next;
    }
    cJSON_Delete(header);
    curl_easy_setopt(curl_handle, CURLOPT_URL, m->url);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, (long)(m->timeout));
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeMemory);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 20L);
    char *buf = malloc(1048576);
    Buffer *ebuf = malloc(sizeof(Buffer));
    ebuf->buf = buf;
    ebuf->offset = 0;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, ebuf);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, outHeaders);
    CURLcode c = curl_easy_perform(curl_handle);
    Response *res = malloc(sizeof(Response));

    if (c != CURLE_OK)
    {
        free(buf);
        res->body = NULL;
        res->statusCode = -1;
    }
    else
    {
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &(res->statusCode));
        res->body = buf;
        res->bodyLength = ebuf->offset;
    }
    m->fn(res, m->seq, m->arg);
    curl_easy_cleanup(curl_handle);
    curl_slist_free_all(outHeaders);
    free(m->url);
    free(m->headers);
    free(m);
    free(ebuf);
    return NULL;
}

void netGet(char *url, char *headers, int timeout, void (*fn)(Response *response, char *seq, void *arg), char *seq, void *arg)
{
    GetRequest *req = malloc(sizeof(GetRequest));
    req->headers = headers;
    req->timeout = timeout;
    req->fn = fn;
    req->url = url;
    req->seq = seq;
    req->arg = arg;
    pthread_t thr;
    pthread_create(&thr, NULL, runGet, req);
}

typedef struct
{
    char *url;
    char *headers;
    char *body;
    int timeout;
    char *seq;
    void *arg;
    void (*fn)(Response *response, char *seq, void *arg);
} PostRequest;

static void *runPost(void *req)
{
    PostRequest *m = (PostRequest *)req;
    CURL *curl_handle = curl_easy_init();
    cJSON *header = cJSON_Parse(m->headers);
    cJSON *headerItem = header->child;
    struct curl_slist *outHeaders = NULL;
    while (headerItem)
    {
        size_t len = strlen(headerItem->string) + strlen(headerItem->valuestring) + 3;
        char *dat = malloc(len);
        sprintf(dat, "%s: %s", headerItem->string, headerItem->valuestring);
        outHeaders = curl_slist_append(outHeaders, dat);
        free(dat);
        headerItem = headerItem->next;
    }
    cJSON_Delete(header);
    curl_easy_setopt(curl_handle, CURLOPT_URL, m->url);
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, (long)(m->timeout));
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeMemory);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 20L);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, m->body);
    char *buf = malloc(1048576);
    Buffer *ebuf = malloc(sizeof(Buffer));
    ebuf->buf = buf;
    ebuf->offset = 0;
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, ebuf);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, outHeaders);

    CURLcode c = curl_easy_perform(curl_handle);
    Response *res = malloc(sizeof(Response));

    if (c != CURLE_OK)
    {
        free(buf);
        res->body = NULL;
        res->statusCode = c;
    }
    else
    {
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &(res->statusCode));
        res->body = buf;
        res->bodyLength = ebuf->offset;
    }

    m->fn(res, m->seq, m->arg);
    curl_easy_cleanup(curl_handle);
    curl_slist_free_all(outHeaders);
    free(m->url);
    free(m->headers);
    free(m->body);
    free(m);
    free(ebuf);

    return NULL;
}

void netPost(char *url, char *headers, char *body, int timeout, void (*fn)(Response *response, char *seq, void *arg), char *seq, void *arg)
{
    PostRequest *req = malloc(sizeof(PostRequest));
    req->headers = headers;
    req->timeout = timeout;
    req->fn = fn;
    req->url = url;
    req->seq = seq;
    req->arg = arg;
    req->body = body;
    pthread_t thr;
    pthread_create(&thr, NULL, runPost, req);
}
