#ifndef CLICORN_Z
#define CLICORN_Z

#include <stdio.h>

/*
Inflate a xz file.

Return 0 if success, or -1 if something is wrong.
*/
int inflateXZ(const char *origin, const char *dest);

/*
Deflate a file to xz.

Return 0 if success, or -1 if something is wrong.
*/
int deflateXZ(const char *origin, const char *dest);

/*
Inflate a zip file. Dest should be a directory.

Return 0 if success, or -1 if something is wrong.
*/
int unZip(const char *origin, const char *dest);

/*
Deflate a directory to a zip file. Origin should be a directory.

Return 0 if success, or -1 if something is wrong.
*/
int doZip(const char *origin, const char *dest);

/*
Extract a tarball to a directory. Dest should be a directory.

Return 0 if success, or -1 if something is wrong.
*/
int extractTar(const char *origin, const char *dest);

/*
Make a tarball from a directory. Origin should be a directory.

Return 0 if success, or -1 if something is wrong.
*/
int createTar(const char *origin, const char *dest);

#endif
