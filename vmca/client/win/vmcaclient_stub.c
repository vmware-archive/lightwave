#include "includes.h"
#if defined(_DEBUG)
#include "../x64/Debug/vmca_c.c"
#else
#include "../x64/Release/vmca_c.c"
#endif
