/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#include <vmcasys.h>

#include <sqlite3.h>

#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include <lw/types.h>
#include <lw/base.h>

#include <vmcatypes.h>
#include <vmcacommon.h>
#include <vmcadb.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#else
#pragma once
#include "targetver.h"
//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <assert.h>
#include <pthread.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include "banned.h"

#include <macros.h>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmcasys.h>
#include <vmcacommon.h>
#include <vmcadb.h>

#include "sqlite3.h"
#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
#endif
