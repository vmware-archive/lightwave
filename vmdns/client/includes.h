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
#include <vmdnssys.h>

#include <lwrpcrt/lwrpcrt.h>
#include <dce/rpc.h>
#include <dce/uuid.h>
#include <dce/dcethread.h>
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>

#include <vmdns.h>
#include <vmdnsdefines.h>

#include <vmdnscommon.h>

#include "defines.h"
#include "vmdns_h.h"
#include "prototypes.h"

#else //_ WIN32

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <dce/uuid.h>
#include <dce/dcethread.h>
typedef unsigned char uuid_t[16];  // typedef dce_uuid_t uuid_t;
#include <dce/rpc.h>
#include <pthread.h>
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>

#include "banned.h" // windows banned APIs

#include <vmdns.h>
#include <vmdnsdefines.h>

#include <vmdnscommon.h>

#include "defines.h"
#include "vmdns_h.h"
#include "prototypes.h"

#endif // #ifndef _WIN32
