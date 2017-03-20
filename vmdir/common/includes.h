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

#ifdef __MACH__
#include <sys/un.h>
#include <sys/ucred.h>
#endif

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

#include <sasl/sasl.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <dce/rpc.h>

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
#include "banned.h"

#endif
