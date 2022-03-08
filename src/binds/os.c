#include "os.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <b64.h>
#define WEBVIEW_HEADER
#include <webview.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/sysinfo.h>
#endif

// There are no bits support since Alicorn PE only supports x64
// Not macOS either

int getOsType(void)
{
#ifdef WIN32
    return 2;
#else
    return 1;
#endif
}

FILE *pcMap[4096] = {NULL};
bool pcUseMap[4096] = {false};

#ifdef WIN32
#define popen _popen
#endif

typedef struct
{
    void *w;
    char *command;
    int pc;
} SpawnArgs;

#define CLICORN_PROC_OUTPUT_BUF_MAX 4096

static void *spawnProcRunnable(void *sas)
{
    SpawnArgs *sa = (SpawnArgs *)sas;

    FILE *f;
    fflush(f);
    f = popen(sa->command, "r");
    pcMap[sa->pc] = f;
    char buf[CLICORN_PROC_OUTPUT_BUF_MAX];
    while (fgets(buf, CLICORN_PROC_OUTPUT_BUF_MAX, f) != NULL)
    {
        char *bs64 = b64_encode(buf, strlen(buf));
        char js[2 * CLICORN_PROC_OUTPUT_BUF_MAX];
        sprintf(js, "window.dispatchEvent(new CustomEvent(\"ProcOutput-%d\", {detail:\"%s\"}));", sa->pc, bs64);
        free(bs64);
        webview_eval(sa->w, js);
    }
    pcMap[sa->pc] = NULL;
    pcUseMap[sa->pc] = false;
    int ret = pclose(f);
    char js[128];
    sprintf(js, "window.dispatchEvent(new CustomEvent(\"ProcExit-%d\",{detail:%d}));", sa->pc, ret);
    webview_eval(sa->w, js);
    free(sa->command);
    free(sa);
}

int spawnProc(char *command, void *w)
{
    SpawnArgs *sa = malloc(sizeof(SpawnArgs));
    sa->w = w;
    sa->command = command;
    sa->pc = -1;
    for (int i = 0; i < 4096; i++)
    {
        if (!pcUseMap[i])
        {
            pcUseMap[i] = true;
            sa->pc = i;
            break;
        }
    }
    if (sa->pc == -1)
    {
        return -1;
    }
    pthread_t pid;
    pthread_create(&pid, NULL, spawnProcRunnable, sa);
    return sa->pc;
}

unsigned long getFreeMemory()
{
#ifdef WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return statex.ullAvailPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    return info.freeram + info.bufferram;
#endif
}

unsigned long getTotalMemory()
{
#ifdef WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    return statex.ullTotalPhys;
#else
    struct sysinfo info;
    sysinfo(&info);
    return info.totalram;
#endif
}
