#ifndef _WIN32
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include <config.h>

#include <vmkdcsys.h>

#include <vmkdc.h>
#include <vmkdctypes.h>
#include <vmkdcdefines.h>
#include <vmkdcerrorcode.h>

#include <vmkdccommon.h>

#include "structs.h"
#include "prototypes.h"

#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <time.h>
#include <direct.h>

#include "banned.h"

#include <vmkdc.h>
#include <vmkdctypes.h>
#include <vmkdcdefines.h>
#include <vmkdcerrorcode.h>

#include <vmkdccommon.h>
#include <pthread.h>

#include "structs.h"
#include "prototypes.h"

#endif
