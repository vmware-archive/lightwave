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
 * Module Name: vmevent db
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * event module include file
 *
 */

#ifndef _WIN32/* ============= LINUX ONLY ================ */
#include <config.h>
#include <vmeventsys.h>

#include <lwadvapi.h>

#include <sqlite3.h>

#include <vmevent.h>
#include <vmeventtypes.h>
#include <vmeventcommon.h>

#include <vmeventdb.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
#include "dbcontext.h"

#else
/* ========================= WIN32 ONLY ======================== */

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>
#include <errno.h>
#include <assert.h>
#include <sqlite3.h>

#include <pthread.h>
#include <openssl/x509.h>

#include <vmevent.h>
#include <vmeventtypes.h>
#include <vmeventcommon.h>
#include <vmeventdb.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
#include "dbcontext.h"
#endif
