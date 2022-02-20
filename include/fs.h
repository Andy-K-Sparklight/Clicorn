#ifndef CLICORN_FS
#define CLICORN_FS

#include <stddef.h>
/*
Open a file.

Return the related descriptor.
If an error occurred, return -1 and the details are stored in errno.
If there are no free descriptor, return -2.
*/
int openFile(const char *pt, const char *mode);

/*
Close a file.

Return 0 if success, or -1 if an error occurred.
It's always safe to close a free fd.
Even if this call failed, this fd will be dropped and should no longer be used.
*/
int closeFile(int fd);

/*
Read from the file as ArrayBuffer. Run in current thread.

Return 0 if success, or -1 if fd is invalid.
This will also rewind the file.
*/
int readFileSync(int fd, unsigned char *buf);

/*
Write to a file from ArrayBuffer. Run in current thread.

Return 0 if success, or -1 if something is wrong.
This will always write from the beginning.
*/
int writeFileSync(int fd, unsigned char *buf, size_t sz);

/*
Append to a file from ArrayBuffer. Run in current thread.

Return 0 if success, or -1 if something is wrong.
*/
int appendFileSync(int fd, unsigned char *buf, size_t sz);

/*
Get the size of a file.

Return the original size of the file, or -1 if something is wrong.
*/
long getFileSize(int fd);

/*
Get the home of the user.
*/
char *getUserHome(void);

#endif
