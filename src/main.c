#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#define WEBVIEW_HEADER
#include "webview.h"
#include "bind.h"
#include "fs.h"
#include "pack.h"
#include "z.h"
#include "os.h"
#include "net.h"

#ifdef WIN32
#include <windows.h>
#include <winuser.h>
#else
#include <gtk/gtk.h>
#endif

char *getEntryPoint(void);
int extractSelf(void);

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
#else
int main(char **argv)
{
#endif
    printf("Initializing main window!\n");
    webview_t w;
    if (argv[0] == "debug")
    {
        w = webview_create(1, NULL);
    }
    else
    {
        w = webview_create(0, NULL);
    }
    void *nw = webview_get_window(w);
#ifdef WIN32
    LONG lExStyle = GetWindowLong(nw, GWL_EXSTYLE);
    lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(nw, GWL_EXSTYLE, lExStyle);
    SetWindowPos(nw, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);
#else
    gtk_window_set_decorated(nw, FALSE);
#endif
    webview_set_title(w, "Clicorn (Alicorn PE)");
    webview_set_size(w, 960, 540, WEBVIEW_HINT_NONE);
    bindAll(w);
    char *entry = getEntryPoint();
    if (strcmp(entry, "") == 0)
    {
        if (extractSelf() != 0)
        {
            printf("Entry point cannot be accessed, nor created. Therefore I shall exit.\n");
            exit(1);
        }
        entry = getEntryPoint();
    }
    printf("Found entry at %s\n", entry);
    FILE *f = fopen(entry, "r");
    free(entry);
    if (f == NULL)
    {
        printf("Cannot open entry, exit.\n");
        exit(1);
    }
    if (fseek(f, 0, SEEK_END))
    {
        printf("Cannot fseek, exit.\n");
        fclose(f);
        exit(1);
    }
    long sz = ftell(f);
    rewind(f);
    unsigned char *buf = malloc(sz + 1);
    long szr;
    if (fread(buf, 1, sz, f) != sz)
    {
        printf("Cannot read entry, exit.\n");
        fclose(f);
        exit(1);
    }
    fclose(f);
    buf[sz] = '\0';
    netInit();
    printf("Opening main window!\n");
    webview_set_html(w, buf);
    webview_run(w);
    webview_destroy(w);
    free(buf);
    netClean();
    return 0;
}

// Gets the Renderer.html file
char *getEntryPoint(void)
{
#ifdef WIN32
    char delimiter = '\\';
    char delimiterStr[] = "\\";
#else
    char delimiter = '/';
    char delimiterStr[] = "/";
#endif
    char *targetDir = getUserHome();
    int len = strlen(targetDir);
    char end = targetDir[len - 1];
    char conjDir[4096];
    strcpy(conjDir, targetDir);

    if (end != delimiter)
    {
        strcat(conjDir, delimiterStr);
    }
    strcat(conjDir, ".alicornpe");

    free(targetDir);
    char *rendererHTML = malloc(4096 * sizeof(char));
    strcpy(rendererHTML, conjDir);
    strcat(rendererHTML, delimiterStr);
    strcat(rendererHTML, "Renderer.html");
    if (access(rendererHTML, F_OK) == 0)
    {
        return rendererHTML;
    }
    else
    {
        free(rendererHTML);
        return "";
    }
}

int extractSelf(void)
{
#ifdef WIN32
    char delimiter = '\\';
    char delimiterStr[] = "\\";
#else
    char delimiter = '/';
    char delimiterStr[] = "/";
#endif
    printf("Extracting resources!\n");
    // Ensure dir
    char *targetDir = getUserHome();
    int len = strlen(targetDir);
    char end = targetDir[len - 1];
    char conjDir[4096];
    strcpy(conjDir, targetDir);
    if (end != delimiter)
    {
        strcat(conjDir, delimiterStr);
    }

    strcat(conjDir, ".alicornpe");
    strcat(conjDir, delimiterStr);
    free(targetDir);
    ensureDir(conjDir);
    // Write ZIP

    char tarTarget[4096];
    strcpy(tarTarget, conjDir);
    strcat(tarTarget, "pack.zip");
    FILE *f = fopen(tarTarget, "wb");

    if (fwrite(pack_zip, sizeof(pack_zip), 1, f) != 1)
    {
        printf("Failed to write zip archive.\n");
        fclose(f);
        return -1;
    }

    fclose(f);

    if (unZip(tarTarget, conjDir) != 0)
    {
        printf("Failed to extract archive.\n");
        return -1;
    }
    return 0;
}
