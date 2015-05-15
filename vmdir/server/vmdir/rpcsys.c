#include "includes.h"
#if !defined(_WIN32) || defined(HAVE_DCERPC_WIN32)
#include "rpc.c"
#else
#include "win/rpc.c"
#endif
