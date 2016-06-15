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
#include "prototypes.h"
#include "schema/prototypes.h"
#include "externs.h"
