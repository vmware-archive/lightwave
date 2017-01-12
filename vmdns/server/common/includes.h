/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

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

#include <ldap.h>
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
#include <time.h>

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
