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
 * Filename: vmacl.h
 *
 * Abstract:
 *
 * ACL api
 *
 */

#ifndef __VIDRACL_H__
#define __VIDRACL_H__

#ifdef __cplusplus
extern "C" {
#endif


#define VMDIR_LEADING_ORGANIZATION_O  "o="
#define VMDIR_LEADING_ORGANIZATION_DC "dc="

#define VMDIR_PARTS_ORGANIZATION_O   ",o="
#define VMDIR_PARTS_ORGANIZATION_DC  ",dc="

#define VMDIR_DEFAULT_SD_RELATIVE_SIZE     512

#define VMDIR_ACCESS_DENIED_ERROR_MSG "Insufficient access, access denied"
#define VMDIR_SD_CREATION_ERROR_MSG "Security descriptor creation failed"

/*
    Access Mask bits:

 // 4 bits - Generic Access Rights (given in request)
 // 2 bits - Unused
 // 2 bits - Special Access Rights
 // 3 bits - Unused
 // 5 bits - Standard Access Rights
 // 16 bits - Specific Access Rights

*/

// SJ-TBD: Following definitions should really be picked from lw/security-types.h

#define ADS_RIGHT_DELETE                    0x10000
#define ADS_RIGHT_READ_CONTROL              0x20000
#define ADS_RIGHT_WRITE_DAC                 0x40000
#define ADS_RIGHT_WRITE_OWNER               0x80000
#define ADS_RIGHT_SYNCHRONIZE               0x100000
#define ADS_RIGHT_ACCESS_SYSTEM_SECURITY    0x1000000
#define ADS_RIGHT_GENERIC_READ              0x80000000
#define ADS_RIGHT_GENERIC_WRITE             0x40000000
#define ADS_RIGHT_GENERIC_EXECUTE           0x20000000
#define ADS_RIGHT_GENERIC_ALL               0x10000000
#define ADS_RIGHT_DS_CREATE_CHILD           0x1
#define ADS_RIGHT_DS_DELETE_CHILD           0x2
#define ADS_RIGHT_ACTRL_DS_LIST             0x4
#define ADS_RIGHT_DS_SELF                   0x8
#define ADS_RIGHT_DS_READ_PROP              0x10
#define ADS_RIGHT_DS_WRITE_PROP             0x20
#define ADS_RIGHT_DS_DELETE_TREE            0x40
#define ADS_RIGHT_DS_LIST_OBJECT            0x80
#define ADS_RIGHT_DS_CONTROL_ACCESS         0x100

//
// Specific Access Rights for Directory Entry
//

// read access within the context of entry, in other words, its attributes
#define VMDIR_RIGHT_DS_READ_PROP    ADS_RIGHT_DS_READ_PROP

// write access within the context of entry, in other words, its attributes
#define VMDIR_RIGHT_DS_WRITE_PROP   ADS_RIGHT_DS_WRITE_PROP

// write access in order to add child entry in a container entry
#define VMDIR_RIGHT_DS_CREATE_CHILD ADS_RIGHT_DS_CREATE_CHILD

// write access in order to delete child entry in a container entry
#define VMDIR_RIGHT_DS_DELETE_CHILD ADS_RIGHT_DS_DELETE_CHILD

#define VMDIR_ENTRY_GENERIC_EXECUTE ADS_RIGHT_GENERIC_EXECUTE

#define VMDIR_ENTRY_ALL_ACCESS      (VMDIR_RIGHT_DS_READ_PROP | VMDIR_RIGHT_DS_WRITE_PROP | \
                                     VMDIR_RIGHT_DS_CREATE_CHILD | VMDIR_RIGHT_DS_DELETE_CHILD | \
                                     VMDIR_ENTRY_GENERIC_EXECUTE)

// Well-know Local SIDs (should be obsolete shortly)
#define VMDIR_DEFAULT_ADMIN_SID                         "S-1-7-32-500"
#define VMDIR_DEFAULT_BUILTIN_ADMINISTRATORS_GROUP_SID  "S-1-7-32-544"
#define VMDIR_DEFAULT_BUILTIN_USERS_GROUP_SID           "S-1-7-32-545"

#define VMDIR_EVERY_ONE_GROUP_SID "S-1-1-0"

// Well-known RIDs
#define VMDIR_DOMAIN_USER_RID_ADMIN      500 // Administrator user
#define VMDIR_DOMAIN_ALIAS_RID_ADMINS    544 // BUILTIN\Administrators group
#define VMDIR_DOMAIN_ALIAS_RID_USERS     545 // BUILTIN\Users group

// Rids higher than this '999' are not "well-known"
// For instance, regular users/groups)
#define VMDIR_DOMAIN_USER_RID_MAX   999

// Pre-defined GUID used to construct domainSid for host instance
// metadata before Replication can be setup
// For instance, (1) dc=com and (2) dc=vmwhost,dc=com
// (the host instance created by running vdcpromo the first time)
#define VMDIR_GUID_STRING_FOR_ROOT_DOMAIN  "00000000-0000-0001-8888-000000000000"

