#include "fs.h"
#include "sha1.h"
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#include <winbase.h>
#else
#include <linux/limits.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

FILE *fdMap[4096] = {NULL};    // It's really hard to see 4096 files opened at the same time...
bool fdUseMap[4096] = {false}; // In use: true; Free: false;

int openFile(const char *pt, const char *mode)
{
    for (int i = 0; i < 4096; i++)
    {
        if (!fdUseMap[i])
        {
            FILE *f = fopen(pt, mode);
            if (f == NULL)
            {
                return -1;
            }
            fdMap[i] = f;
            fdUseMap[i] = true;
            return i;
        }
    }
    return -2;
}

int closeFile(int fd)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return 0;
    }
    fdUseMap[fd] = false;
    fdMap[fd] = NULL;
    if (fclose(f) == EOF)
    {
        return -1;
    }
    return 0;
}

int readFile(int fd, unsigned char *buf)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return -1;
    }
    if (fseek(f, 0L, SEEK_END) == 0)
    {
        long sz = ftell(f);
        rewind(f);
        if (fread(buf, sizeof(unsigned char), sz, f) != sizeof(char) * sz)
        {
            rewind(f);
            return -1;
        }
        else
        {
            rewind(f);
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

int writeFile(int fd, const unsigned char *buf, size_t sz)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return -1;
    }
    rewind(f);
    if (fwrite(buf, sizeof(unsigned char), sz, f) == sz)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int appendFile(int fd, const unsigned char *buf, size_t sz)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return -1;
    }
    if (fwrite(buf, sizeof(char), sz, f) == sz)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

long getFileSize(int fd)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return -1L;
    }
    if (fseek(f, 0L, SEEK_END) == 0)
    {
        long s = ftell(f);
        rewind(f);
        return s;
    }
    else
    {
        return -1L;
    }
}

char *getUserHome(void)
{
    char *homedir = malloc(PATH_MAX * sizeof(char));
    snprintf(homedir, PATH_MAX * sizeof(char), "%s", getenv("HOME"));
    return homedir;
}

int ensureDir(const char *pt)
{
#ifdef WIN32
    char delimiter = '\\';
#else
    char delimiter = '/';
#endif
    char pts[PATH_MAX];
    int len = strlen(pt);
    strcpy(pts, pt);
    for (char *p = strchr(pts + 1, delimiter); p; p = strchr(p + 1, delimiter))
    {
        *p = '\0';
#ifdef WIN32
        if (_mkdir(pts) == -1)
#else
        if (mkdir(pts, 0777) == -1)
#endif
        {
            if (errno != EEXIST)
            {
                *p = delimiter;
                return -1;
            }
        }
        *p = delimiter;
    }
    return 0;
}

int isFileExist(const char *pt)
{
    if (access(pt, F_OK) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

char **readDirectory(const char *pt)
{
    DIR *d = opendir(pt);
    struct dirent *dir;
    if (!d)
    {
        return NULL;
    }
    char **out = malloc(8192 * sizeof(char *));
    int i = 0;
    while ((dir = readdir(d)) != NULL && i < 8191)
    {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
        {
            continue;
        }
        int l = strlen(dir->d_name);
        char *t = malloc(l * sizeof(char));
        strcpy(t, dir->d_name);
        out[i] = t;
        i++;
    }
    out[i] = NULL;
    closedir(d);
    return out;
}

char *getModificationTime(const char *pt)
{
    struct stat attr;
    if (stat(pt, &attr) != 0)
    {
        return NULL;
    }
    return ctime(&attr.st_mtime);
}

int linkFile(const char *origin, const char *target)
{
#ifdef WIN32
    return CreateSymbolicLinkA(target, origin, 0x0);
#else
    return symlink(origin, target);
#endif
}

char *getSHA1(const char *pt)
{
    FILE *f = fopen(pt, "rb");
    if (f == NULL)
    {
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fclose(f);
        return NULL;
    }
    long sz = ftell(f);
    rewind(f);
    if (sz <= 0)
    {
        fclose(f);
        return NULL;
    }
    char *output = malloc(41);
    unsigned char *buf = malloc(sz);
    memset(buf, 0, sz);
    if (fread(buf, sizeof(unsigned char), sz, f) != sz)
    {
        fclose(f);
        return NULL;
    }
    unsigned char temp[20];
    SHA1(temp, buf, sz);
    fclose(f);
    free(buf);
    for (int i = 0; i < 20; i++)
    {
        sprintf(output + 2 * i, "%02x", temp[i]);
    }
    output[41] = '\0';
    return output;
}
