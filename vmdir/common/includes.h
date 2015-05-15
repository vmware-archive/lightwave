#ifndef _WIN32
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include <config.h>
#include <ldap.h>
#include <sasl/sasl.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <dce/rpc.h>

#include <vmdirsys.h>
#include <sys/types.h>
#include <pwd.h>
#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrorcode.h>
#include <vmdirerrors.h>
#include <vmdircommon.h>
#include <vmdirclient.h>

#include "structs.h"
#include "prototypes.h"
#include "defines.h"

#include <reg/lwreg.h>
#include <reg/regutil.h>

#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <tchar.h>
#include <wchar.h>
#include <malloc.h>
#include <errno.h>
#include <process.h>
#include <time.h>
#include <assert.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "banned.h"

#include <sasl/sasl.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <dce/rpc.h>

#include <lw/security-api.h>
#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrorcode.h>
#include <vmdirerrors.h>
#include <vmdircommon.h>
#include <vmdirclient.h>

#include "structs.h"
#include "prototypes.h"
#include "defines.h"

#endif
