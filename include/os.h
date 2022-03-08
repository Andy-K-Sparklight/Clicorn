#ifndef CLICORN_OS
#define CLICORN_OS

/*
Get the type os the OS.

GNU/Linux: 1, Windows: 2
*/
int getOsType(void);

/*
Popen a new process.

The return value is a number (PC) represents the process and data will be broadcasted using: new CustomEvent("ProcOutput-<PC>", {detail: <data>}); After the process exits, the exit code will be broadcasted using: new CustomEvent("ProcExit-<PC>", {detail: <code>});

Return 0 if ok, or -2 if pc used up.
*/
int spawnProc(char *command, void *w);
/*
Get the free memory size.
*/
unsigned long getFreeMemory();

/*
Get the total memory size.
*/
unsigned long getTotalMemory();
#endif