#define VMDIR_RID_STACK_SEPARATOR  ':'

typedef struct _VDIR_DOMAIN_SID_GEN_STATE
{
    PSTR                    pszDomainDn; // in order to search with a given DN
    PSTR                    pszDomainSid; // comparably to domainSid
    DWORD                   dwDomainRidSeqence;
    DWORD                   dwCount;

    LW_HASHTABLE_NODE    Node;

} VDIR_DOMAIN_SID_GEN_STATE, *PVDIR_DOMAIN_SID_GEN_STATE;

typedef struct _VDIR_SID_GEN_STATE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX        mutex;
    PLW_HASHTABLE    pHashtable;
    PVDIR_THREAD_INFO	pRIDSyncThr;
    PVMDIR_TSSTACK      pStack;
} VDIR_SID_GEN_STATE, *PVDIR_SID_GEN_STATE;

extern VDIR_SID_GEN_STATE gSidGenState;

// objectSid.c

DWORD
VmDirAdvanceDomainRID(
    DWORD   dwCnt
    );

DWORD
VmDirGenerateObjectSid(
    PVDIR_ENTRY pEntry,
    PSTR * ppszObjectSid
    );

DWORD
VmDirIsDomainObjectWithEntry(
    PVDIR_ENTRY pEntry,
    PBOOLEAN pbIsDomainObject
    );

DWORD
VmDirInternalRemoveOrgConfig(
    PVDIR_OPERATION pOperation,
    PSTR pszDomainDN
    );

PCSTR
VmDirFindDomainDN(
    PCSTR pszObjectDN
    );

DWORD
VmDirGenerateWellknownSid(
    PCSTR    pszDomainDN,
    DWORD    dwWellKnowRid,
    PSTR*    ppszAdminSid
    );

DWORD
VmDirFindDomainSidRid(
    PCSTR pszObjectSid,
    PSTR* ppszDomainSid,
    PDWORD pdwRid
    );

DWORD
VmDirGetSidGenStateIfDomain_inlock(
    PCSTR                       pszObjectDN,
    PSTR                        pszGuid, /* optional Guid used to generate sid */
    PVDIR_DOMAIN_SID_GEN_STATE* ppDomainState
    );

// libmain.c

DWORD
VmDirVmAclInit(
    VOID
    );

VOID
VmDirVmAclShutdown(
    VOID
    );

// acl.c

DWORD
VmDirSrvCreateAccessTokenWithEntry(
    PVDIR_ENTRY pEntry,
    PACCESS_TOKEN* ppToken,
    PSTR* ppszObjectSid /* Optional */
    );

DWORD
VmDirSrvAccessCheck(
    PVDIR_OPERATION pOperation, /* optional */
    PVDIR_ACCESS_INFO pAccessInfo,
    PVDIR_ENTRY pEntry,
    ACCESS_MASK AccessDesired
    );

VOID
VmDirAclCtxContentFree(
    PVDIR_ACL_CTX pAclCtx
    );

DWORD
VmDirSrvCreateDefaultSecDescRel(
    PSTR                           pszSystemAdministratorDn,
    PSTR                           pszAdminsGroupSid,
    PSECURITY_DESCRIPTOR_RELATIVE* ppSecDescRel,
    PULONG                         pulSecDescLength,
    PSECURITY_INFORMATION          pSecInfo
    );

DWORD
VmDirGetObjectSidFromDn(
    PSTR pszObjectDn,
    PSID* ppSid
    );

VOID
VmDirFreeAbsoluteSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppSecDesc
    );

DWORD
VmDirSrvAccessCheckIsAdminRole(
   PVDIR_OPERATION    pOperation, /* Optional */
   PCSTR              pszNormTargetDN, /* Mandatory */
   PVDIR_ACCESS_INFO  pAccessInfo, /* Mandatory */
   PBOOLEAN           pbIsAdminRole
   );

BOOLEAN
VmDirIsFailedAccessInfo(
    PVDIR_ACCESS_INFO pAccessInfo
    );

// security.c

DWORD
VmDirGetSecurityInformationFromSD(
    PSECURITY_DESCRIPTOR_RELATIVE pSecDesc,
    PSECURITY_INFORMATION pSecInfo
    );

DWORD
VmDirGetSecurityDescriptorForDn(
    PSTR                            pszObjectDn,
    SECURITY_INFORMATION            SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE*  ppSecDesc,
    PULONG                          pulSecDescLength
    );

DWORD
VmDirGetSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE* ppSecDesc,
    PULONG pulSecDescLength
    );

DWORD
VmDirSetSecurityDescriptorForDn(
    PSTR pszObjectDn,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel,
    ULONG ulSecDescRel
    );

DWORD
VmDirSetSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel,
    ULONG ulSecDescRel
    );

#ifdef __cplusplus
}
#endif

#endif /* __VIDRACL_H__ */
