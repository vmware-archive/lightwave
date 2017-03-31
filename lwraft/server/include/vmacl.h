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

#define VMDIR_ACCESS_DENIED_ERROR_MSG "Insufficient access, access denied"

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

// Well-known RIDs
#define VMDIR_DOMAIN_USER_RID_ADMIN      500 // Administrator user
#define VMDIR_DOMAIN_ALIAS_RID_ADMINS    544 // BUILTIN\Administrators group
#define VMDIR_DOMAIN_ALIAS_RID_USERS     545 // BUILTIN\Users group

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
VmDirGetSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE* ppSecDesc,
    PULONG pulSecDescLength
    );

DWORD
VmDirSetSecurityDescriptorForDn(
    PCSTR pszObjectDn,
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
