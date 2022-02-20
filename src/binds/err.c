#include "err.h"
#include <errno.h>

int getErrno(void)
{
    return errno;
}
