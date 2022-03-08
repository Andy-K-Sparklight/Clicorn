#ifndef CLICORN_Z
#define CLICORN_Z

/*
Inflate some gz data.

Return 0 if success, or -1 if something is wrong.
*/
int inflateGZ(const unsigned char *buf, unsigned long sourceLength, unsigned char *target, unsigned long *targetLength);

/*
Inflate a zip file WITHOUT its directories. (i.e. only root files)

Return 0 if success, or -1 if something is wrong.
*/
int unZip(const char *origin, const char *dest);

/*
Make a zip file.

Return 0 if success, or -1 if something is wrong.
*/
int mkZip(const char *origin, const char *dest);
#endif
