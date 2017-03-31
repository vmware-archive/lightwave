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

#if 1
/* define VMCISLIB_BIND_T incomplete type; should define in vmcislib */
#  ifndef _VMCISLIB_BIND_T
#    define _VMCISLIB_BIND_T
typedef void *VMCISLIB_BIND_T;
#  endif
#endif

#ifndef _WIN32 // Linux includes

#include <config.h>
#include <vmdirsys.h>

#include <lwrpcrt/lwrpcrt.h>
#include <ldap.h>

#include <krb5.h>

#include <reg/lwreg.h>
#include <reg/regutil.h>

#else //_ WIN32

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <assert.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winreg.h>
#include "ldap-int.h"
#include "ldap.h"
#include <tchar.h>

#define LW_STRICT_NAMESPACE
#include <lw/hash.h>

#include "banned.h" // windows banned APIs
#endif // #ifndef _WIN32

/* common includes across all platforms */
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>

#include <vmdir.h>
#include <vmdirdefines.h>
#include <vmdirerrorcode.h>
#include <vmdircommon.h>
#include <vmdirerrors.h>
#include <vmdirclient.h>
#include <type_spec.h>

#include "defines.h"
#include "structs.h"
#include "vmdir_h.h"
#include "vmdirftp_h.h"
#include "vmdirdbcp_h.h"
#include "vmdirsuperlog_h.h"
#include "vmdirurgentrepl_h.h"
#include "vmdirraft_h.h"
#include "prototypes.h"
#include "externs.h"
