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
#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <reg/lwreg.h>

#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <config.h>

#include <vmdnssys.h>

#include <vmdns.h>
#include <vmdnstypes.h>
#include <vmdnsdefines.h>
#include <vmdnserrorcode.h>

#include <vmdnsbuffer.h>
#include <vmdnscommon.h>

#include "structs.h"
#include "prototypes.h"

#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <pthread.h>
#include <assert.h>
#include "banned.h"

#include <vmdns.h>
#include <vmdnstypes.h>
#include <vmdnsdefines.h>
#include <vmdnserrorcode.h>

#include <vmdnsbuffer.h>
#include <vmdnscommon.h>

#include "structs.h"
#include "prototypes.h"
#include <Ws2ipdef.h>
#endif

#include "extern.h"
#include "defines.h"
#include "dce/rpc.h"
