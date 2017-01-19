/*
 * Copyright (C) 2011-2016 VMware, Inc. All rights reserved.
 *
 * Module   : security-sd.c
 *
 * Abstract :
 *
 *            VMware AFD Service
 *
 *            Common Utilities (Client & Server)
 *
 *            Security Descriptor Management
 *
 * Authors  : Wei Fu (wfu@vmware.com)
 *
 *
 */

#include "includes.h"

DWORD
VmAfdQuerySecurityDescriptorInfo(
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
VmAfdAclAccessCheck(
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
VmAfdGetOwnerSecurityDescriptor(
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
VmAfdGetGroupSecurityDescriptor(
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
VmAfdGetDaclSecurityDescriptor(
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
VmAfdGetSaclSecurityDescriptor(
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
VmAfdValidRelativeSecurityDescriptor(
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
VmAfdSetSecurityDescriptorInfo(
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
VmAfdCreateSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    ULONG Revision
    )
{
    return LwNtStatusToWin32Error(
            RtlCreateSecurityDescriptorAbsolute(SecurityDescriptor, Revision));
}

VOID
VmAfdReleaseAccessToken(
    PACCESS_TOKEN* AccessToken
    )
{
    RtlReleaseAccessToken(AccessToken);
}

DWORD
VmAfdSetOwnerSecurityDescriptor(
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
VmAfdSetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor,
    PSID Group,
    BOOLEAN IsGroupDefaulted
    )
{
    return LwNtStatusToWin32Error(
             RtlSetGroupSecurityDescriptor(SecurityDescriptor, Group, IsGroupDefaulted));
}

ULONG
VmAfdLengthSid(
    PSID Sid
    )
{
    return RtlLengthSid(Sid);
}

DWORD
VmAfdCreateAcl(
    PACL Acl,
    ULONG AclLength,
    ULONG AclRevision
    )
{
    return LwNtStatusToWin32Error(RtlCreateAcl(Acl, AclLength, AclRevision));
}

DWORD
VmAfdAddAccessAllowedAceEx(
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
VmAfdSetDaclSecurityDescriptor(
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
VmAfdValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE SecurityDescriptor
    )
{
    return RtlValidSecurityDescriptor(SecurityDescriptor);
}

DWORD
VmAfdAbsoluteToSelfRelativeSD(
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

DWORD VmAfdCreateWellKnownSid(
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
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

VOID
VmAfdMapGenericMask(
    PDWORD pdwAccessMask,
    PGENERIC_MAPPING pGenericMapping
)
{
    RtlMapGenericMask(pdwAccessMask, pGenericMapping);
}

DWORD
VmAfdQueryAccessTokenInformation(
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
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfdAllocateSddlCStringFromSecurityDescriptor(
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
   BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}
