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
#else
#include <gtk/gtk.h>
#endif

char *getEntryPoint(void);
int extractSelf(void);

#ifdef WIN32
int WINAPI WinMain(HINSTANCE hInt, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
#else
int main()
{
#endif
    printf("Initializing main window!\n");
#ifdef WIN32
    webview_t w = webview_create(0, NULL);
#else
    webview_t w = webview_create(1, NULL);
#endif

#ifndef WIN32
    void *nw = webview_get_window(w);
    gtk_window_set_decorated(nw, FALSE);
#endif
    printf("Setting window title!\n");
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
    char url[4096];
    sprintf(url, "file://%s", entry);
    free(entry);
    netInit();
    printf("Opening main window!\n");
    webview_navigate(w, url);
    webview_run(w);
    netClean();
    webview_destroy(w);
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
