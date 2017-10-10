/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
#include <vmdirsys.h>

#include <ldap_log.h>

#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <tchar.h>

#endif

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrorcode.h>
#include <vmdirerrors.h>
#include <vmdircommon.h>
#include <srvcommon.h>

#include <backend.h>
#include <middlelayer.h>
#include <ldaphead.h>
#include <resthead.h>
#include <schema.h>
#include <vmdirserver.h>

#ifdef REST_ENABLED

#include <copenapi/copenapi.h>
#include <curl/curl.h>
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
#include <jansson.h>
#include <sasl/saslutil.h>
#include <vmrest.h>

#include <ssotypes.h>
#include <ssoerrors.h>
#include <oidc_types.h>
#include <oidc.h>

#include "defines.h"
#include "structs.h"
#include "externs.h"
#include "prototypes.h"

#endif
