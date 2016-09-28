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
#include <sys/stat.h>

#include <config.h>

#include <vmkdcsys.h>

#include <vmkdc.h>
#include <vmkdctypes.h>
#include <vmkdcdefines.h>
#include <vmkdcerrorcode.h>

#include <vmkdccommon.h>

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
#include <time.h>
#include <direct.h>

#include "banned.h"

#include <vmkdc.h>
#include <vmkdctypes.h>
#include <vmkdcdefines.h>
#include <vmkdcerrorcode.h>

#include <vmkdccommon.h>
#include <pthread.h>

#include "structs.h"
#include "prototypes.h"

#endif
