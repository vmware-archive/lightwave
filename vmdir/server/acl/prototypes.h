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
 * Module Name: Directory Schema
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

VOID
VmDirFreeOrgState(
    PVOID pOrgStat
    );

DWORD
VmDirSyncRIDSeqToDB(
    PCSTR   pszDomainDN,
    DWORD   dwRID
    );

// acl.c

DWORD
VmDirGetObjectSidFromDn(
    PCSTR pszObjectDn,
    PSID* ppSid
    );

DWORD
VmDirSrvCreateAccessTokenWithDn(
    PCSTR pszObjectDn,
    PACCESS_TOKEN* ppToken
    );

DWORD
VmDirCreateAccessToken(
    PACCESS_TOKEN*          AccessToken,
    PTOKEN_USER             User,
    PTOKEN_GROUPS           Groups,
    PTOKEN_PRIVILEGES       Privileges,
    PTOKEN_OWNER            Owner,
    PTOKEN_PRIMARY_GROUP    PrimaryGroup,
    PTOKEN_DEFAULT_DACL     DefaultDacl
    );

// legacy_checks.c
DWORD
VmDirLegacyAccessCheck(
    PVDIR_OPERATION pOperation,
    PVDIR_ACCESS_INFO pAccessInfo,
    PVDIR_ENTRY pEntry,
    ACCESS_MASK accessDesired
    );

BOOLEAN
VmDirIsLegacySecurityDescriptor(
    VOID
    );

// security.c

DWORD
VmDirSetSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel,
    ULONG ulSecDescRel
    );

DWORD
VmDirSecurityAclSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppAbsolute,
    PSECURITY_DESCRIPTOR_RELATIVE pRelative
    );

// objectSid.c

void
VmDirFindDomainRidSequenceWithDN(
    PCSTR pszDomainDN,
    PDWORD pRidSeq
    );

DWORD
VmDirGetSidGenStateIfDomain_inlock(
    PCSTR                       pszObjectDN,
    PCSTR                       pszGuid, /* optional Guid used to generate sid */
    PVDIR_DOMAIN_SID_GEN_STATE* ppDomainState
    );

// ridsyncthr.c
DWORD
VmDirInitRidSynchThr(
    PVDIR_THREAD_INFO* ppThrInfo
    );

#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

