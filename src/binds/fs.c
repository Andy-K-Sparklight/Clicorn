#include <fs.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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

int readFileSync(int fd, unsigned char *buf)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return -1;
    }
    int cur;
    if (fseek(f, 0L, SEEK_END) == 0)
    {
        long sz = ftell(f);
        rewind(f);
        if (fread(buf, sizeof(char), sz, f) != sizeof(char) * sz)
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

int writeFileSync(int fd, unsigned char *buf, size_t sz)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return -1;
    }
    int cur;
    rewind(f);
    if (fwrite(buf, sizeof(char), sz, f) == sizeof(char) * sz)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int appendFileSync(int fd, unsigned char *buf, size_t sz)
{
    FILE *f = fdMap[fd];
    if (f == NULL)
    {
        return -1;
    }
    int cur;
    if (fwrite(buf, sizeof(char), sz, f) == sizeof(char) * sz)
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
