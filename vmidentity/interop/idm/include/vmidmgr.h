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
 * Module Name:
 *
 *        vmidmgr.h
 *
 * Abstract:
 *
 *        Identity Manager - AD Integration
 *
 *        Public header 
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef _WIN32
#include <Windows.h>
#include <sspi.h>
#else

typedef struct _vmidm_cred_id_t *vmidm_cred_id_t;
typedef struct _vmidm_ctx_id_t *vmidm_ctx_id_t;

typedef vmidm_cred_id_t CredHandle, *PCredHandle;
typedef vmidm_ctx_id_t CtxtHandle, *PCtxtHandle;

#endif

typedef struct _IDM_USER_INFO_
{
    PWSTR pszUserName;
    DWORD dwNumGroups;
    PWSTR * ppszGroupNames;
    PWSTR * ppszSids;
    PWSTR pszUserSid;
} IDM_USER_INFO, *PIDM_USER_INFO;

#define AUTH_START_STATE    0
#define AUTH_START_CONTINUE 1
#define AUTH_START_COMPLETE 2

typedef struct _IDM_AUTH_CONTEXT_
{
    DWORD       dwState;
    CredHandle  hCredsHandle;
    PCredHandle phCredsHandle;
    CtxtHandle  hCtxtHandle;
    PCtxtHandle phCtxtHandle;
    DWORD       dwMaxTokenSize;
    BOOL        fNewConversation;
} IDM_AUTH_CONTEXT, *PIDM_AUTH_CONTEXT;

#ifndef _WIN32

DWORD
IDMInitialize(
    VOID
    );

#endif /* !_WIN32 */

DWORD
IDMGetComputerName(
    PWSTR* ppszName                      /*    OUT */
    );

DWORD
IDMAuthenticateUser(
    PWSTR           pszUserName,         /* IN     */
    PWSTR           pszDomainName,       /* IN     */
    PWSTR           pszPassword,         /* IN     */
    PIDM_USER_INFO* ppIdmUserInformation /*    OUT */
    );

DWORD
IDMCreateAuthContext(
    PWSTR              pszPackageName,   /* IN     */
    PIDM_AUTH_CONTEXT* ppAuthContext     /*    OUT */
    );

DWORD
IDMAuthenticate2(
    PIDM_AUTH_CONTEXT pAuthContext,        /* IN     */
    PBYTE             pInputBuffer,        /* IN     */
    DWORD             dwInputBufferSize,   /* IN     */
    PBYTE*            ppOutputBuffer,      /*    OUT */
    DWORD*            pdwOutputBufferSize, /* IN OUT */
    BOOL*             pfDone               /* IN OUT */
    );

DWORD
IDMGetUserInformationFromAuthContext(
    PIDM_AUTH_CONTEXT pAuthContext,        /* IN     */
    PIDM_USER_INFO*   ppIdmUserInformation /*    OUT */
    );

#ifndef _WIN32

DWORD
IDMLdapSaslBind(
    LDAP*       pLd,                       /* IN     */
    PWSTR       pszUser,                   /* IN     */
    PWSTR       pszDomain,                 /* IN     */
    PWSTR       pszPassword                /* IN     */
    );

#endif /* !_WIN32 */

VOID
IDMFreeAuthContext(
    PIDM_AUTH_CONTEXT pAuthContext         /* IN OUT OPTIONAL */
    );

VOID
IDMFreeUserInfo(
    PIDM_USER_INFO pIdmUserInformation     /* IN OUT OPTIONAL */
    );

DWORD
IDMAllocateMemory(
    SIZE_T size,                           /* IN     */
    PVOID* ppMemory                        /*    OUT */
    );

VOID
IDMFreeMemory(
    PVOID pMemory                          /* IN OUT OPTIONAL */
    );

#ifndef _WIN32

VOID
IDMShutdown(
    VOID
    );

#endif /* !_WIN32 */

#ifdef __cplusplus
}
#endif /* __cplusplus */
