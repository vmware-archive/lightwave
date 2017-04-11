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



/*
 * Module Name: Kdc main
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * Kdc main module include file
 *
 */


#ifndef _WIN32/* ============= LINUX ONLY ================ */
#include <config.h>
#include <vmkdcsys.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <lwrpcrt/lwrpcrt.h>
#include <reg/lwreg.h>
#include <reg/regutil.h>

#include <net/if.h>
#include <ifaddrs.h>

#else
/* ========================= WIN32 ONLY ======================== */

#pragma once
#include <errno.h>
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>
#include <winsvc.h>
#include <Winreg.h>
#include <assert.h>
#include <Ws2tcpip.h>

#define LW_STRICT_NAMESPACE
#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-types.h>

#endif

//SUNG vmkdc merge,
#include <topdefines.h>
#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdircommon.h>
#include "srvcommon.h"
#include "vmdirserver.h"
#include "ldaphead.h"
#include "middlelayer.h"
#include <dce/rpc.h>
#include <pthread.h>


/* Common include between Linux and Windows */
#include <vmkdc.h>
#include <vmkdctypes.h>
#include <vmkdcdefines.h>
#include <vmkdcerrorcode.h>
#include <vmkdccommon.h>
#include <kdcsrvcommon.h>
#include <vmkdcserver.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#include <kdckrb5/types.h>
#include <kdckrb5/structs.h>
#include <kdckrb5/prototypes.h>

#include "directory.h"
#include "networking.h"
#include "process.h"
#include "parsekt.h"
#include "princtok.h"
#include "fgetsl.h"

#include <krb5-crypto/includes.h>
