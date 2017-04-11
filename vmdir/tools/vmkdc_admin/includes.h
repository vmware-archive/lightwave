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
 * Module Name: vmkdc_admin
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * vmkdc_admin main module include file
 *
 */
#ifndef _WIN32 /* ============= LINUX ONLY ================ */

#include <config.h>
#include <vmkdcsys.h>

#include <lwrpcrt/lwrpcrt.h>
#include <reg/lwreg.h>
#include <reg/regutil.h>

#else
/* ========================= WIN32 ONLY ======================== */

#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <tchar.h>

#define LW_STRICT_NAMESPACE
#include <lw/types.h>

#include "banned.h"

#endif

#include <kdctools/includes.h>
#include <public/vmdirclient.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"

