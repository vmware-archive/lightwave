/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
LwCAIsValidDN(
    PCSTR       pcszDN,
    PBOOLEAN    pbIsValid
    )
{
    DWORD dwError = 0;
    LDAPDN dn = NULL;

    if (IsNullOrEmptyString(pcszDN) || !pbIsValid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = ldap_str2dn(pcszDN, &dn, LDAP_DN_FORMAT_LDAPV3);
    if (dwError == LDAP_SUCCESS && dn != NULL)
    {
        *pbIsValid = TRUE;
    }
    else
    {
        *pbIsValid = FALSE;
    }

error:
    if (dn)
    {
        ldap_dnfree(dn);
    }
    if (pbIsValid)
    {
        *pbIsValid = FALSE;
    }
    return dwError;
}

/*
 * convert DN to a list of RDN.
 *
 * say dc=lwraft,dc=local
 * if bNotypes == false, {"dc=vmdir", "dc=local"} is returned;
 * otherwise {"vmdir", "local"} is returned.
 */
DWORD
LwCADNToRDNArray(
    PCSTR               pcszDN,
    BOOLEAN             bNotypes,
    PLWCA_STRING_ARRAY* ppRDNStrArray
    )
{
    DWORD               dwError = 0;
    DWORD               dwCount = 0;
    PLWCA_STRING_ARRAY  pStrArray = NULL;
    PSTR*               ppszRDN = NULL;
    PSTR*               ppszTmp = NULL;
    DWORD               iEntry = 0;

    if (IsNullOrEmptyString(pcszDN) || !ppRDNStrArray)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    if (bNotypes)
    {
        ppszRDN = ldap_explode_dn(pcszDN, 1);
    }
    else
    {
        ppszRDN = ldap_explode_dn(pcszDN, 0);
    }

    if (!ppszRDN)
    {
        dwError = LWCA_ERROR_INVALID_DN;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    for (ppszTmp = ppszRDN; *ppszTmp; ppszTmp++)
    {
        dwCount++;
    }

    dwError = LwCAAllocateMemory(sizeof(LWCA_STRING_ARRAY), (PVOID*)&pStrArray);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(PSTR) * dwCount, (PVOID*)&pStrArray->ppData);
    BAIL_ON_LWCA_ERROR(dwError);

    pStrArray->dwCount = dwCount;

    for (ppszTmp = ppszRDN; *ppszTmp; ppszTmp++)
    {
        dwError = LwCAAllocateStringA(*ppszTmp, &pStrArray->ppData[iEntry++]);
        BAIL_ON_LWCA_ERROR(dwError);
    }

    *ppRDNStrArray = pStrArray;

cleanup:
    if (ppszRDN)
    {
        ldap_value_free(ppszRDN);
    }

    return dwError;

error:
    LwCAFreeStringArray(pStrArray);
    if (ppRDNStrArray)
    {
        *ppRDNStrArray = NULL;
    }

    goto cleanup;
}
