#include "includes.h"
#if defined(_DEBUG)
#include "../x64/Debug/vmevent_s.c"
#else
#include "../x64/Release/vmevent_s.c"
#endif
