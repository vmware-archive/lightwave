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

#ifdef _WIN32
#pragma once

// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run
// your application.  The macros work by enabling all features available on platform versions up to and
// including the version specified.

// #define WIN2008

#ifdef WIN2008

#   ifndef NTDDI_VERSION
#   define NTDDI_VERSION NTDDI_WS08
#   endif

#   ifndef _WIN32_WINNT
#   define _WIN32_WINNT _WIN32_WINNT_WS08
#   endif

#   ifndef WINVER
#   define WINVER _WIN32_WINNT
#   endif

#   ifndef _WIN32_IE
#   define _WIN32_IE _WIN32_IE_WS08
#   endif

#else

#   ifndef NTDDI_VERSION
#   define NTDDI_VERSION NTDDI_WS03SP2
#   endif

#   ifndef _WIN32_WINNT
#   define _WIN32_WINNT _WIN32_WINNT_WS03
#   endif

#   ifndef WINVER
#   define WINVER _WIN32_WINNT
#   endif

#   ifndef _WIN32_IE
#   define _WIN32_IE _WIN32_IE_WS03
#   endif

#endif

#include <sdkddkver.h>

#endif
