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



#pragma once

DWORD
VmAfdOpenServerConnectionImpl(
	PVM_AFD_CONNECTION *ppConection
	);

VOID
VmAfdCloseServerConnectionImpl(
	PVM_AFD_CONNECTION pConnection
	);

DWORD
VmAfdOpenClientConnectionImpl(
	PVM_AFD_CONNECTION *ppConnection
	);

VOID
VmAfdCloseClientConnectionImpl(
	PVM_AFD_CONNECTION pConnection
	);

DWORD
VmAfdAcceptConnectionImpl(
	PVM_AFD_CONNECTION pConnection,
	PVM_AFD_CONNECTION *ppConnection
	);

DWORD
VmAfdReadDataImpl(
	PVM_AFD_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	);

DWORD
VmAfdWriteDataImpl(
	PVM_AFD_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwResponseSize
	);

VOID
VmAfdFreeConnectionImpl(
        PVM_AFD_CONNECTION pConnection
        );

DWORD
VmAfdInitializeConnectionContextImpl(
    PVM_AFD_CONNECTION pConnection,
    PVM_AFD_CONNECTION_CONTEXT *ppConnectionContext
    );

VOID
VmAfdFreeConnectionContextImpl(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

DWORD
VmAfdInitializeSecurityContextImpl(
        PVM_AFD_CONNECTION pConnection,
        PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );
VOID
VmAfdFreeSecurityContextImpl(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext
    );

DWORD
VmAfdEncodeSecurityContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PBYTE pByteBuffer,
    DWORD dwAvailableBuffSize,
    PDWORD pdwBuffUsed
    );

DWORD
VmAfdDecodeSecurityContextImpl (
    PBYTE pByteSecurityContext,
    DWORD dwBufSize,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmAfdGetSecurityContextSizeImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PDWORD pdwSize
    );

BOOL
VmAfdIsRootSecurityContextImpl (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext
    );

BOOL
VmAfdEqualsSecurityContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext1,
    PVM_AFD_SECURITY_CONTEXT pSecurityContext12
    );

DWORD
VmAfdAllocateContextFromNameImpl (
    PCWSTR pszAccountName,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

DWORD
VmAfdAllocateNameFromContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PWSTR *ppszAccountName
    );

DWORD
VmAfdCopySecurityContextImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContextSrc,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContextDst
    );


DWORD
VmAfdCreateAnonymousConnectionContextImpl (
    PVM_AFD_CONNECTION_CONTEXT *ppSecurityContext
    );

DWORD
VmAfdCreateWellKnownContextImpl (
    VM_AFD_CONTEXT_TYPE contextType,
    PVM_AFD_SECURITY_CONTEXT *ppSecurityContext
    );

BOOL
VmAfdContextBelongsToGroupImpl (
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PVM_AFD_SECURITY_CONTEXT pSecurityContextGroup
    );

DWORD
VmAfdCheckAclContextImpl(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PSTR pszSddlAcl,
    BOOL *pbIsAllowed
    );

DWORD
VmAfdGenRandomImpl(
    PDWORD pdwRandomNumber
    );

#ifndef _WIN32
/* acl.c */

DWORD
VmAfdCreateAccessTokenFromSecurityContext(
    PVM_AFD_SECURITY_CONTEXT pSecurityContext,
    PACCESS_TOKEN *ppAccessToken
    );

DWORD
VmAfdAllocateSecurityDescriptorFromSddlCString(
    PSECURITY_DESCRIPTOR_RELATIVE *ppSDRel,
    DWORD *pdwSDRelSize,
    PCSTR pszSddlAcl
    );

DWORD
VmAfdSecurityAclSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppAbsolute,
    PSECURITY_DESCRIPTOR_RELATIVE pRelative
    );

VOID
VmAfdFreeAbsoluteSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppSecDesc
    );

/* security-sd.c */

DWORD
VmAfdQuerySecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformationNeeded,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorInput,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorOutput,
    PULONG Length
    );

DWORD
VmAfdSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_RELATIVE SelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR_ABSOLUTE AbsoluteSecurityDescriptor,
    PULONG AbsoluteSecurityDescriptorSize,
    PACL pDacl,
    PULONG pDaclSize,
    PACL pSacl,
    PULONG pSaclSize,
    PSID Owner,
    PULONG pOwnerSize,
    PSID PrimaryGroup,
    PULONG pPrimaryGroupSize
    );

BOOLEAN
VmAfdAclAccessCheck(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PACCESS_TOKEN AccessToken,
    ACCESS_MASK DesiredAccess,
    ACCESS_MASK PreviouslyGrantedAccess,
    PGENERIC_MAPPING GenericMapping,
    PACCESS_MASK GrantedAccess,
    PDWORD pAccessError
    );

DWORD
VmAfdGetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Owner,
    PBOOLEAN pIsOwnerDefaulted
    );

DWORD
VmAfdGetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Group,
    PBOOLEAN pIsGroupDefaulted
    );

DWORD
VmAfdGetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsDaclPresent,
    PACL* Dacl,
    PBOOLEAN pIsDaclDefaulted
    );

DWORD
VmAfdGetSaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsSaclPresent,
    PACL* Sacl,
    PBOOLEAN pIsSaclDefaulted
    );

BOOLEAN
VmAfdValidRelativeSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    ULONG SecurityDescriptorLength,
    SECURITY_INFORMATION RequiredInformation
    );

DWORD
VmAfdSetSecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE InputSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE ObjectSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE NewObjectSecurityDescriptor,
    PULONG NewObjectSecurityDescriptorLength,
    PGENERIC_MAPPING GenericMapping
    );

DWORD
VmAfdCreateSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    ULONG Revision
    );

VOID
VmAfdReleaseAccessToken(
    PACCESS_TOKEN* AccessToken
    );

DWORD
VmAfdSetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Owner,
    BOOLEAN IsOwnerDefaulted
    );

DWORD
VmAfdSetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Group,
    BOOLEAN IsGroupDefaulted
    );

ULONG
VmAfdLengthSid(
    PSID Sid
    );

DWORD
VmAfdCreateAcl(
    PACL Acl,
    ULONG AclLength,
    ULONG AclRevision
    );

DWORD
VmAfdAddAccessAllowedAceEx(
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

DWORD
VmAfdSetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    BOOLEAN IsDaclPresent,
    PACL Dacl,
    BOOLEAN IsDaclDefaulted
    );

BOOLEAN
VmAfdValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor
    );

DWORD
VmAfdAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE SelfRelativeSecurityDescriptor,
    PULONG BufferLength
    );

DWORD VmAfdCreateWellKnownSid(
    WELL_KNOWN_SID_TYPE wellKnownSidType,
    PSID pDomainSid,
    PSID pSid,
    DWORD* pcbSid
    );

VOID
VmAfdMapGenericMask(
    PDWORD pdwAccessMask,
    PGENERIC_MAPPING pGenericMapping
    );

DWORD
VmAfdQueryAccessTokenInformation(
    HANDLE hTokenHandle,
    TOKEN_INFORMATION_CLASS tokenInformationClass,
    PVOID pTokenInformation,
    DWORD dwTokenInformationLength,
    PDWORD pdwReturnLength
    );

DWORD
VmAfdAllocateSddlCStringFromSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    DWORD dwRequestedStringSDRevision,
    SECURITY_INFORMATION securityInformation,
    PSTR* ppStringSecurityDescriptor
    );
#endif
