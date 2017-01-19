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



#ifndef _WIN32
#include <config.h>
#include <vmafdsys.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vecs_error.h>
#include <vmafdcommon.h>

#ifdef __MACH__
#include <sys/un.h>
#include <sys/ucred.h>
#endif

#include "structs.h"
#include "prototypes.h"
#include "defines.h"
#include "externs.h"
#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#pragma comment (lib, "Ws2_32.lib")
#include <windows.h>
#include <wincrypt.h>
#include <psapi.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <pthread.h>
#include <direct.h>
#include <in6addr.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <vecs_error.h>
#include "banned.h"
#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vmafdcommon.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include "structs.h"
#include "prototypes.h"
#include "defines.h"
#include "externs.h"
#endif

#include <ldap.h>
#include <dce/rpc.h>


