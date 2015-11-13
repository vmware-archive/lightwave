/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *·
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/*
 * Module Name:  vmevent.h
 *
 * Abstract: VMware Cloud Directory Platform.
 *
 * Author: Kyoung Won Kwon (kkwon@vmware.com)
 */


#ifndef VMEVENT_H
#define VMEVENT_H


#ifdef __cplusplus
extern "C" {
#endif


#ifndef _WIN32
#include <lw/base.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


#ifndef VMEVENT_WSTRING_DEFINED
#define VMEVENT_WSTRING_DEFINED

typedef
#ifdef _DCE_IDL_
[ptr, string]
#endif
unsigned short int *  wstring_t;

#endif

#ifndef _EVENTLOG_ENTRY_DEFINED_
#define _EVENTLOG_ENTRY_DEFINED_

typedef struct _EVENTLOG_ENTRY
{
    unsigned int dwEventId;
    unsigned int dwEventType;
    wstring_t pszMessage;
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


#define EVENTLOG_ADD "EventLogAdd"
typedef DWORD (*PFEVENTLOG_ADD)(
        PCSTR pszServerName,
        DWORD dwEventID,
        DWORD dwEventType,
        PCSTR pszMessage
        );

#define EVENTLOG_INIT_ENUM_EVENTS_HANDLE "EventLogInitEnumEventsHandle"
typedef DWORD (*PFEVENTLOG_INIT_ENUM_EVENTS_HANDLE)(
        PCSTR pszServerName,
        PDWORD pdwHandle
        );

#define EVENTLOG_ENUM_EVENTS "EventLogEnumEvents"
typedef DWORD (*PFEVENTLOG_ENUM_EVENTS)(
        PCSTR pszServerName,
        DWORD dwHandle,
        DWORD dwStartIndex,
        DWORD dwNumPackages,
        PEVENTLOG_CONTAINER * ppEventContainer
        );


#ifndef VMAFD_NAME
#define VMAFD_NAME                  "vmafd"
#endif

#ifndef _WIN32

#ifndef VMAFD_CONFIG_KEY_ROOT
#define VMAFD_CONFIG_KEY_ROOT       "Services\\vmafd"
#endif
#ifndef VMAFD_REG_KEY_PATH
#define VMAFD_REG_KEY_PATH          "Path"
#endif
#define VMEVENT_CLIENT_LIBRARY      "/lib64/libvmeventclient.so"

#else

#ifndef VMAFD_CONFIG_KEY_ROOT
#define VMAFD_CONFIG_KEY_ROOT       ""
#endif
#ifndef VMAFD_REG_KEY_PATH
#define VMAFD_REG_KEY_PATH          ""
#endif
#define VMEVENT_CLIENT_LIBRARY      "\\libvmeventclient.dll"

#endif


#ifdef __cplusplus
}
#endif


#endif /* VMEVENT_H */
