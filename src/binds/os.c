#include <os.h>

// There are no bits support since Alicorn PE only supports x64

int getOSType(void)
{
#ifdef WIN32
    return 2;
#else
    return 1;
#endif
}
