/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : includes.h
 *
 * Abstract :
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32

#include <vmeventsys.h>

#include <locale.h>
#include <lwadvapi.h>
#include <lw/types.h>
#include <lw/base.h>
#include <lwerror.h>
#include <lw/security-types.h>
#include <lw/security-api.h>
#include <lw/ntstatus.h>
#include <lsa/lsa.h>
#include <lwstr.h>
#include <dce/rpc.h>
#include <dce/dcethread.h>
#include <reg/lwreg.h>

#include <config.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <assert.h>

#include <vmeventcommon.h>
#include <vmevent_h.h>
#include <vmeventdb.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
#else // _WIN32
#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
/*
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include <tchar.h>
#include <errno.h>
#include <winsvc.h>
#include <dce/rpc.h>
#include <dce/dcethread.h>
#include <locale.h>
*/
#include <stdint.h>
#include <stdio.h>
#include <tchar.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include "banned.h" // windows banned APIs
#include <dce/rpc.h>
#include <dce/dcethread.h>
#include <locale.h>

#include <openssl/x509.h>

#include <vmeventsys.h>
#include <vmevent.h>
#include <vmeventcommon.h>
#include <vmeventdb.h>
#include <vmevent_h.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#endif //_WIN32

#ifdef __cplusplus
}
#endif

