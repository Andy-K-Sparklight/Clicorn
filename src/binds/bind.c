#include "bind.h"
#include "fs.h"
#include "app.h"
#include "cJSON.h"
#include "os.h"
#include "z.h"
#include "net.h"
#include "b64.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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
        webview_return(arg, seq, 1, "\"No free file descriptor\"");
    }
    else if (fd == -1)
    {
        webview_return(arg, seq, 1, "\"Failed to open file\"");
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
        webview_return(arg, seq, 1, "\"Failed to close file\"");
    }
    else
    {
        webview_return(arg, seq, 0, "\"\"");
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

void readFileHandler(const char *seq, const char *req, void *arg)
// readFile(fd: number): Promise<string>
{
    // It's really not recommended to read a very large file in sync mode. Also, this will bring tons of delays in parsing and serializing them.
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    long size = getFileSize(fd->valueint);
    if (size < 0)
    {
        webview_return(arg, seq, 1, "\"File size smaller than 0\"");
    }
    else if (size > 1073741824)
    {
        webview_return(arg, seq, 1, "\"Maximum file size exceeded\"");
    }
    else
    {

        unsigned char *buf = malloc(size * sizeof(char)); // For quotes
        if (buf == NULL)
        {
            webview_return(arg, seq, 1, "\"Cannot allocate enough memory space\"");
        }
        else
        {

            if (readFile(fd->valueint, buf) != 0)
            {
                webview_return(arg, seq, 1, "\"Failed to read\"");
            }
            else
            {
                char *res = b64_encode(buf, size);
                char *dat = malloc((strlen(res) + 2) * sizeof(char));
                sprintf(dat, "`%s`", res);
                webview_return(arg, seq, 0, dat);
                free(res);
                free(dat);
            }
        }
        free(buf);
    }
    cJSON_Delete(argv);
}

void writeFileHandler(const char *seq, const char *req, void *arg)
// writeFile(fd: number, buf: Buffer): Promise<void>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    cJSON *buf = cJSON_GetArrayItem(argv, 1); // Base64
    size_t bufl = strlen(buf->valuestring);
    size_t decl;
    unsigned char *dat = b64_decode_ex(buf->valuestring, bufl, &decl);
    if (writeFile(fd->valueint, dat, decl) == -1)
    {
        webview_return(arg, seq, 1, "\"Failed to write file\"");
    }
    else
    {
        webview_return(arg, seq, 0, "\"\"");
    }
    cJSON_Delete(argv);
}

void appendFileHandler(const char *seq, const char *req, void *arg)
// appendFile(fd: number, buf: Buffer): Promise<void>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *fd = cJSON_GetArrayItem(argv, 0);
    cJSON *buf = cJSON_GetArrayItem(argv, 1); // Of string format, but actually a buffer
    size_t bufl = strlen(buf->valuestring);
    if (appendFile(fd->valueint, buf->valuestring, bufl) == -1)
    {
        webview_return(arg, seq, 1, "\"Failed to append file\"");
    }
    else
    {
        webview_return(arg, seq, 0, "\"\"");
    }
    cJSON_Delete(argv);
}

void getUserHomeHandler(const char *seq, const char *req, void *arg)
{
    char *home = getUserHome();
    char *dat = calloc(strlen(home) + 2, sizeof(char));
    sprintf(dat, "'%s'", home);
    webview_return(arg, seq, 0, dat);
    free(home);
    free(dat);
}

void ensureDirHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *pt = cJSON_GetArrayItem(argv, 0);
    if (ensureDir(pt->valuestring) == -1)
    {
        webview_return(arg, seq, 1, "\"Failed to create dirs\"");
    }
    else
    {
        webview_return(arg, seq, 0, "\"\"");
    }
    cJSON_Delete(argv);
}

void unZipHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *origin = cJSON_GetArrayItem(argv, 0);
    cJSON *dest = cJSON_GetArrayItem(argv, 1);
    if (unZip(origin->valuestring, dest->valuestring) != 0)
    {
        webview_return(arg, seq, 1, "\"Failed to unzip\"");
    }
    else
    {
        webview_return(arg, seq, 0, "\"\"");
    }
    cJSON_Delete(argv);
}

void isFileExistHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *pt = cJSON_GetArrayItem(argv, 0);
    if (isFileExist(pt->valuestring) == 0)
    {
        webview_return(arg, seq, 0, "true");
    }
    else
    {
        webview_return(arg, seq, 0, "false");
    }
    cJSON_Delete(argv);
}

void readDirectoryHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *pt = cJSON_GetArrayItem(argv, 0);
    char **dt = readDirectory(pt->valuestring);
    if (dt == NULL)
    {
        webview_return(arg, seq, 1, "\"Failed to readdir\"");
    }
    else
    {
        char json[65536] = "[";
        int i = 0;
        while (dt[i] != NULL)
        {
            strcat(json, "\"");
            strcat(json, dt[i]);
            strcat(json, "\",");
            free(dt[i]);
            i++;
        }
        free(dt);
        strcat(json, "]");
        webview_return(arg, seq, 0, json);
    }
    cJSON_Delete(argv);
}

void removeHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *pt = cJSON_GetArrayItem(argv, 0);
    if (remove(pt->valuestring) != 0)
    {
        printf("Failed to remove: %s", pt->valuestring);
    }
    webview_return(arg, seq, 0, "\"\""); // Resolve anyway
    cJSON_Delete(argv);
}

void printHandler(const char *seq, const char *req, void *arg)
// print(msg: string): Promise<void>
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *msg = cJSON_GetArrayItem(argv, 0);
    printf("Remote: %s", msg->valuestring);
    webview_return(arg, seq, 0, "\"\"");
    cJSON_Delete(argv);
}

void resizeHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *w = cJSON_GetArrayItem(argv, 0);
    cJSON *h = cJSON_GetArrayItem(argv, 1);
    resize(arg, w->valueint, h->valueint);
    webview_return(arg, seq, 0, "\"\"");
    cJSON_Delete(argv);
}

void openExternalHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *e = cJSON_GetArrayItem(argv, 0);
    char *u = malloc((strlen(e->valuestring) + 16) * sizeof(char));
#ifdef WIN32
    sprintf(u, "start %s", e->valuestring);
#else
    sprintf(u, "xdg-open %s", e->valuestring);
#endif
    if (system(u) != 0)
    {
        webview_return(arg, seq, 1, "\"\"");
    }
    else
    {
        webview_return(arg, seq, 0, "\"\"");
    }

    cJSON_Delete(argv);
}

void getModificationTimeHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *pt = cJSON_GetArrayItem(argv, 0);
    char tm[128];
    char *md = getModificationTime(pt->valuestring);
    if (md == NULL)
    {
        webview_return(arg, seq, 1, "\"Failed to stat\"");
    }
    else
    {
        sprintf(tm, "`%s`", md);
        free(md);
        webview_return(arg, seq, 0, tm);
    }
    cJSON_Delete(argv);
}

void linkFileHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *o = cJSON_GetArrayItem(argv, 0);
    cJSON *t = cJSON_GetArrayItem(argv, 1);
    if (linkFile(o->valuestring, t->valuestring) != 0)
    {
        webview_return(arg, seq, 1, "\"Failed to link\"");
    }
    else
    {
        webview_return(arg, seq, 0, "\"\"");
    }
    cJSON_Delete(argv);
}

void getSHA1Handler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *pt = cJSON_GetArrayItem(argv, 0);
    char *hsh = getSHA1(pt->valuestring);
    if (hsh == NULL)
    {
        webview_return(arg, seq, 1, "\"Cannot getSHA1\"");
    }
    else
    {
        char sha[43];
        sprintf(sha, "`%s`", hsh);
        webview_return(arg, seq, 0, sha);
    }
    free(hsh);
    cJSON_Delete(argv);
}

void getOsTypeHandler(const char *seq, const char *req, void *arg)
{
    webview_return(arg, seq, 0, getOsType() == 2 ? "\"win32\"" : "\"linux\"");
}

void spawnProcHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *cmd = cJSON_GetArrayItem(argv, 0);
    size_t len = strlen(cmd->valuestring);
    char *cmdx = malloc((len + 1) * sizeof(char));
    strcpy(cmdx, cmd->valuestring);
    int i;
    if ((i = spawnProc(cmdx, arg)) < 0)
    {
        webview_return(arg, seq, 1, "\"Failed to popen\"");
    }
    else
    {
        char pc[16];
        sprintf(pc, "%d", i);
        webview_return(arg, seq, 0, pc);
    }
    cJSON_Delete(argv);
}

void getFreeMemoryHandler(const char *seq, const char *req, void *arg)
{
    char mem[64];
    sprintf(mem, "%ld", getFreeMemory());
    webview_return(arg, seq, 0, mem);
}

void getTotalMemoryHandler(const char *seq, const char *req, void *arg)
{
    char mem[64];
    sprintf(mem, "%ld", getTotalMemory());
    webview_return(arg, seq, 0, mem);
}

void chdirHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *t = cJSON_GetArrayItem(argv, 0);
    if (chdir(t->valuestring) == 0)
    {
        webview_return(arg, seq, 0, "\"\"");
    }
    else
    {
        webview_return(arg, seq, 1, "\"Failed to chdir\"");
    }
    cJSON_Delete(argv);
}

void closeWindowHandler(const char *seq, const char *req, void *arg)
{
    webview_return(arg, seq, 0, "\"\"");
    webview_terminate(arg);
}
void downloadFileCallback(int status, char *seq, void *arg)
{
    char out[16];
    sprintf(out, "\"%d\"", status);
    if (status == 0)
    {
        webview_return(arg, seq, 0, out);
    }
    else
    {
        webview_return(arg, seq, 1, out);
    }
    free(seq);
}

void downloadFileHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *url = cJSON_GetArrayItem(argv, 0);
    cJSON *savePath = cJSON_GetArrayItem(argv, 1);
    cJSON *timeout = cJSON_GetArrayItem(argv, 2);
    char *u = malloc(4096);
    char *sp = malloc(4096);
    char *seq0 = malloc((strlen(seq) + 1) * sizeof(char));
    strcpy(u, url->valuestring);
    strcpy(sp, savePath->valuestring);
    downloadFile(u, sp, timeout->valueint, downloadFileCallback, seq0, arg);
    cJSON_Delete(argv);
}
void netGetCallback(Response *res, char *seq, void *arg)
{
    if (res->body == NULL)
    {
        char outStr[64];
        sprintf(outStr, "\"GET failed, err code: %d\"", res->statusCode);
        webview_return(arg, seq, 1, outStr);
    }
    else
    {
        char *ebd = b64_encode(res->body, res->bodyLength);
        char *output = malloc((strlen(ebd) + 32) * sizeof(char));
        sprintf(output, "{status:%d,body:\"%s\"}", res->statusCode, ebd);
        free(res->body);
        free(ebd);
        webview_return(arg, seq, 0, output);
        free(output);
    }
    free(seq); // Was allocated by us.
    free(res);
}

void netGetHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *url = cJSON_GetArrayItem(argv, 0);
    cJSON *headers = cJSON_GetArrayItem(argv, 1);
    cJSON *timeout = cJSON_GetArrayItem(argv, 2);
    char *u = malloc(4096);
    char *hd = malloc(32768);
    char *seq0 = malloc((strlen(seq) + 1) * sizeof(char));
    strcpy(seq0, seq);
    strcpy(u, url->valuestring);
    strcpy(hd, headers->valuestring);
    netGet(u, hd, timeout->valueint, netGetCallback, seq0, arg);
    cJSON_Delete(argv);
}

void netPostCallback(Response *res, char *seq, void *arg)
{
    if (res->body == NULL)
    {
        char outStr[64];
        sprintf(outStr, "\"POST failed, err code: %d\"", res->statusCode);
        webview_return(arg, seq, 1, outStr);
    }
    else
    {
        char *ebd = b64_encode(res->body, res->bodyLength);
        char *output = malloc((strlen(ebd) + 32) * sizeof(char));
        sprintf(output, "{status:%d,body:\"%s\"}", res->statusCode, ebd);
        free(res->body);
        free(ebd);
        webview_return(arg, seq, 0, output);
        free(output);
    }
    free(seq); // Was allocated by us.
    free(res);
}

void netPostHandler(const char *seq, const char *req, void *arg)
{
    cJSON *argv = cJSON_Parse(req);
    cJSON *url = cJSON_GetArrayItem(argv, 0);
    cJSON *headers = cJSON_GetArrayItem(argv, 1);
    cJSON *body = cJSON_GetArrayItem(argv, 2);
    cJSON *timeout = cJSON_GetArrayItem(argv, 3);
    char *u = malloc(4096);
    char *hd = malloc(32768);
    char *bd = malloc(32768);
    char *seq0 = malloc((strlen(seq) + 1) * sizeof(char));
    strcpy(seq0, seq);
    strcpy(u, url->valuestring);
    strcpy(hd, headers->valuestring);
    strcpy(bd, headers->valuestring);
    netPost(u, hd, bd, timeout->valueint, netPostCallback, seq0, arg);
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
    webview_bind(w, "_readFile", readFileHandler, w);
    webview_bind(w, "_writeFile", writeFileHandler, w);
    webview_bind(w, "_appendFile", appendFileHandler, w);
    webview_bind(w, "_getUserHome", getUserHomeHandler, w);
    webview_bind(w, "_ensureDir", ensureDirHandler, w);
    webview_bind(w, "_unZip", unZipHandler, w);
    webview_bind(w, "_isFileExist", isFileExistHandler, w);
    webview_bind(w, "_readDirectory", readDirectoryHandler, w);
    webview_bind(w, "_remove", removeHandler, w);
    webview_bind(w, "_getModificationTime", getModificationTimeHandler, w);
    webview_bind(w, "_linkFile", linkFileHandler, w);
    webview_bind(w, "_getSHA1", getSHA1Handler, w);

    // App
    webview_bind(w, "_resize", resizeHandler, w);
    webview_bind(w, "_openExternal", openExternalHandler, w);
    webview_bind(w, "_closeWindow", closeWindowHandler, w);

    // Os
    webview_bind(w, "_getOsType", getOsTypeHandler, w);
    webview_bind(w, "_spawnProc", spawnProcHandler, w);
    webview_bind(w, "_getFreeMemory", getFreeMemoryHandler, w);
    webview_bind(w, "_getTotalMemory", getTotalMemoryHandler, w);
    webview_bind(w, "_chdir", chdirHandler, w);

    // Net
    webview_bind(w, "_downloadFile", downloadFileHandler, w);
    webview_bind(w, "_netGet", netGetHandler, w);
    webview_bind(w, "_netPost", netPostHandler, w);
}
