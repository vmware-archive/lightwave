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
 * Module Name: VMEVENT
 *
 * Filename: vmeventtypes.h
 *
 * Abstract:
 *
 * Common types definition
 *
 */

#ifndef __VMEVENTTYPES_H__
#define __VMEVENTTYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#define VMEVENT_NCALRPC_END_POINT "vmafdsvc"
#else
#define VMEVENT_NCALRPC_END_POINT "VMWareAfdService"
#endif

#define VMEVENT_RPC_TCP_END_POINT "2020"

#ifdef _DCE_IDL_

cpp_quote("#include <vmeventtypes.h>")
cpp_quote("#if 0")

#endif

#ifndef VMEVENT_WSTRING_DEFINED
#define VMEVENT_WSTRING_DEFINED 1
typedef
#ifdef _DCE_IDL_
[ptr, string]
#endif
unsigned short int *  wstring_t;
#endif /* VMEVENT_WSTRING_DEFINED */

#ifndef _EVENTLOGLOG_LEVEL_DEFINED_
#define _EVENTLOGLOG_LEVEL_DEFINED_

typedef enum
{
    EVENTLOGLOG_LEVEL_ERROR = 0,
    EVENTLOGLOG_LEVEL_WARNING,
    EVENTLOGLOG_LEVEL_INFO,
    EVENTLOGLOG_LEVEL_VERBOSE,
    EVENTLOGLOG_LEVEL_DEBUG,
    EVENTLOGLOG_LEVEL_UNKNOWN
} EVENTLOGLOG_LEVEL;

#endif

#ifndef _EVENTLOG_ENTRY_DEFINED_
#define _EVENTLOG_ENTRY_DEFINED_

typedef struct _EVENTLOG_ENTRY
{
    unsigned  int dwEventId;
    unsigned int dwEventType;
    wstring_t    pszMessage;
} EVENTLOG_ENTRY, *PEVENTLOG_ENTRY;

#endif

#ifndef _EVENTLOG_CONTAINER_DEFINED_
#define _EVENTLOG_CONTAINER_DEFINED_


typedef struct _EVENTLOG_CONTAINER
{
   unsigned int dwCount;
   unsigned int dwTotalCount;
#ifdef _DCE_IDL_
    [size_is(dwCount)]
#endif /* _DCE_IDL_ */
   PEVENTLOG_ENTRY pPkgEntries;
} EVENTLOG_CONTAINER, *PEVENTLOG_CONTAINER;

#endif

#ifdef _DCE_IDL_
cpp_quote("#endif")
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VMEVENTTYPES_H__ */
