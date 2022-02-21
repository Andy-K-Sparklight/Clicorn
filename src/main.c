#include <stdio.h>
#include <stdlib.h>
#define WEBVIEW_HEADER
#include "webview.h"
#include "bind.h"

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#else
#include <gtk/gtk.h>
#include <sys/stat.h>
#endif

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
#else
int main()
{
#endif
    webview_t w = webview_create(0, NULL);
    void *nw = webview_get_window(w);

#ifndef WIN32
    gtk_window_set_decorated(nw, FALSE);
#endif

    webview_set_title(w, "Clicorn (Alicorn PE)");
    webview_set_size(w, 960, 540, WEBVIEW_HINT_NONE);
    bindAll(w);
    webview_navigate(w, "about:blank");
    webview_run(w);
    webview_destroy(w);
    return 0;
}

extern unsigned char pack_tar_xz[]; // It's always this name

int extractSelf()
{
}
