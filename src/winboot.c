#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Clicorn.exe.h"
#include "webview.dll.h"
#include "WebView2Loader.dll.h"
#include "libcurl.dll.h"
#include "libwinpthread_1.dll.h"

#ifdef WIN32

#include <windows.h>
int WINAPI
WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)

#else

int main()

#endif
{
    if (access("Clicorn.exe", F_OK) != 0)
    {
        FILE *f = fopen("Clicorn.exe", "wb");
        fwrite(Clicorn_exe, sizeof(Clicorn_exe), 1, f);
        fclose(f);
    }
    if (access("webview.dll", F_OK) != 0)
    {
        FILE *f = fopen("webview.dll", "wb");
        fwrite(webview_dll, sizeof(webview_dll), 1, f);
        fclose(f);
    }
    if (access("WebView2Loader.dll", F_OK) != 0)
    {
        FILE *f = fopen("WebView2Loader.dll", "wb");
        fwrite(WebView2Loader_dll, sizeof(WebView2Loader_dll), 1, f);
        fclose(f);
    }
    if (access("libcurl-x64.dll", F_OK) != 0)
    {
        FILE *f = fopen("libcurl-x64.dll", "wb");
        fwrite(libcurl_dll, sizeof(libcurl_dll), 1, f);
        fclose(f);
    }
    if (access("libwinpthread-1.dll", F_OK) != 0)
    {
        FILE *f = fopen("libwinpthread-1.dll", "wb");
        fwrite(libwinpthread_1_dll, sizeof(libwinpthread_1_dll), 1, f);
        fclose(f);
    }
    system("Clicorn.exe");
}
