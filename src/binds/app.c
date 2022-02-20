#include "app.h"

void resize(webview_t window, int w, int h)
{
    webview_set_size(window, w, h, WEBVIEW_HINT_NONE);
}
