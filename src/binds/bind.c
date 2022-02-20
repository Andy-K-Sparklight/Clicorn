#include "bind.h"
#include "fs.h"
#include "app.h"
#include "cJSON.h"
#include "errno.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"

char *replaceWord(const char *s, const char *oldW, const char *newW)
{
    char *result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);

    for (i = 0; s[i] != '\0'; i++)
    {
        if (strstr(&s[i], oldW) == &s[i])
        {
            cnt++;
            i += oldWlen - 1;
        }
    }
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1);

    i = 0;
    while (*s)
    {
        if (strstr(s, oldW) == s)
        {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }

    result[i] = '\0';
    return result;
}

void openFileHandler(const char *seq, const char *req, void *arg)
// openFile(pt: string, mode: string): Promise<number>
{
    cJSON *argv = cJSON_Parse(req); // Assume args are valid, if webview runs well.
    cJSON *pt = cJSON_GetArrayItem(argv, 0);
    cJSON *mode = cJSON_GetArrayItem(argv, 1);
    int fd = openFile(pt->valuestring, mode->valuestring);
    if (fd == -2)
    {
        webview_return(arg, seq, 1, "No free file descriptor");
    }
    else if (fd == -1)
    {
        webview_return(arg, seq, 1, strerror(errno));
    }
    else
    {
        char fds[4];
        sprintf(fds, "%d", fd);
        webview_return(arg, seq, 0, fds);
    }
    cJSON_Delete(argv);
}

void closeFileHandler(const char *seq, const char *req, void *arg)
// closeFile(fd: number): Promise<void>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    int s = closeFile(fd->valueint);
    if (s == -1)
    {
        webview_return(arg, seq, 1, strerror(errno));
    }
    else
    {
        webview_return(arg, seq, 0, "");
    }
    cJSON_Delete(argv);
}

void getFileSizeHandler(const char *seq, const char *req, void *arg)
// getFileSize(fd: number): Promise<number>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    long size = getFileSize(fd->valueint);
    char szs[32];
    sprintf(szs, "%ld", size);
    webview_return(arg, seq, 0, szs);
    cJSON_Delete(argv);
}

void readFileSyncHandler(const char *seq, const char *req, void *arg)
// readFileSync(fd: number): Promise<Buffer>
{
    // It's really not recommended to read a very large file in sync mode. Also, this will bring tons of delays in parsing and serializing them.
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    long size = getFileSize(fd->valueint);
    if (size < 0)
    {
        webview_return(arg, seq, 1, "File size smaller than 0");
    }
    else if (size > 1073741824)
    {
        webview_return(arg, seq, 1, "Maximum file size exceeded");
    }
    else
    {

        unsigned char *buf = malloc(size * sizeof(char)); // For quotes
        if (buf == NULL)
        {
            webview_return(arg, seq, 1, "Cannot allocate enough memory space");
        }
        else
        {
            if (readFileSync(fd->valueint, buf) != 0)
            {
                webview_return(arg, seq, 1, "Failed to read");
            }
            else
            {

                char *ebuf1 = replaceWord(buf, "\\", "\\\\");
                char *ebuf2 = replaceWord(ebuf1, "`", "\\`");
                free(ebuf1);

                char *o = malloc((size + 2) * sizeof(char));
                sprintf(o, "`%s`", ebuf2);
                free(ebuf2);
                webview_return(arg, seq, 0, o);
                free(o);
            }
        }
        free(buf);
    }
    cJSON_Delete(argv);
}

void writeFileSyncHandler(const char *seq, const char *req, void *arg)
// writeFileSync(fd: number, buf: Buffer): Promise<void>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    cJSON *buf = cJSON_GetArrayItem(argv, 1); // Of string format, but actually a buffer
    size_t bufl = strlen(buf->valuestring);
    if (writeFileSync(fd->valueint, buf->valuestring, bufl) == -1)
    {
        webview_return(arg, seq, 1, "Failed to write");
    }
    else
    {
        webview_return(arg, seq, 0, "");
    }
    cJSON_Delete(argv);
}

void appendFileSyncHandler(const char *seq, const char *req, void *arg)
// appendFileSync(fd: number, buf: Buffer): Promise<void>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    cJSON *buf = cJSON_GetArrayItem(argv, 1); // Of string format, but actually a buffer
    size_t bufl = strlen(buf->valuestring);
    if (appendFileSync(fd->valueint, buf->valuestring, bufl) == -1)
    {
        webview_return(arg, seq, 1, "Failed to append");
    }
    else
    {
        webview_return(arg, seq, 0, "");
    }
    cJSON_Delete(argv);
}

void getUserHomeHandler(const char *seq, const char *req, void *arg)
{
    char *home = getUserHome();
    webview_return(arg, seq, 0, home);
    free(home);
}

void printHandler(const char *seq, const char *req, void *arg)
// print(msg: string): Promise<void>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *msg = cJSON_GetArrayItem(argv, 0);
    printf("Remote: %s", msg->valuestring);
    webview_return(arg, seq, 0, "");
    cJSON_Delete(argv);
}

void resizeHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *w = cJSON_GetArrayItem(argv, 0);
    cJSON *h = cJSON_GetArrayItem(argv, 1);
    resize(arg, w->valueint, h->valueint);
    webview_return(arg, seq, 0, "");
    cJSON_Delete(argv);
}

void bindAll(webview_t w)
{
    // Dev
    webview_bind(w, "_print", printHandler, w);

    // File System
    webview_bind(w, "_openFile", openFileHandler, w);
    webview_bind(w, "_closeFile", closeFileHandler, w);
    webview_bind(w, "_getFileSize", getFileSizeHandler, w);
    webview_bind(w, "_readFileSync", readFileSyncHandler, w);
    webview_bind(w, "_writeFileSync", writeFileSyncHandler, w);
    webview_bind(w, "_appendFileSync", appendFileSyncHandler, w);
    webview_bind(w, "_getUserHome", getUserHomeHandler, w);

    // App
    webview_bind(w, "_resize", resizeHandler, w);
}
