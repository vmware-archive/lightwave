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

#include "stdafx.h"
/*
static
DWORD
MapBuiltinNameToSid(
    PSID *ppSid,
    PCWSTR pwszName
    )
{
    DWORD dwError = 0;
    union
    {
        SID sid;
        BYTE buffer[SID_MAX_SIZE];
    } Sid;
    ULONG SidSize = sizeof(Sid.buffer);
    PWSTR pwszEveryone = NULL;

    dwError = ConvertAnsitoUnicodeString(
                  "Everyone",
                  &pwszEveryone);
    BAIL_ON_VMEVENT_ERROR(dwError);


    if (LwRtlWC16StringIsEqual(pwszName, pwszEveryone, FALSE)) {
        dwError = LwNtStatusToWin32Error(
            RtlCreateWellKnownSid(
                WinWorldSid,
                NULL,
                &Sid.sid,
                &SidSize));
    }
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlDuplicateSid(ppSid, &Sid.sid));

cleanup:

    VMEVENT_SAFE_FREE_STRINGW(pwszEveryone);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
MapNameToSid(
    HANDLE hLsa,
    PCWSTR pwszName,
    PSID* ppSid
    )
{
    DWORD dwError = 0;
    PSTR pszName = NULL;
    LSA_QUERY_LIST QueryList;
    PLSA_SECURITY_OBJECT* ppObjects = NULL;

    dwError = LwWc16sToMbs(pwszName, &pszName);
    BAIL_ON_VMEVENT_ERROR(dwError);

    QueryList.ppszStrings = (PCSTR*) &pszName;

    dwError = LsaFindObjects(
        hLsa,
        NULL,
        0,
        LSA_OBJECT_TYPE_UNDEFINED,
        LSA_QUERY_TYPE_BY_NAME,
        1,
        QueryList,
        &ppObjects);
    BAIL_ON_VMEVENT_ERROR(dwError);

    if (ppObjects[0] == NULL) {
        dwError = LW_ERROR_NO_SUCH_OBJECT;
        BAIL_ON_VMEVENT_ERROR(dwError);
    }

    dwError = LwNtStatusToWin32Error(
                  RtlAllocateSidFromCString(
                  ppSid,
                  ppObjects[0]->pszObjectSid
                  )
              );
    BAIL_ON_VMEVENT_ERROR(dwError);

cleanup:

    LsaFreeSecurityObjectList(1, ppObjects);

    VMEVENT_SAFE_FREE_STRINGA(pszName);

    return dwError;

error:

    *ppSid = NULL;

    goto cleanup;
}

DWORD
ConstructSecurityDescriptor(
    DWORD dwAllowUserCount,
    PWSTR* ppwszAllowUsers,
    DWORD dwDenyUserCount,
    PWSTR* ppwszDenyUsers,
    BOOLEAN bReadOnly,
    PSECURITY_DESCRIPTOR_RELATIVE* ppRelative,
    PDWORD pdwRelativeSize
    )
{
    DWORD dwError = 0;
    PSECURITY_DESCRIPTOR_ABSOLUTE pAbsolute = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE pRelative = NULL;
    union
    {
        SID sid;
        BYTE buffer[SID_MAX_SIZE];
    } Owner;
    union
    {
        SID sid;
        BYTE buffer[SID_MAX_SIZE];
    } Group;
    ULONG OwnerSidSize = sizeof(Owner.buffer);
    ULONG GroupSidSize = sizeof(Group.buffer);
    DWORD dwDaclSize = 0;
    PACL pDacl = NULL;
    DWORD dwIndex = 0;
    PSID pSid = NULL;
    ULONG ulRelativeSize = 0;
    HANDLE hLsa = NULL;
    ACCESS_MASK mask = bReadOnly ?
        (FILE_GENERIC_READ|FILE_GENERIC_EXECUTE) :
        FILE_ALL_ACCESS;

    dwError = LsaOpenServer(&hLsa);
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlCreateWellKnownSid(
            WinBuiltinAdministratorsSid,
            NULL,
            &Owner.sid,
            &OwnerSidSize));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlCreateWellKnownSid(
            WinBuiltinPowerUsersSid,
            NULL,
            &Group.sid,
            &GroupSidSize));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwDaclSize = ACL_HEADER_SIZE +
        dwAllowUserCount * (sizeof(ACCESS_ALLOWED_ACE) + SID_MAX_SIZE) +
        dwDenyUserCount * (sizeof(ACCESS_DENIED_ACE) + SID_MAX_SIZE) +
        RtlLengthSid(&Owner.sid) + RtlLengthSid(&Group.sid);

    dwError = EventLogAllocateMemory(
        dwDaclSize,
        OUT_PPVOID(&pDacl));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlCreateAcl(pDacl, dwDaclSize, ACL_REVISION));
    BAIL_ON_VMEVENT_ERROR(dwError);

    for (dwIndex = 0; dwIndex < dwDenyUserCount; dwIndex++) {
        dwError = MapNameToSid(hLsa, ppwszDenyUsers[dwIndex], &pSid);
        if (dwError != LW_ERROR_SUCCESS) {
            dwError = MapBuiltinNameToSid(&pSid, ppwszDenyUsers[dwIndex]);
        }

        BAIL_ON_VMEVENT_ERROR(dwError);

        dwError = LwNtStatusToWin32Error(
            RtlAddAccessDeniedAceEx(
                pDacl,
                ACL_REVISION,
                0,
                FILE_ALL_ACCESS,
                pSid));
        BAIL_ON_VMEVENT_ERROR(dwError);

        RTL_FREE(&pSid);
    }

    for (dwIndex = 0; dwIndex < dwAllowUserCount; dwIndex++) {
        dwError = MapNameToSid(hLsa, ppwszAllowUsers[dwIndex], &pSid);
        if (dwError != LW_ERROR_SUCCESS) {
            dwError = MapBuiltinNameToSid(&pSid, ppwszAllowUsers[dwIndex]);
        }
        BAIL_ON_VMEVENT_ERROR(dwError);

        dwError = LwNtStatusToWin32Error(
            RtlAddAccessAllowedAceEx(
                pDacl,
                ACL_REVISION,
                0,
                mask,
                pSid));
        BAIL_ON_VMEVENT_ERROR(dwError);

        RTL_FREE(&pSid);
    }

    dwError = EventLogAllocateMemory(
        SECURITY_DESCRIPTOR_ABSOLUTE_MIN_SIZE,
        OUT_PPVOID(&pAbsolute));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlCreateSecurityDescriptorAbsolute(
            pAbsolute,
            SECURITY_DESCRIPTOR_REVISION));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlSetOwnerSecurityDescriptor(
            pAbsolute,
            &Owner.sid,
            FALSE));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlSetGroupSecurityDescriptor(
            pAbsolute,
            &Group.sid,
            FALSE));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlSetDaclSecurityDescriptor(
            pAbsolute,
            TRUE,
            pDacl,
            FALSE));
    BAIL_ON_VMEVENT_ERROR(dwError);

    RtlAbsoluteToSelfRelativeSD(
        pAbsolute,
        NULL,
        &ulRelativeSize);

    dwError = EventLogAllocateMemory(ulRelativeSize, OUT_PPVOID(&pRelative));
    BAIL_ON_VMEVENT_ERROR(dwError);

    dwError = LwNtStatusToWin32Error(
        RtlAbsoluteToSelfRelativeSD(
            pAbsolute,
            pRelative,
            &ulRelativeSize));
    BAIL_ON_VMEVENT_ERROR(dwError);

    *ppRelative = pRelative;
    *pdwRelativeSize = ulRelativeSize;

cleanup:

    if (hLsa) {
        LsaCloseServer(hLsa);
    }

    VMEVENT_SAFE_FREE_MEMORY(pSid);
    VMEVENT_SAFE_FREE_MEMORY(pDacl);
    VMEVENT_SAFE_FREE_MEMORY(pAbsolute);

    return dwError;

error:

    *ppRelative = NULL;
    *pdwRelativeSize = 0;

    VMEVENT_SAFE_FREE_MEMORY(pRelative);

    goto cleanup;
}
*/
