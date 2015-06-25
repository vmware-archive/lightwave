#include <ldap.h>
#include "defines.h"

#ifndef _WIN32
#include <config.h>

#include <vmdnssys.h>
#include <vmdns.h>
#include <vmdnstypes.h>
#include <vmdnsdefines.h>
#include <vmdnserrorcode.h>

#include <vmdnsbuffer.h>
#include <vmdnscommon.h>
#include <srvcommon.h>
#include <vmsock.h>

#include <vmdnsserver.h>

#else

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <dce/rpc.h>
#include <pthread.h>

#include <vmdns.h>
#include <vmdnstypes.h>
#include <vmdnsdefines.h>
#include <vmdnserrorcode.h>
#include <vmdnsbuffer.h>
#include <vmdnscommon.h>
#include <srvcommon.h>
#include <vmdnsserver.h>
#include <vmsock.h>

#endif

#include "structs.h"
#include "externs.h"
#include "prototypes.h"

#ifdef __cplusplus
extern "C"
#endif
DWORD
VmDirSafeLDAPBind(
    LDAP**      ppLd,
    PCSTR       pszHost,
    PCSTR       pszUPN,         // opt, if exists, will try SRP mech
    PCSTR       pszPassword     // opt, if exists, will try SRP mech
);
