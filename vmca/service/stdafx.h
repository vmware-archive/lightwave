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

#ifndef _STDAFX_H_
#define _STDAFX_H_

#ifndef _WIN32

#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <wc16str.h>
#include <compat/dcerpc.h>
#include <ldap.h>
#include <vmca_h.h>
#else
#include <windows.h>
#include <tchar.h>
#include <direct.h> // for _mkdir
#if defined(_DEBUG)
#include "../x64/Debug/vmca_h.h"
#else
#include "../x64/Release/vmca_h.h"
#endif
#endif

#include <dce/dcethread.h>
#include <dce/rpc.h>
#include <vmcasys.h>
#include <vmcacommon.h>
#include <vmcadb.h>
#include <vmca_error.h>
#include "defines.h"
#include "errormap.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"


#endif // _STDAFX_H_
