#ifndef _WIN32
#include <config.h>
#include <vmauthsvcsys.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include <vmauthsvc.h>
#include <vmauthsvcdefines.h>
#include <vmauthsvcerrorcode.h>

#include <vmauthsvccommon.h>

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
#include <openssl/x509.h>

#include <vmauthsvc.h>
#include <vmauthsvcdefines.h>
#include <vmauthsvcerrorcode.h>

#include <vmauthsvccommon.h>

#include "structs.h"
#include "prototypes.h"

#endif
