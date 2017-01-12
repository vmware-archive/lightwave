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

#ifndef _WIN32

DWORD
VmKdcQuerySecurityDescriptorInfo(
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
VmKdcSelfRelativeToAbsoluteSD(
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
VmKdcAccessCheck(
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
VmKdcGetOwnerSecurityDescriptor(
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
VmKdcGetGroupSecurityDescriptor(
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
VmKdcGetDaclSecurityDescriptor(
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
VmKdcGetSaclSecurityDescriptor(
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
VmKdcValidRelativeSecurityDescriptor(
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
VmKdcSetSecurityDescriptorInfo(
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
VmKdcCreateSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    ULONG Revision
    )
{
    return LwNtStatusToWin32Error(
            RtlCreateSecurityDescriptorAbsolute(SecurityDescriptor, Revision));
}

VOID
VmKdcReleaseAccessToken(
    PACCESS_TOKEN* AccessToken
    )
{
    RtlReleaseAccessToken(AccessToken);
}

DWORD
VmKdcSetOwnerSecurityDescriptor(
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
VmKdcSetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Group,
    BOOLEAN IsGroupDefaulted
    )
{
    return LwNtStatusToWin32Error(
             RtlSetGroupSecurityDescriptor(SecurityDescriptor, Group, IsGroupDefaulted));
}

ULONG
VmKdcLengthSid(
    PSID Sid
    )
{
    return RtlLengthSid(Sid);
}

DWORD
VmKdcCreateAcl(
    PACL Acl,
    ULONG AclLength,
    ULONG AclRevision
    )
{
    return LwNtStatusToWin32Error(RtlCreateAcl(Acl, AclLength, AclRevision));
}

DWORD
VmKdcAddAccessAllowedAceEx(
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
VmKdcSetDaclSecurityDescriptor(
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
VmKdcValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor
    )
{
    return RtlValidSecurityDescriptor(SecurityDescriptor);
}

DWORD
VmKdcAbsoluteToSelfRelativeSD(
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

DWORD VmKdcCreateWellKnownSid(
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
    BAIL_ON_VMKDC_ERROR(dwError);

error:

    return dwError;
}

VOID
VmKdcMapGenericMask(
    PDWORD pdwAccessMask,
    PGENERIC_MAPPING pGenericMapping
)
{
    RtlMapGenericMask(pdwAccessMask, pGenericMapping);
}

DWORD
VmKdcQueryAccessTokenInformation(
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
    BAIL_ON_VMKDC_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmKdcAllocateSddlCStringFromSecurityDescriptor(
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
   BAIL_ON_VMKDC_ERROR(dwError);

error:

    return dwError;
}

#endif

