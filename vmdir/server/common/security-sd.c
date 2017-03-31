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



#include "includes.h"

DWORD
VmDirQuerySecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformationNeeded,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorInput,
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptorOutput,
    PULONG Length
    )
{
    return LwNtStatusToWin32Error(
            RtlQuerySecurityDescriptorInfo(SecurityInformationNeeded,
                                           SecurityDescriptorOutput,
                                           Length,
                                           SecurityDescriptorInput));
}

DWORD
VmDirSelfRelativeToAbsoluteSD(
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
    )
{
    return LwNtStatusToWin32Error(
            RtlSelfRelativeToAbsoluteSD(SelfRelativeSecurityDescriptor,
                                        AbsoluteSecurityDescriptor,
                                        AbsoluteSecurityDescriptorSize,
                                        pDacl,
                                        pDaclSize,
                                        pSacl,
                                        pSaclSize,
                                        Owner,
                                        pOwnerSize,
                                        PrimaryGroup,
                                        pPrimaryGroupSize));
}

BOOLEAN
VmDirAccessCheck(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PACCESS_TOKEN AccessToken,
    ACCESS_MASK DesiredAccess,
    ACCESS_MASK PreviouslyGrantedAccess,
    PGENERIC_MAPPING GenericMapping,
    PACCESS_MASK GrantedAccess,
    PDWORD pAccessError
    )
{
    BOOLEAN bRet = FALSE;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    bRet = RtlAccessCheck(SecurityDescriptor,
                          AccessToken,
                          DesiredAccess,
                          PreviouslyGrantedAccess,
                          GenericMapping,
                          GrantedAccess,
                          &ntStatus);

    *pAccessError = LwNtStatusToWin32Error(ntStatus);

    return bRet;
}

DWORD
VmDirGetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Owner,
    PBOOLEAN pIsOwnerDefaulted
    )
{
    return LwNtStatusToWin32Error(
            RtlGetOwnerSecurityDescriptor(SecurityDescriptor,
                                          Owner,
                                          pIsOwnerDefaulted));
}

DWORD
VmDirGetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID* Group,
    PBOOLEAN pIsGroupDefaulted
    )
{
    return LwNtStatusToWin32Error(
            RtlGetGroupSecurityDescriptor(SecurityDescriptor,
                                          Group,
                                          pIsGroupDefaulted));
}

DWORD
VmDirGetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsDaclPresent,
    PACL* Dacl,
    PBOOLEAN pIsDaclDefaulted
    )
{
    return LwNtStatusToWin32Error(
            RtlGetDaclSecurityDescriptor(SecurityDescriptor,
                                         pIsDaclPresent,
                                         Dacl,
                                         pIsDaclDefaulted));
}

DWORD
VmDirGetSaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PBOOLEAN pIsSaclPresent,
    PACL* Sacl,
    PBOOLEAN pIsSaclDefaulted
    )
{
    return LwNtStatusToWin32Error(
            RtlGetSaclSecurityDescriptor(SecurityDescriptor,
                                         pIsSaclPresent,
                                         Sacl,
                                         pIsSaclDefaulted));
}

BOOLEAN
VmDirValidRelativeSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    ULONG SecurityDescriptorLength,
    SECURITY_INFORMATION RequiredInformation
    )
{
    return RtlValidRelativeSecurityDescriptor(SecurityDescriptor,
                                              SecurityDescriptorLength,
                                              RequiredInformation);
}

DWORD
VmDirSetSecurityDescriptorInfo(
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE InputSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE ObjectSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE NewObjectSecurityDescriptor,
    PULONG NewObjectSecurityDescriptorLength,
    PGENERIC_MAPPING GenericMapping
    )
{
    return LwNtStatusToWin32Error(RtlSetSecurityDescriptorInfo(SecurityInformation,
                                        InputSecurityDescriptor,
                                        ObjectSecurityDescriptor,
                                        NewObjectSecurityDescriptor,
                                        NewObjectSecurityDescriptorLength,
                                        GenericMapping));
}

DWORD
VmDirCreateSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR_ABSOLUTE *ppSecurityDescriptor
    )
{
    DWORD dwError = 0;
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor = NULL;
    NTSTATUS Status = 0;

    dwError = VmDirAllocateMemory(
                SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE,
                (PVOID*)&SecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    Status = RtlCreateSecurityDescriptorAbsolute(
                SecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION);
    dwError = LwNtStatusToWin32Error(Status);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSecurityDescriptor = SecurityDescriptor;
    SecurityDescriptor = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(SecurityDescriptor);
    return dwError;
error:
    goto cleanup;
}

VOID
VmDirReleaseAccessToken(
    PACCESS_TOKEN* AccessToken
    )
{
    RtlReleaseAccessToken(AccessToken);
}

DWORD
VmDirSetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Owner,
    BOOLEAN IsOwnerDefaulted
    )
{
    return LwNtStatusToWin32Error(
             RtlSetOwnerSecurityDescriptor(
                                SecurityDescriptor,
                                Owner,
                                IsOwnerDefaulted));
}

