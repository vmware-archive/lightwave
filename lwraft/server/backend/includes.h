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
 * Module Name: Backend
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * Backend module include file
 *
 */
#ifndef _WIN32
#include <config.h>

#include <vmdirsys.h>

// OpenLDAP ber library include files
#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>
#include <vmdirserver.h>
#include <schema.h>
#include <ldaphead.h>

#include <backend.h>

#ifdef HAVE_DB_H
#include <bdbstore.h>
#endif

#ifdef HAVE_LMDB_H
#include <mdbstore.h>
#endif

#ifdef HAVE_TCBDB_H
#include <tcstore.h>
#endif

//#include "defines.h"
#include "structs.h"
//#include "prototypes.h"
#include "externs.h"

#else

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <errno.h>
#include <windows.h>
#if !defined(HAVE_DCERPC_WIN32)
#include <rpc.h>
#endif
#include <stdint.h>
#include <assert.h>
#include <tchar.h>

// OpenLDAP ber library include files
#include <lber.h>
#include <ldap.h>
#include <ldap_log.h>
#include <lber_pvt.h>
#include <lber-int.h>

#define LW_STRICT_NAMESPACE
#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-types.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>
#include <vmdirserver.h>
#include <schema.h>
#include <ldaphead.h>

#include <backend.h>

#ifdef HAVE_DB_H
#include <bdbstore.h>
#endif

#ifdef HAVE_LMDB_H
#include <mdbstore.h>
#endif

#include "structs.h"
#include "externs.h"
#include "banned.h"

#endif
