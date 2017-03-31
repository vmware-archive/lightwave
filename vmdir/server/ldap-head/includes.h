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
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <config.h>

#include <vmdirsys.h>

// OpenLDAP ber library include files
#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>

#include <middlelayer.h>
#include <vmdirserver.h>
#include <schema.h>
#include <indexcfg.h>
#include <backend.h>
#include <replication.h>

#include <ldaphead.h>

#include <vmacl.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#else

#pragma once
#include "targetver.h"
#include <windows.h>
#include <tchar.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>
#include <errno.h>
#include <limits.h>
#include <WinSock2.h>
#include <ws2tcpip.h>

#include "topdefines.h" // this one has to come before lber.h.

// OpenLDAP ber library include files
#include <lber.h>
#include "ldap-int.h"
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define LW_STRICT_NAMESPACE
#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-types.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>

#include <middlelayer.h>
#include <vmdirserver.h>
#include <schema.h>
#include <indexcfg.h>
#include <backend.h>
#include <replication.h>

#include <ldaphead.h>

#include <vmacl.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#include "banned.h"

#endif
