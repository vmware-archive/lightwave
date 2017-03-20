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
#include <backend.h>
#include <middlelayer.h>

#include <indexcfg.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#else

#pragma once
#include "targetver.h"
#include <windows.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <search.h>
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
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>

#include <vmdirserver.h>
#include <schema.h>
#include <backend.h>
#include <middlelayer.h>

#include <indexcfg.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#include "banned.h"

#endif
