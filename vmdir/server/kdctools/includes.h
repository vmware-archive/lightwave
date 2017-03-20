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
 * Module Name: Kdc main
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * Kdc main module include file
 *
 */


#ifndef _WIN32/* ============= LINUX ONLY ================ */

#include <config.h>
#include <vmkdcsys.h>

#else
/* ========================= WIN32 ONLY ======================== */

#include "targetver.h"
#include <windows.h>
#include <stdint.h>
#include <tchar.h>
#ifndef strdup
#define strdup _strdup
#endif
#endif

/* Common include between Linux and Windows */
#include <dce/rpc.h>

#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdircommon.h>
#include <kdcsrvcommon.h>
#include <vmkdcdefines.h>
#include <vmkdccommon.h>
#include <vmkdcerrorcode.h>
#include <vmkdcserver.h>

#include <kdckrb5/types.h>
#include <kdckrb5/structs.h>
#include <kdckrb5/prototypes.h>

#include "princtok.h"
#include "parsekt.h"
#include "fgetsl.h"
