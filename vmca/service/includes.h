/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#ifndef _VMCA_SERVICE_INCLUDES_H__
#define _VMCA_SERVICE_INCLUDES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32

#include <vmcasys.h>
#include <vmafdclient.h>
#include <vmafdtypes.h>

#include <lw/types.h>
#include <lw/base.h>
#include <lwerror.h>
#include <dce/dcethread.h>
#include <reg/lwreg.h>
#include <dce/rpc.h>
#include <dce/id_base.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <gssapi/gssapi.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <vmcatypes.h>
#include <vmcacommon.h>
#include <vmca_h.h>
//#include <vmcasrvutils.h>
#include <vmcadb.h>

#if 0
#include <vmrest.h>
#include <jansson.h>
#include <common_types.h>
#include <oidc_types.h>
#include <oidc.h>
#endif

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
#include "vmca_error.h"
#include "errormap.h"

#else

#pragma once
#pragma warning(disable : 4995)
#define NOMINMAX 1

#include "targetver.h"
//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <dce/dcethread.h>
#include <dce/rpc.h>
#include <dce/id_base.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <assert.h>
#include <winsock2.h>
#include "banned.h"
#include <vmafdclient.h>
#include <vmafdtypes.h>


//#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#endif

#include <dce/rpc.h>
#include <dce/dcethread.h>

#include <vmcasys.h>
#include <vmcatypes.h>
#include <vmcacommon.h>

#pragma warning (disable : 4142)
#if defined(_DEBUG)
#include "../x64/Debug/vmca_h.h"
#else
#include "../x64/Release/vmca_h.h"
#endif
#pragma warning(default:4142)

#include <vmcadb.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
#include "vmca_error.h"
#include "errormap.h"
#endif // _WIN32

#ifdef __cplusplus
}
#endif

#endif // _VMCA_SERVICE_INCLUDES_H__
