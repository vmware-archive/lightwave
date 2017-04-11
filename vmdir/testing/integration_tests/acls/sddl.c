/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
_ConvertStringToDescriptor(
    PCSTR pszSddl,
    PSECURITY_DESCRIPTOR_RELATIVE *ppSecurityDescriptor
    )
{
    DWORD dwError = 0;
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor = NULL;

    dwError = RtlAllocateSecurityDescriptorFromSddlCString(
                &pSecurityDescriptor,
                NULL,
                pszSddl,
                SDDL_REVISION_1);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSecurityDescriptor = pSecurityDescriptor;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
_ConvertDescriptorToString(
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor,
    PSTR *ppszSddl
    )
{
    DWORD dwError = 0;
    PSTR pszLocalAclString = NULL;
    SECURITY_INFORMATION secInfoAll = (OWNER_SECURITY_INFORMATION |
                                       GROUP_SECURITY_INFORMATION |
                                       DACL_SECURITY_INFORMATION |
                                       SACL_SECURITY_INFORMATION);

    dwError = RtlAllocateSddlCStringFromSecurityDescriptor(
                &pszLocalAclString,
                pSecurityDescriptor,
                SDDL_REVISION_1,
                secInfoAll);
    dwError = LwNtStatusToWin32Error(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszSddl = pszLocalAclString;

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
TestSDDLRoundTrip(
    PVMDIR_TEST_STATE pState
    )
{
#if 0 // TODO
    original SDDL ==> O:BAG:BAD:(A;;SDRPWPCC;;;S-1-7-32-666)(A;;GXWDRCCCDCRPWP;;;BA)
    converted SDDL ==> O:BAG:BAD:(A;;SDCCRPWP;;;S-1-7-32-666)(A;;GXRCWDGXCCDCRPWP;;;BA)

#endif

    // TODO
    PCSTR pszOriginalSddl = "O:BAG:BAD:(A;;SDCCRPWP;;;S-1-7-32-666)(A;;GXRCWDCCDCRPWP;;;BA)";
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor = NULL;
    PSTR pszConvertedSddl = NULL;
    DWORD dwError = 0;

    dwError = _ConvertStringToDescriptor(pszOriginalSddl, &pSecurityDescriptor);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _ConvertDescriptorToString(pSecurityDescriptor, &pszConvertedSddl);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("original SDDL ==> %s\n", pszOriginalSddl);
    printf("converted SDDL ==> %s\n", pszConvertedSddl);
    dwError = VmDirStringCompareA(pszOriginalSddl, pszConvertedSddl, TRUE);
    TestAssert(dwError == 0);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pSecurityDescriptor);
    VMDIR_SAFE_FREE_MEMORY(pszConvertedSddl);
    return dwError;
error:
    goto cleanup;
}


// TODO -- Make sure all permissions (and permutations?) are tested
// TODO -- Move to unit tests.
DWORD
TestSecurityDescriptorsSddl(
    PVMDIR_TEST_STATE pState
    )
{
    DWORD dwError = 0;

    printf("Testing security descriptor SDDL related code ...\n");

    dwError = TestSDDLRoundTrip(pState);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    printf("Security Descriptor tests failed with error 0n%d\n", dwError);
    goto cleanup;
}
