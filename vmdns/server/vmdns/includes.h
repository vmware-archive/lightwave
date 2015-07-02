/*
 * Copyright (c) VMware Inc.  All rights Reserved.
 */

/*
 * Module Name: dns main
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * dns main module include file
 *
 */

#define VMDNS_PORT 389

#ifndef _WIN32/* ============= LINUX ONLY ================ */

#include <config.h>
#include <vmdnssys.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <lwrpcrt/lwrpcrt.h>
#include <dce/rpc.h>
#include <reg/lwreg.h>
#include <reg/regutil.h>

#else
/* ========================= WIN32 ONLY ======================== */

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>
#include <errno.h>
#include <winsvc.h>
#include <assert.h>
#include <Ws2tcpip.h>
#include <dce/rpc.h>
#include <pthread.h>

#endif

#include <ldap.h>
#include <sasl/sasl.h>

#include <vmdns.h>
#include <vmdnstypes.h>
#include <vmdnsdefines.h>
#include <vmdnserrorcode.h>
#include <vmdnsbuffer.h>
#include <vmdnscommon.h>
#include <srvcommon.h>
#include <vmsock.h>
#include <vmdnsserver.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "vmdns_h.h"
#include "externs.h"