DWORD
VmDirSetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Group,
    BOOLEAN IsGroupDefaulted
    )
{
    return LwNtStatusToWin32Error(
             RtlSetGroupSecurityDescriptor(SecurityDescriptor, Group, IsGroupDefaulted));
}

ULONG
VmDirLengthSid(
    PSID Sid
    )
{
    return RtlLengthSid(Sid);
}

DWORD
VmDirCreateAcl(
    PACL Acl,
    ULONG AclLength,
    ULONG AclRevision
    )
{
    return LwNtStatusToWin32Error(RtlCreateAcl(Acl, AclLength, AclRevision));
}

DWORD
VmDirGetAce(
    PACL pAcl,
    ULONG dwIndex,
    PACE_HEADER *ppAce
    )
{
    return LwNtStatusToWin32Error(RtlGetAce(pAcl, dwIndex, (PVOID*)ppAce));
}

DWORD
VmDirAddAccessAllowedAceEx(
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    )
{
    return LwNtStatusToWin32Error(RtlAddAccessAllowedAceEx(Acl,
                                                           AceRevision,
                                                           AceFlags,
                                                           AccessMask,
                                                           Sid));
}

DWORD
VmDirAddAccessDeniedAceEx(
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    )
{
    return LwNtStatusToWin32Error(RtlAddAccessDeniedAceEx(Acl,
                                                           AceRevision,
                                                           AceFlags,
                                                           AccessMask,
                                                           Sid));
}

DWORD
VmDirSetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    BOOLEAN IsDaclPresent,
    PACL Dacl,
    BOOLEAN IsDaclDefaulted
    )
{
    return LwNtStatusToWin32Error(
    RtlSetDaclSecurityDescriptor(SecurityDescriptor,
                                 IsDaclPresent,
                                 Dacl,
                                 IsDaclDefaulted));
}

BOOLEAN
VmDirValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor
    )
{
    return RtlValidSecurityDescriptor(SecurityDescriptor);
}

DWORD
VmDirAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE SelfRelativeSecurityDescriptor,
    PULONG BufferLength
    )
{
    return LwNtStatusToWin32Error(RtlAbsoluteToSelfRelativeSD(
                                  AbsoluteSecurityDescriptor,
                                  SelfRelativeSecurityDescriptor,
                                  BufferLength));
}

DWORD VmDirCreateWellKnownSid(
    WELL_KNOWN_SID_TYPE wellKnownSidType,
    PSID pDomainSid,
    PSID pSid,
    DWORD* pcbSid
)
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = LwNtStatusToWin32Error(
                    RtlCreateWellKnownSid(
                        wellKnownSidType,
                        pDomainSid,
                        pSid,
                        pcbSid));
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

VOID
VmDirMapGenericMask(
    PDWORD pdwAccessMask,
    PGENERIC_MAPPING pGenericMapping
)
{
    RtlMapGenericMask(pdwAccessMask, pGenericMapping);
}

DWORD
VmDirQueryAccessTokenInformation(
    HANDLE hTokenHandle,
    TOKEN_INFORMATION_CLASS tokenInformationClass,
    PVOID pTokenInformation,
    DWORD dwTokenInformationLength,
    PDWORD pdwReturnLength
)
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = LwNtStatusToWin32Error(
                       RtlQueryAccessTokenInformation(
                               hTokenHandle,
                               tokenInformationClass,
                               pTokenInformation,
                               dwTokenInformationLength,
                               pdwReturnLength));
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDirAllocateSddlCStringFromSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    DWORD dwRequestedStringSDRevision,
    SECURITY_INFORMATION securityInformation,
    PSTR* ppStringSecurityDescriptor
)
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = LwNtStatusToWin32Error(
                      RtlAllocateSddlCStringFromSecurityDescriptor(
                              ppStringSecurityDescriptor,
                              pSecurityDescriptor,
                              dwRequestedStringSDRevision,
                              securityInformation));
   BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmDirSetSecurityDescriptorControl(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    SECURITY_DESCRIPTOR_CONTROL BitsToChange,
    SECURITY_DESCRIPTOR_CONTROL BitsToSet
    )
{
    return LwNtStatusToWin32Error(
                RtlSetSecurityDescriptorControl(
                    pSecurityDescriptor,
                    BitsToChange,
                    BitsToSet));
}
