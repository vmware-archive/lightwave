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

#include <vmeventsys.h>

#include <locale.h>
#include <lwadvapi.h>
#include <lw/types.h>
#include <lw/base.h>
#include <lwio/io-types.h>
#include <compat/dcerpc.h>
#include <dce/rpc.h>
#include <vmeventcommon.h>
#include <vmevent_h.h>
#include <netdb.h>

#include "prototypes.h"
#include "defines.h"

#else // _WIN32


#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include "banned.h" // windows banned APIs
#include <dce/rpc.h>
#include <dce/dcethread.h>
#include <locale.h>

#include <vmeventsys.h>
#include <vmevent.h>
#include <vmeventcommon.h>
#include <vmeventclient.h>
#include <vmevent_h.h>

#include "defines.h"
#include "prototypes.h"

#endif // #ifndef _WIN32

#endif // _STDAFX_H_
