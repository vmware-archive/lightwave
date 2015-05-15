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
 * Module Name: VDIR
 *
 * Filename: vmdirtypes.h
 *
 * Abstract:
 *
 * Common types definition
 *
 */

#ifndef __VDIR_TYPES_H__
#define __VDIR_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(WIN32_LEAN_AND_MEAN)
typedef     _Longlong             EntryId;
typedef     _Longlong             USN;
#else
typedef     int64_t               EntryId;
typedef     int64_t               USN;
#endif
////////////////////////////////////////////////////////////////////////////
// !!!!!!!!!!!!!!!!!!! IMPORTANT IMPORTANT IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!! READ READ READ READ READ READ !!!!!!!!!!!!!!!!!!!!!!!
// make sure ENTRYID and USN are same as db_seq_t(int64_t) in BDB db.h
////////////////////////////////////////////////////////////////////////////
typedef EntryId               ENTRYID;

#ifndef VMDIR_MAX_LDAP_URI_LEN
#define VMDIR_MAX_LDAP_URI_LEN 256
#endif

#ifndef VMDIR_MAX_DN_LEN
#define VMDIR_MAX_DN_LEN 1024
#endif

#ifndef VMDIR_MAX_UPN_LEN
#define VMDIR_MAX_UPN_LEN       512
#endif

typedef struct _VMDIR_REPL_PARTNER_INFO
{
    CHAR*       pszURI;
} VMDIR_REPL_PARTNER_INFO, *PVMDIR_REPL_PARTNER_INFO;

typedef struct _VMDIR_REPL_PARTNER_STATUS
{
    CHAR*       pszHost;
    BOOLEAN     bHostAvailable;
    BOOLEAN     bStatusAvailable;
    USN         targetUsn;
    USN         partnerUsn;
} VMDIR_REPL_PARTNER_STATUS, *PVMDIR_REPL_PARTNER_STATUS;

typedef struct _VMDIR_SERVER_INFO
{
    CHAR*       pszServerDN;
} VMDIR_SERVER_INFO, *PVMDIR_SERVER_INFO;

// opaque type PVMDIR_LOG_CTX
typedef struct _VMDIR_LOG_CTX* PVMDIR_LOG_CTX;

typedef struct _VMDIR_CONNECTION* PVMDIR_CONNECTION;

typedef struct _VMDIR_SERVER_CONTEXT VMDIR_SERVER_CONTEXT, *PVMDIR_SERVER_CONTEXT;


typedef enum
{
    VMDIRD_STATE_UNDEFINED = 0,
    VMDIRD_STATE_STARTUP,
    VMDIRD_STATE_READ_ONLY,   // Process only read/search requests. Originating and replication updates fail with UNWILLING_TO_PERFORM error
    VMDIRD_STATE_NORMAL,      // Process read-write requests. The normal mode
    VMDIRD_STATE_SHUTDOWN

} VDIR_SERVER_STATE;

#ifndef ENUM_VMDIR_LOG_LEVEL
#define ENUM_VMDIR_LOG_LEVEL
typedef enum
{
    VMDIR_LOG_ERROR = 0,
    VMDIR_LOG_WARNING,
    VMDIR_LOG_INFO,
    VMDIR_LOG_VERBOSE,
    VMDIR_LOG_DEBUG

} VMDIR_LOG_LEVEL;
#endif

#ifndef VMDIR_LOG_MASK_ALL
#define VMDIR_LOG_MASK_ALL -1
#endif

#ifndef VMDIR_USER_CREATE_PARAMS_A_DEFINED
#define VMDIR_USER_CREATE_PARAMS_A_DEFINED 1

typedef struct _VMDIR_USER_CREATE_PARAMS_A
{
    CHAR* pszName;
    CHAR* pszAccount;
    CHAR* pszUPN;
    CHAR* pszFirstname;
    CHAR* pszLastname;
    CHAR* pszPassword;

} VMDIR_USER_CREATE_PARAMS_A, *PVMDIR_USER_CREATE_PARAMS_A;

#endif /* VMDIR_USER_CREATE_PARAMS_A_DEFINED */

#ifndef VMDIR_USER_CREATE_PARAMS_W_DEFINED
#define VMDIR_USER_CREATE_PARAMS_W_DEFINED 1

typedef struct _VMDIR_USER_CREATE_PARAMS_W
{
    WCHAR* pwszName;
    WCHAR* pwszAccount;
    WCHAR* pwszUPN;
    WCHAR* pwszFirstname;
    WCHAR* pwszLastname;
    WCHAR* pwszPassword;

} VMDIR_USER_CREATE_PARAMS_W, *PVMDIR_USER_CREATE_PARAMS_W;

#endif /* VMDIR_USER_CREATE_PARAMS_W_DEFINED */

#ifndef VMDIR_USER_INFO_0_A_DEFINED
#define VMDIR_USER_INFO_0_A_DEFINED 1

typedef struct _VMDIR_USER_INFO_0_A
{
    CHAR* pszAccount;
    CHAR* pszFirstname;
    CHAR* pszLastname;
} VMDIR_USER_INFO_0_A, *PVMDIR_USER_INFO_0_A;

#endif /* VMDIR_USER_INFO_0_DEFINED */

#ifndef VMDIR_USER_INFO_1_A_DEFINED
#define VMDIR_USER_INFO_1_A_DEFINED 1

typedef struct _VMDIR_USER_INFO_1_A
{
    CHAR*   pszAccount;
    CHAR*   pszFirstname;
    CHAR*   pszLastname;
    CHAR*   pszUserSID;
    BOOLEAN bEnabled;
    BOOLEAN bPasswordExpired;
    LONG64  passwordExpiry;
} VMDIR_USER_INFO_1_A, *PVMDIR_USER_INFO_1_A;

#endif /* VMDIR_USER_INFO_1_DEFINED */

#ifdef __cplusplus
}
#endif

#endif /* __VDIR_TYPES_H__ */

