/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

#define STATUS_BUFFER_TOO_SMALL          (0xC0000023L)

static
VOID
VmKdcFreeAbsoluteSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecDescAbs
);

static
DWORD
VmKdcCreateAbsSecDescFromRelative(
    PSECURITY_DESCRIPTOR_RELATIVE pRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR_ABSOLUTE* ppAbsoluteSecurityDescriptor
);

DWORD
VmKdcQuerySecurityDescriptorInfo(
    SECURITY_INFORMATION securityInformationNeeded,
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptorInput,
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptorOutput,
    PULONG pLength
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecDescAbs = NULL;

    if ( (securityInformationNeeded == 0)
         || (pSecurityDescriptorInput == NULL)
         || (pSecurityDescriptorOutput == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError =
        VmKdcCreateAbsSecDescFromRelative(
            pSecurityDescriptorInput, &pSecDescAbs
        );
    BAIL_ON_VMKDC_ERROR(dwError);

    // Remove any pieces not requested
    if ( (securityInformationNeeded & OWNER_SECURITY_INFORMATION)
         !=
         OWNER_SECURITY_INFORMATION
       )
    {
        PSID pOwnerSid = NULL;
        BOOLEAN bOwnerDefaulted = FALSE;

        if( GetSecurityDescriptorOwner(
                pSecDescAbs, &pOwnerSid, &bOwnerDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pOwnerSid);

        if( SetSecurityDescriptorOwner( pSecDescAbs, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    if ( (securityInformationNeeded & GROUP_SECURITY_INFORMATION)
         !=
         GROUP_SECURITY_INFORMATION
       )
    {
        PSID pGroupSid = NULL;
        BOOLEAN bGroupDefaulted = FALSE;

        if( GetSecurityDescriptorGroup(
                pSecDescAbs, &pGroupSid, &bGroupDefaulted) == 0)
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pGroupSid);

        if( SetSecurityDescriptorGroup(pSecDescAbs, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    if ( (securityInformationNeeded & DACL_SECURITY_INFORMATION)
         !=
         DACL_SECURITY_INFORMATION
       )
    {
        PACL pDacl = NULL;
        BOOLEAN bDaclDefaulted = FALSE;
        BOOLEAN bDaclPresent = FALSE;
        SECURITY_DESCRIPTOR_CONTROL DaclControlChange =
            (SE_DACL_PROTECTED|
             SE_DACL_AUTO_INHERITED|
             SE_DACL_AUTO_INHERIT_REQ);

        if( GetSecurityDescriptorDacl(
                     pSecDescAbs,
                     &bDaclPresent,
                     &pDacl,
                     &bDaclDefaulted) == 0
          )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pDacl);

        if( SetSecurityDescriptorDacl(pSecDescAbs, FALSE, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if( SetSecurityDescriptorControl(
                pSecDescAbs, DaclControlChange, 0) == 0)
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    if ( (securityInformationNeeded & SACL_SECURITY_INFORMATION)
         !=
         SACL_SECURITY_INFORMATION
       )
    {
        PACL pSacl = NULL;
        BOOLEAN bSaclDefaulted = FALSE;
        BOOLEAN bSaclPresent = FALSE;
        SECURITY_DESCRIPTOR_CONTROL SaclControlChange =
            (SE_SACL_PROTECTED|
             SE_SACL_AUTO_INHERITED|
             SE_SACL_AUTO_INHERIT_REQ);

        if( GetSecurityDescriptorSacl(
                     pSecDescAbs,
                     &bSaclPresent,
                     &pSacl,
                     &bSaclDefaulted) == 0
          )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pSacl);

        if( SetSecurityDescriptorSacl(pSecDescAbs, FALSE, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if( SetSecurityDescriptorControl(
                pSecDescAbs, SaclControlChange, 0) == 0)
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    // Convert back to relative form
    if( MakeSelfRelativeSD(
            pSecDescAbs, pSecurityDescriptorOutput, pLength) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    if (pSecDescAbs)
    {
        VmKdcFreeAbsoluteSecurityDescriptor(pSecDescAbs);
        pSecDescAbs = NULL;
    }

    return dwError;
}

DWORD
VmKdcSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR_RELATIVE pSelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR_ABSOLUTE pAbsoluteSecurityDescriptor,
    PULONG pAbsoluteSecurityDescriptorSize,
    PACL pDacl,
    PULONG pDaclSize,
    PACL pSacl,
    PULONG pSaclSize,
    PSID pOwner,
    PULONG pOwnerSize,
    PSID pPrimaryGroup,
    PULONG pPrimaryGroupSize
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if ( MakeAbsoluteSD(pSelfRelativeSecurityDescriptor,
                        pAbsoluteSecurityDescriptor,
                        pAbsoluteSecurityDescriptorSize,
                        pDacl,
                        pDaclSize,
                        pSacl,
                        pSaclSize,
                        pOwner,
                        pOwnerSize,
                        pPrimaryGroup,
                        pPrimaryGroupSize
                        ) == 0)
    {
        dwError = GetLastError();
        if(dwError == STATUS_BUFFER_TOO_SMALL)
        {
            dwError = ERROR_INSUFFICIENT_BUFFER;
        }
    }

    return dwError;
}

BOOLEAN
VmKdcAccessCheck(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    PACCESS_TOKEN pAccessToken,
    ACCESS_MASK desiredAccess,
    ACCESS_MASK previouslyGrantedAccess,
    PGENERIC_MAPPING pGenericMapping,
    PACCESS_MASK pGrantedAccess,
    PDWORD pAccessError
    )
{
    DWORD PrivilegeSetLength = 0;
    // TODO: pgu
    // Double check the accessStatus should be initialized to TRUE or FALSE.
    BOOL accessStatus = TRUE; 

    /*
    MSDN:
    AccessStatus [out]
        A pointer to a variable that receives the results of the access check.
        If the security descriptor allows the requested access rights to the
        client identified by the access token, AccessStatus is set to TRUE.
        Otherwise, AccessStatus is set to FALSE, and you can call
        GetLastError to get extended error information.
    */
    //TODO: pgu
    // remove the following line: temporary workaround
    desiredAccess = GENERIC_WRITE;

    if( AccessCheck(
            pSecurityDescriptor,
            pAccessToken,
            desiredAccess,
            pGenericMapping,
            NULL,
            &PrivilegeSetLength,
            pGrantedAccess,
            &accessStatus
       ) == 0 )
    {
        // function failed  - so we will assume FALSE
        accessStatus = FALSE;

        // TODO: pgu
        // Remove the following line
        accessStatus = TRUE;
    }

    if ( (accessStatus == FALSE) && (pAccessError != NULL) )
    {
        *pAccessError = GetLastError();
    }

    return accessStatus;
}

DWORD
VmKdcGetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    PSID* ppOwner,
    PBOOLEAN pIsOwnerDefaulted
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if( GetSecurityDescriptorOwner(
            pSecurityDescriptor, ppOwner, pIsOwnerDefaulted) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
}

DWORD
VmKdcGetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    PSID* ppGroup,
    PBOOLEAN pIsGroupDefaulted
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if( GetSecurityDescriptorGroup(
            pSecurityDescriptor, ppGroup, pIsGroupDefaulted) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
}

DWORD
VmKdcGetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    PBOOLEAN pIsDaclPresent,
    PACL* ppDacl,
    PBOOLEAN pIsDaclDefaulted
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if( GetSecurityDescriptorDacl(
            pSecurityDescriptor, pIsDaclPresent,
            ppDacl, pIsDaclDefaulted) == 0 )
    {
        dwError = GetLastError();
    }

    return dwError;
}

DWORD
VmKdcGetSaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    PBOOLEAN pIsSaclPresent,
    PACL* ppSacl,
    PBOOLEAN pIsSaclDefaulted
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if( GetSecurityDescriptorSacl(
            pSecurityDescriptor, pIsSaclPresent,
            ppSacl, pIsSaclDefaulted) == 0 )
    {
        dwError = GetLastError();
    }

    return dwError;
}

BOOLEAN
VmKdcValidRelativeSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    ULONG securityDescriptorLength,
    SECURITY_INFORMATION requiredInformation
    )
{
    /*
    MSDN:
    The IsValidSecurityDescriptor function determines whether
    the components of a security descriptor are valid.

    The IsValidSecurityDescriptor function checks the validity of
    the components that are present in the security descriptor.
    It does not verify whether certain components are present.
    */
    BOOLEAN isValid = IsValidSecurityDescriptor(pSecurityDescriptor);

    // check relative, and expected components
    if(isValid != FALSE)
    {
        SECURITY_DESCRIPTOR_CONTROL control = 0;
        DWORD revision = 0;
        if( GetSecurityDescriptorControl(
                pSecurityDescriptor, &control, &revision
            )
            == 0
          )
        {
            isValid = FALSE;
        }
        else
        {
            isValid = isValid &&
                ( ( control & SE_SELF_RELATIVE) == SE_SELF_RELATIVE );

            if ( (isValid)
                 &&
                 ( ( requiredInformation & DACL_SECURITY_INFORMATION)
                   ==
                   DACL_SECURITY_INFORMATION
                 ) )
            {
                isValid = isValid &&
                    ( ( control & SE_DACL_PRESENT) == SE_DACL_PRESENT );
            }

            if ( (isValid)
                 &&
                 ( ( requiredInformation & SACL_SECURITY_INFORMATION)
                   == SACL_SECURITY_INFORMATION
                 ) )
            {
                isValid = isValid &&
                    ( ( control & SE_SACL_PRESENT) == SE_SACL_PRESENT );
            }

            if ( (isValid)
                 &&
                 ( ( requiredInformation & OWNER_SECURITY_INFORMATION)
                   == OWNER_SECURITY_INFORMATION
                 ) )
            {
                PSID owner = NULL;
                BOOLEAN ownerDefaulted = FALSE;

                if( GetSecurityDescriptorOwner(
                        pSecurityDescriptor, &owner, &ownerDefaulted ) == 0 )
                {
                    isValid = FALSE;
                }
                else
                {
                    isValid = isValid && ( owner != NULL );
                }
            }

            if ( (isValid)
                  &&
                  ( ( requiredInformation & GROUP_SECURITY_INFORMATION)
                    == GROUP_SECURITY_INFORMATION
                  ) )
            {
                PSID group = NULL;
                BOOLEAN groupDefaulted = FALSE;

                if( GetSecurityDescriptorGroup(
                       pSecurityDescriptor, &group, &groupDefaulted ) == 0 )
                {
                    isValid = FALSE;
                }
                else
                {
                    isValid = isValid && ( group != NULL );
                }
            }
        }

    }

    return isValid;
}

DWORD
VmKdcSetSecurityDescriptorInfo(
    SECURITY_INFORMATION securityInformation,
    PSECURITY_DESCRIPTOR_RELATIVE pInputSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE pObjectSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE pNewObjectSecurityDescriptor,
    PULONG pNewObjectSecurityDescriptorLength,
    PGENERIC_MAPPING pGenericMapping
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR_ABSOLUTE pObjSecDescAbs = NULL;
    PSECURITY_DESCRIPTOR_ABSOLUTE pInputSecDescAbs = NULL;
    SECURITY_DESCRIPTOR_CONTROL InputSecDescControl = 0;
    DWORD Revision = 0;

    if ( (securityInformation == 0) || (pInputSecurityDescriptor == NULL)
        ||
        (pObjectSecurityDescriptor == NULL)
        ||
        (pNewObjectSecurityDescriptor == NULL)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    // Convert to an Absolute SecDesc
    dwError =
        VmKdcCreateAbsSecDescFromRelative(
            pObjectSecurityDescriptor, &pObjSecDescAbs
        );
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError =
        VmKdcCreateAbsSecDescFromRelative(
            pInputSecurityDescriptor, &pInputSecDescAbs
        );
    BAIL_ON_VMKDC_ERROR(dwError);

    // Merge

    if( GetSecurityDescriptorControl(
            pInputSecDescAbs, &InputSecDescControl, &Revision) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    // Owner
    if ( (securityInformation & OWNER_SECURITY_INFORMATION)
         ==
         OWNER_SECURITY_INFORMATION
       )
    {
        PSID pObjOwnerSid = NULL;
        BOOLEAN bObjOwnerDefaulted = FALSE;
        PSID pInputOwnerSid = NULL;
        BOOLEAN bInputOwnerDefaulted = FALSE;

        if ( GetSecurityDescriptorOwner(
                pObjSecDescAbs, &pObjOwnerSid, &bObjOwnerDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pObjOwnerSid);

        if ( GetSecurityDescriptorOwner(
                pInputSecDescAbs, &pInputOwnerSid, &bInputOwnerDefaulted
             )
             == 0
           )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if( SetSecurityDescriptorOwner( pInputSecDescAbs, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorOwner(
                pObjSecDescAbs, pInputOwnerSid, bInputOwnerDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    // Group
    if ( (securityInformation & GROUP_SECURITY_INFORMATION)
         ==
         GROUP_SECURITY_INFORMATION
       )
    {
        PSID pObjGroupSid = NULL;
        BOOLEAN bObjGroupDefaulted = FALSE;
        PSID pInputGroupSid = NULL;
        BOOLEAN bInputGroupDefaulted = FALSE;

        if ( GetSecurityDescriptorGroup(
                pObjSecDescAbs, &pObjGroupSid, &bObjGroupDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pObjGroupSid);

        if ( GetSecurityDescriptorGroup(
                pInputSecDescAbs, &pInputGroupSid, &bInputGroupDefaulted
             )
             == 0
           )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorGroup( pInputSecDescAbs, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorGroup(
                pObjSecDescAbs, pInputGroupSid, bInputGroupDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    // Dacl
    if ( (securityInformation & DACL_SECURITY_INFORMATION)
          ==
          DACL_SECURITY_INFORMATION
       )
    {
        PACL pObjDacl = NULL;
        BOOLEAN bObjDaclDefaulted = FALSE;
        BOOLEAN bObjDaclPresent = FALSE;
        PACL pInputDacl = NULL;
        BOOLEAN bInputDaclDefaulted = FALSE;
        BOOLEAN bInputDaclPresent = FALSE;
        SECURITY_DESCRIPTOR_CONTROL DaclControlSet = 0;
        SECURITY_DESCRIPTOR_CONTROL DaclControlChange =
            (SE_DACL_PROTECTED|
             SE_DACL_AUTO_INHERITED|
             SE_DACL_AUTO_INHERIT_REQ);

        if ( GetSecurityDescriptorDacl(
                pObjSecDescAbs, &bObjDaclPresent, &pObjDacl, &bObjDaclDefaulted
             )
             == 0
           )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pObjDacl);

        if ( GetSecurityDescriptorDacl(
                pInputSecDescAbs, &bInputDaclPresent,
                &pInputDacl, &bInputDaclDefaulted
             )
             == 0
           )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorDacl(
                 pInputSecDescAbs, FALSE, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorControl(
                pObjSecDescAbs, DaclControlChange, 0) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorDacl(
                pObjSecDescAbs, TRUE, pInputDacl, bInputDaclDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        DaclControlSet = InputSecDescControl & DaclControlChange;

        if ( SetSecurityDescriptorControl(
                pObjSecDescAbs, DaclControlChange, DaclControlSet) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    // Sacl
    if ( (securityInformation & SACL_SECURITY_INFORMATION)
         ==
         SACL_SECURITY_INFORMATION
       )
    {
        PACL pObjSacl = NULL;
        BOOLEAN bObjSaclDefaulted = FALSE;
        BOOLEAN bObjSaclPresent = FALSE;
        PACL pInputSacl = NULL;
        BOOLEAN bInputSaclDefaulted = FALSE;
        BOOLEAN bInputSaclPresent = FALSE;
        SECURITY_DESCRIPTOR_CONTROL SaclControlSet = 0;
        SECURITY_DESCRIPTOR_CONTROL SaclControlChange =
            (SE_SACL_PROTECTED|
             SE_SACL_AUTO_INHERITED|
             SE_SACL_AUTO_INHERIT_REQ);

        if( GetSecurityDescriptorSacl(
                pObjSecDescAbs, &bObjSaclPresent,
                &pObjSacl, &bObjSaclDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        VMKDC_SAFE_FREE_MEMORY(pObjSacl);

        if ( GetSecurityDescriptorSacl(
                pInputSecDescAbs, &bInputSaclPresent,
                &pInputSacl, &bInputSaclDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorSacl(
                pInputSecDescAbs, FALSE, NULL, FALSE) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorControl(
                pObjSecDescAbs, SaclControlChange, 0) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        if ( SetSecurityDescriptorSacl(
                pObjSecDescAbs, TRUE, pInputSacl, bInputSaclDefaulted) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }

        SaclControlSet = InputSecDescControl & SaclControlChange;

        if ( SetSecurityDescriptorControl(
                pObjSecDescAbs, SaclControlChange, SaclControlSet) == 0 )
        {
            dwError = GetLastError();
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    // Convert back to relative form
    if ( MakeSelfRelativeSD(
            pObjSecDescAbs, pNewObjectSecurityDescriptor,
            pNewObjectSecurityDescriptorLength) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    if (pObjSecDescAbs)
    {
        VmKdcFreeAbsoluteSecurityDescriptor(pObjSecDescAbs);
        pObjSecDescAbs = NULL;
    }

    if (pInputSecDescAbs)
    {
        VmKdcFreeAbsoluteSecurityDescriptor(pInputSecDescAbs);
        pInputSecDescAbs = NULL;
    }

    return dwError;
}

DWORD
VmKdcCreateSecurityDescriptorAbsolute(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    ULONG revision
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if ( InitializeSecurityDescriptor( pSecurityDescriptor, revision ) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
}

VOID
VmKdcReleaseAccessToken(
    PACCESS_TOKEN* ppAccessToken
    )
{
    if ( (ppAccessToken != NULL) && ((*ppAccessToken) != NULL) )
    {
        CloseHandle(*ppAccessToken);
        *ppAccessToken = NULL;
    }
}

DWORD
VmKdcSetOwnerSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    PSID pOwner,
    BOOLEAN isOwnerDefaulted
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if ( SetSecurityDescriptorOwner(
             pSecurityDescriptor,
             pOwner,
             isOwnerDefaulted) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
}

DWORD
VmKdcSetGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    PSID pGroup,
    BOOLEAN isGroupDefaulted
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if ( SetSecurityDescriptorGroup(
             pSecurityDescriptor,
             pGroup,
             isGroupDefaulted) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
}

ULONG
VmKdcLengthSid(
    PSID pSid
    )
{
    return GetLengthSid( pSid );
}

DWORD
VmKdcCreateAcl(
    PACL pAcl,
    ULONG aclLength,
    ULONG aclRevision
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if( InitializeAcl( pAcl, aclLength, aclRevision ) == 0 )
    {
        dwError = GetLastError();
    }

    return dwError;
}

DWORD
VmKdcAddAccessAllowedAceEx(
    PACL pAcl,
    ULONG aceRevision,
    ULONG aceFlags,
    ACCESS_MASK accessMask,
    PSID pSid
    )
{
    DWORD dwError = ERROR_SUCCESS;

    if( AddAccessAllowedAceEx(
            pAcl, aceRevision, aceFlags, accessMask, pSid) == 0 )
    {
        dwError = GetLastError();
    }

    return dwError;
}

DWORD
VmKdcSetDaclSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor,
    BOOLEAN isDaclPresent,
    PACL pDacl,
    BOOLEAN isDaclDefaulted
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if ( SetSecurityDescriptorDacl(
             pSecurityDescriptor,
             isDaclPresent,
             pDacl,
             isDaclDefaulted) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
}

BOOLEAN
VmKdcValidSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecurityDescriptor
    )
{
    return IsValidSecurityDescriptor( pSecurityDescriptor );
}

DWORD
VmKdcAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR_ABSOLUTE pAbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR_RELATIVE pSelfRelativeSecurityDescriptor,
    PULONG pBufferLength
    )
{
    DWORD dwError = ERROR_SUCCESS;
    if ( MakeSelfRelativeSD(
             pAbsoluteSecurityDescriptor,
             pSelfRelativeSecurityDescriptor,
             pBufferLength ) == 0 )
    {
        dwError = GetLastError();
    }
    return dwError;
}

DWORD VmKdcCreateWellKnownSid(
    WELL_KNOWN_SID_TYPE wellKnownSidType,
    PSID pDomainSid,
    PSID pSid,
    DWORD* pcbSid
)
{
    DWORD dwError = ERROR_SUCCESS;

    if( CreateWellKnownSid( wellKnownSidType, pDomainSid, pSid, pcbSid ) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:

    return dwError;
}

VOID
VmKdcMapGenericMask(
    PDWORD pdwAccessMask,
    PGENERIC_MAPPING pGenericMapping
)
{
    MapGenericMask( pdwAccessMask, pGenericMapping);
}

DWORD
VmKdcQueryAccessTokenInformation(
    HANDLE hTokenHandle,
    TOKEN_INFORMATION_CLASS tokenInformationClass,
    LPVOID pTokenInformation,
    DWORD dwTokenInformationLength,
    PDWORD pdwReturnLength
)
{
    DWORD dwError = ERROR_SUCCESS;

    if ( GetTokenInformation(
             hTokenHandle, tokenInformationClass,
             pTokenInformation, dwTokenInformationLength,
             pdwReturnLength ) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:

    return dwError;
}

DWORD
VmKdcAllocateSddlCStringFromSecurityDescriptor(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    DWORD dwRequestedStringSDRevision,
    SECURITY_INFORMATION securityInformation,
    LPSTR* ppStringSecurityDescriptor
)
{
    DWORD dwError = ERROR_SUCCESS;
    LPSTR pSecDescriptorStr = NULL;
    PSTR  pStrToReturn = NULL;
    ULONG secDescLength = 0;

    if ( ppStringSecurityDescriptor == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    *ppStringSecurityDescriptor = NULL;

    if ( ConvertSecurityDescriptorToStringSecurityDescriptorA(
             pSecurityDescriptor, dwRequestedStringSDRevision,
             securityInformation, &pSecDescriptorStr, &secDescLength ) == 0 )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
    MSDN:
    To free the returned buffer, call the LocalFree function
    */
    // since the caller will not be prepared to release with
    // LocalFree we will clone the string
    dwError = VmKdcAllocateMemory( secDescLength + 1, (PVOID*)&pStrToReturn );
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcStringCpyA( pStrToReturn, secDescLength + 1, pSecDescriptorStr);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppStringSecurityDescriptor = pStrToReturn;
    pStrToReturn = NULL;

error :

    if( pSecDescriptorStr != NULL )
    {
        LocalFree(pSecDescriptorStr);
        pSecDescriptorStr = NULL;
    }

    VMKDC_SAFE_FREE_MEMORY( pStrToReturn );

    return dwError;
}

static
VOID
VmKdcFreeAbsoluteSecurityDescriptor(
    PSECURITY_DESCRIPTOR_ABSOLUTE pSecDescAbs
)
{
    if( pSecDescAbs != NULL )
    {
        PSID pOwner = NULL;
        PSID pGroup = NULL;
        PACL pDacl = NULL;
        PACL pSacl = NULL;
        BOOLEAN bDefaulted = FALSE;
        BOOLEAN bPresent = FALSE;

        GetSecurityDescriptorOwner(pSecDescAbs, &pOwner, &bDefaulted);
        GetSecurityDescriptorGroup(pSecDescAbs, &pGroup, &bDefaulted);

        GetSecurityDescriptorDacl(pSecDescAbs, &bPresent, &pDacl, &bDefaulted);
        GetSecurityDescriptorSacl(pSecDescAbs, &bPresent, &pSacl, &bDefaulted);

        VMKDC_SAFE_FREE_MEMORY(pSecDescAbs);
        VMKDC_SAFE_FREE_MEMORY(pOwner);
        VMKDC_SAFE_FREE_MEMORY(pGroup);
        VMKDC_SAFE_FREE_MEMORY(pDacl);
        VMKDC_SAFE_FREE_MEMORY(pSacl);
    }
}

static
DWORD
VmKdcCreateAbsSecDescFromRelative(
    PSECURITY_DESCRIPTOR_RELATIVE pRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR_ABSOLUTE* ppAbsoluteSecurityDescriptor
)
{
    DWORD dwError = ERROR_SUCCESS;

    PSECURITY_DESCRIPTOR_ABSOLUTE pAbsSecDesc = NULL;
    DWORD dwAbsSecDescLen = 0;
    PACL pDacl = NULL;
    DWORD dwDaclLen = 0;
    PACL pSacl = NULL;
    DWORD dwSaclLen = 0;
    PSID pOwner = NULL;
    DWORD dwOwnerLen = 0;
    PSID pGroup = NULL;
    DWORD dwGroupLen = 0;

    if( ( pRelativeSecurityDescriptor == NULL)
        ||
        (ppAbsoluteSecurityDescriptor == NULL) )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    *ppAbsoluteSecurityDescriptor = NULL;

    // Get sizes
    if ( MakeAbsoluteSD(
                   pRelativeSecurityDescriptor,
                   pAbsSecDesc,
                   &dwAbsSecDescLen,
                   pDacl,
                   &dwDaclLen,
                   pSacl,
                   &dwSaclLen,
                   pOwner,
                   &dwOwnerLen,
                   pGroup,
                   &dwGroupLen) == 0 )
    {
        dwError = GetLastError();
        if ( ( dwError == STATUS_BUFFER_TOO_SMALL )
             ||
             (dwError == ERROR_INSUFFICIENT_BUFFER) )
        {
            dwError = ERROR_SUCCESS;
        }
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    // Allocate
    if (dwOwnerLen)
    {
        dwError = VmKdcAllocateMemory( dwOwnerLen, (PVOID*)(&pOwner));
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (dwGroupLen)
    {
        dwError = VmKdcAllocateMemory( dwGroupLen, (PVOID*)(&pGroup));
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (dwDaclLen)
    {
        dwError = VmKdcAllocateMemory( dwDaclLen, (PVOID*)(&pDacl));
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (dwSaclLen)
    {
        dwError = VmKdcAllocateMemory( dwSaclLen, (PVOID*)(&pSacl));
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateMemory(dwAbsSecDescLen, (PVOID*)&pAbsSecDesc);
    BAIL_ON_VMKDC_ERROR(dwError);

    // Translate
    if ( MakeAbsoluteSD(
            pRelativeSecurityDescriptor,
            pAbsSecDesc,
            &dwAbsSecDescLen,
            pDacl,
            &dwDaclLen,
            pSacl,
            &dwSaclLen,
            pOwner,
            &dwOwnerLen,
            pGroup,
            &dwGroupLen) == 0
      )
    {
        dwError = GetLastError();
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    *ppAbsoluteSecurityDescriptor = pAbsSecDesc;
    pAbsSecDesc = NULL;
    pOwner = NULL;
    pGroup = NULL;
    pSacl = NULL;
    pDacl = NULL;

error:

    VMKDC_SAFE_FREE_MEMORY(pAbsSecDesc);
    VMKDC_SAFE_FREE_MEMORY(pOwner);
    VMKDC_SAFE_FREE_MEMORY(pGroup);
    VMKDC_SAFE_FREE_MEMORY(pSacl);
    VMKDC_SAFE_FREE_MEMORY(pDacl);

    return dwError;
}
