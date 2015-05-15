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
 * Module Name: VMAFD
 *
 * Filename: vmauthsvctypes.h
 *
 * Abstract:
 *
 * Common types definition
 *
 */

#ifndef __VMAUTHSVCTYPES_H__
#define __VMAUTHSVCTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    #define wchar16_t wchar_t
#endif

#ifdef _DCE_IDL_

cpp_quote("#include <vmauthsvctypes.h>")
cpp_quote("#if 0")

#endif

#ifndef VMAUTHSVC_WSTRING_DEFINED
#define VMAUTHSVC_WSTRING_DEFINED 1
typedef
#ifdef _DCE_IDL_
[ptr, string]
#endif
unsigned short *wstring_t;   /* wchar16_t */
#endif /* VMAUTHSVC_WSTRING_DEFINED */

#ifndef VMAUTHSVC_STATUS_DEFINED
#define VMAUTHSVC_STATUS_DEFINED 1

typedef enum
{
	VMAUTHSVC_STATUS_UNKNOWN       = 0,
	VMAUTHSVC_STATUS_INITIALIZING,
	VMAUTHSVC_STATUS_PAUSED,
	VMAUTHSVC_STATUS_RUNNING,
	VMAUTHSVC_STATUS_STOPPING,
	VMAUTHSVC_STATUS_STOPPED
} VMAUTHSVC_STATUS, *PVMAUTHSVC_STATUS;

#endif /* VMAUTHSVC_STATUS_DEFINED */

#ifdef _DCE_IDL_
cpp_quote("#endif")
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VMAUTHSVCTYPES_H__ */
