#ifndef _WIN32
#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <reg/lwreg.h>

#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include <config.h>

#include <vmdnssys.h>

#include <vmdns.h>
#include <vmdnstypes.h>
#include <vmdnsdefines.h>
#include <vmdnserrorcode.h>

#include <vmdnsbuffer.h>
#include <vmdnscommon.h>

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
#include <pthread.h>

#include "banned.h"

#include <vmdns.h>
#include <vmdnstypes.h>
#include <vmdnsdefines.h>
#include <vmdnserrorcode.h>

#include <vmdnsbuffer.h>
#include <vmdnscommon.h>

#include "structs.h"
#include "prototypes.h"

#endif

#include "extern.h"
#include "dce/rpc.h"
