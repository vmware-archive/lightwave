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




#ifndef VMDIRAUTH_H_
#define VMDIRAUTH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vmdirtypes.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct _USERINFO
{
    ULONG uid;
    ULONG gid;
    PSTR  pszName;
    PSTR  pszFullName;
    PSTR  pszComment;
} USERINFO, *PUSERINFO;

typedef struct _GROUPINFO
{
    ULONG gid;
    PSTR  pszName;
    PSTR  pszComment;
} GROUPINFO, *PGROUPINFO;

typedef struct _AUTHCONTEXT* PAUTHCONTEXT;

DWORD
VmDirAuthFindUser(
    PCSTR      pszUsername,     /* IN     */
    PUSERINFO* ppUserInfo       /*    OUT */
    );

DWORD
VmDirAuthBeginEnumUsers(
    PAUTHCONTEXT* ppContext     /*    OUT */
    );

DWORD
VmDirAuthGetNextUser(
    PAUTHCONTEXT pContext,      /* IN     */
    PUSERINFO*   ppUserInfo     /*    OUT */
    );

VOID
VmDirAuthEndEnumUsers(
    PAUTHCONTEXT pContext       /* IN OUT */
    );

DWORD
VmDirAuthFindGroup(
    PCSTR       pszGroupname,   /* IN     */
    PGROUPINFO* ppGroupInfo     /*    OUT */
    );

DWORD
VmDirAuthBeginEnumGroups(
    PAUTHCONTEXT* ppContext     /*    OUT */
    );

DWORD
VmDirAuthGetNextGroup(
    PAUTHCONTEXT pContext,      /* IN     */
    PGROUPINFO*  ppGroupInfo    /*    OUT */
    );

VOID
VmDirAuthEndEnumGroups(
    PAUTHCONTEXT pContext       /* IN OUT */
    );

DWORD
VmDirAuthFindUsersInGroup(
    PCSTR       pszGroupname,   /* IN     */
    PUSERINFO*  ppUsers,        /*    OUT */
    PDWORD      pdwNumUsers     /* IN OUT */
    );

DWORD
VmDirAuthFindGroupsForUser(
    PCSTR       pszUsername,    /* IN     */
    PGROUPINFO* ppGroups,       /*    OUT */
    PDWORD      pdwNumGroups    /* IN OUT */
    );

DWORD
VmDirAuthCredentials(
    PCSTR pszUsername,          /* IN     */
    PCSTR pszPassword           /* IN     */
    );

VOID
VmDirAuthFreeUserInfoArray(
    PUSERINFO pUserInfoArray,   /* IN OUT */
    DWORD     dwNumUsers        /* IN     */
    );

VOID
VmDirAuthFreeUserInfo(
    PUSERINFO pUserInfo         /* IN OUT */
    );

VOID
VmDirAuthFreeGroupInfoArray(
    PGROUPINFO pGroupInfoArray, /* IN OUT */
    DWORD      dwNumGroups      /* IN     */
    );

VOID
VmDirAuthFreeGroupInfo(
    PGROUPINFO pGroupInfo       /* IN OUT */
    );

#endif /* VMDIRAUTH_H_ */



