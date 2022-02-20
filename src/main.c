#include <stdio.h>
#include <stdlib.h>
#define WEBVIEW_HEADER
#include "webview.h"
#include "bind.h"
#include "limits.h"

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
#else
int main()
{
#endif
    webview_t w = webview_create(0, NULL);
    webview_set_title(w, "Clicorn (Alicorn PE)");
    webview_set_size(w, 960, 540, WEBVIEW_HINT_NONE);
    bindAll(w);
    webview_navigate(w, "about:blank");
    webview_run(w);
    webview_destroy(w);
    return 0;
}
