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

//
// Allows the client to delete this object, specifically (as opposed to
// VMDIR_RIGHT_DS_DELETE_CHILD which allows the client to delete any object
// underneath the object in question, but not the object itself).
//
#define VMDIR_RIGHT_DS_DELETE_OBJECT ADS_RIGHT_DELETE

#define VMDIR_ENTRY_GENERIC_EXECUTE ADS_RIGHT_GENERIC_EXECUTE

//
// These two permissions control a client being able to read or write the
// entry's security descriptor itself (the whole thing, not just the DACL,
// despite the name).
//
#define VMDIR_ENTRY_READ_ACL    ADS_RIGHT_READ_CONTROL
#define VMDIR_ENTRY_WRITE_ACL   ADS_RIGHT_WRITE_DAC

#define VMDIR_ENTRY_ALL_ACCESS  \
    (VMDIR_RIGHT_DS_READ_PROP |     \
     VMDIR_RIGHT_DS_WRITE_PROP |    \
     VMDIR_ENTRY_READ_ACL |         \
     VMDIR_ENTRY_WRITE_ACL |        \
     VMDIR_RIGHT_DS_CREATE_CHILD |  \
     VMDIR_RIGHT_DS_DELETE_CHILD |  \
     VMDIR_ENTRY_GENERIC_EXECUTE)

#define VMDIR_ENTRY_ALL_ACCESS_NO_DELETE_CHILD \
    (VMDIR_RIGHT_DS_READ_PROP |      \
     VMDIR_RIGHT_DS_WRITE_PROP |     \
     VMDIR_ENTRY_READ_ACL |          \
     VMDIR_ENTRY_WRITE_ACL |         \
     VMDIR_RIGHT_DS_CREATE_CHILD |   \
     VMDIR_ENTRY_GENERIC_EXECUTE)

#define VMDIR_ENTRY_ALL_ACCESS_NO_DELETE_CHILD_BUT_DELETE_OBJECT \
    (VMDIR_RIGHT_DS_READ_PROP |     \
     VMDIR_RIGHT_DS_WRITE_PROP |    \
     VMDIR_ENTRY_READ_ACL |         \
     VMDIR_ENTRY_WRITE_ACL |        \
     VMDIR_RIGHT_DS_CREATE_CHILD |  \
     VMDIR_ENTRY_GENERIC_EXECUTE |  \
     VMDIR_RIGHT_DS_DELETE_OBJECT)

//
// Members of the DCClients group get full access to entries under
// "cn=services,<domain>".
//
#define VMDIR_DCCLIENTS_FULL_ACCESS \
    (VMDIR_RIGHT_DS_READ_PROP |     \
     VMDIR_RIGHT_DS_WRITE_PROP |    \
     VMDIR_ENTRY_READ_ACL |         \
     VMDIR_ENTRY_WRITE_ACL |        \
     VMDIR_RIGHT_DS_CREATE_CHILD |  \
     VMDIR_RIGHT_DS_DELETE_CHILD)

// Well-known RIDs
#define VMDIR_DOMAIN_USER_RID_ADMIN      500 // Administrator user
#define VMDIR_DOMAIN_KRBTGT_RID          502 // Kerberos TGT user
#define VMDIR_DOMAIN_ADMINS_RID          512 // Domain Admins group
#define VMDIR_DOMAIN_CLIENTS_RID         515 // Domain Users group
#define VMDIR_DOMAIN_ALIAS_RID_ADMINS    544 // BUILTIN\Administrators group
#define VMDIR_DOMAIN_ALIAS_RID_USERS     545 // BUILTIN\Users group

//
// Well-known SID for a user who has connected anonymously.
//
#define VMDIR_ANONYMOUS_LOGON_SID "S-1-5-7"


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

DWORD
VmDirRegisterACLMode(
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
    PVDIR_OPERATION pOperation,
    PVDIR_ACCESS_INFO pAccessInfo,
    PVDIR_ENTRY pEntry,
    ACCESS_MASK AccessDesired
    );

VOID
VmDirAclCtxContentFree(
    PVDIR_ACL_CTX pAclCtx
    );

DWORD
VmDirSrvCreateSecurityDescriptor(
    ACCESS_MASK amAccess,
    PCSTR pszSystemAdministratorDn,
    PCSTR pszAdminsGroupSid,
    PCSTR pszDomainAdminsGroupSid,
    PCSTR pszDomainClientsGroupSid,
    PCSTR pszUsersGroupSid,
    BOOLEAN bProtectedDacl,
    BOOLEAN bAnonymousRead,
    BOOLEAN bServicesDacl,
    BOOLEAN bTenantDomain,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
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

DWORD
VmDirAddAceToSecurityDescriptor(
    PVDIR_ENTRY pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDesc,
    PCSTR pszAdminUserDn,
    ACCESS_MASK amAccess
    );

DWORD
VmDirGetObjectSidFromEntry(
    PVDIR_ENTRY pEntry,
    PSTR* ppszObjectSid, /* Optional */
    PSID* ppSid /* Optional */
    );

DWORD
VmDirIsBindDnMemberOfSystemDomainAdmins(
    PVDIR_BACKEND_CTX   pBECtx,
    PVDIR_ACCESS_INFO   pAccessInfo,
    PBOOLEAN            pbIsMemberOfAdmins
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
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
    );

DWORD
VmDirEntryCacheSecurityDescriptor(
    PVDIR_ENTRY pEntry,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRelToSet,
    ULONG ulSecDescToSetLen
    );

DWORD
VmDirSetRecursiveSecurityDescriptorForDn(
    PCSTR pszObjectDn,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
    );

DWORD
VmDirSetSecurityDescriptorForEntry(
    PVDIR_ENTRY pEntry,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel,
    ULONG ulSecDescRel
    );

// sdcalc.c
DWORD
VmDirComputeObjectSecurityDescriptor(
    PVDIR_ACCESS_INFO pAccessInfo,
    PVDIR_ENTRY pEntry,
    PVDIR_ENTRY pParentEntry
    );

// token.c
DWORD
VmDirSrvCreateAccessTokenForWellKnowObject(
    PACCESS_TOKEN *ppToken,
    PCSTR pszWellknownObjectSid
    );

VOID
VmDirSetACLMode(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif /* __VIDRACL_H__ */
