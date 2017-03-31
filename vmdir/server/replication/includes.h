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
 * Module Name: indexer
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * indexer module include file
 *
 */
#ifndef _WIN32

#include <config.h>

#include <vmdirsys.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// OpenLDAP ber library include files
#include <lber.h>
#include "ldap-int.h"
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>

#include <krb5/krb5.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>
#include <vmdircommon.h>
#include <vmdirclient.h>
#include <srvcommon.h>
#include <schema.h>
#include <indexcfg.h>
#include <backend.h>
#include <vmdirserver.h>
#include <ldaphead.h>
#include <middlelayer.h>
#include <replication.h>
#include <vmacl.h>

#include <structs.h>
#include <prototypes.h>

#define VDIR_SAFE_UNBIND_EXT_S(pLd)             \
    do {                                        \
        if (pLd) {                              \
            ldap_unbind_ext_s(pLd,NULL,NULL);   \
            (pLd) = NULL;                       \
        }                                       \
    } while(0)

#else

#pragma once
#include "targetver.h"
#include <windows.h>
#include <WinSock2.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <tchar.h>
#define LDAP_UNICODE 0

// OpenLDAP ber library include files
#define LBERLIB_ONLY

#include <lber.h>
#include "ldap-int.h"
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>

// SUNG kdcmerge should revmove this if not mantain our own cread cache
#include <krb5/krb5.h>

#define LW_STRICT_NAMESPACE
#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-types.h>

#undef LBERLIB_ONLY

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>
#include <vmdircommon.h>
#include <vmdirclient.h>
#include <srvcommon.h>
#include <schema.h>
#include <indexcfg.h>
#include <backend.h>
#include <vmdirserver.h>
#include <ldaphead.h>
#include <middlelayer.h>
#include <replication.h>
#include <vmacl.h>

#include <structs.h>
#include "prototypes.h"
#include "banned.h"
#define VDIR_SAFE_UNBIND_EXT_S(pLd)             \
    do {                                        \
        if (pLd) {                              \
            ldap_unbind_ext_s(pLd,NULL,NULL);   \
            (pLd) = NULL;                       \
        }                                       \
    } while(0)
#endif
