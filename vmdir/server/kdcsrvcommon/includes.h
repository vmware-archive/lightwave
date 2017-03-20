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
#include <vmkdcsys.h>

#else

#pragma once
#include "targetver.h"
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#define LW_STRICT_NAMESPACE
#include <lw/types.h>
#include <lw/hash.h>
#include <lw/security-types.h>

#endif

#include <vmkdc.h>
#include <vmkdctypes.h>
#include <vmkdcdefines.h>
#include <vmkdcerrorcode.h>

#include <dce/rpc.h>
#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif
#include <dce/idlddefs.h>
#include <pthread.h>
#include <vmdirtypes.h>
#include <vmkdccommon.h>
#include <kdcsrvcommon.h>
#include <vmkdcserver.h>

#ifdef VMDIR_ENABLE_PAC
#include "vmdir_pac_h.h"
#endif
