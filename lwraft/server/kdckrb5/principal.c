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

VOID
VmKdcFreePrincipal(
    PVMKDC_PRINCIPAL pPrincipal)
{
    DWORD i = 0;

    if (pPrincipal)
    {
        if (pPrincipal->components) {
            for (i=0; i<(int) pPrincipal->numComponents; i++) {
                VMKDC_SAFE_FREE_DATA(pPrincipal->components[i]);
            }
            VMKDC_SAFE_FREE_MEMORY(pPrincipal->components);
        }
        VMKDC_SAFE_FREE_DATA(pPrincipal->realm);
        VMKDC_SAFE_FREE_MEMORY(pPrincipal->name);
        VMKDC_SAFE_FREE_MEMORY(pPrincipal);
    }
}

DWORD
VmKdcAllocatePrincipal(
    PVMKDC_PRINCIPAL *ppRetPrincipal)
{
    PVMKDC_PRINCIPAL pPrincipal = NULL;
    DWORD dwError = 0;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_PRINCIPAL), (PVOID*)&pPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_PRINCIPAL(pPrincipal);
    }
    *ppRetPrincipal = pPrincipal;
    return dwError;
}

DWORD
VmKdcMakePrincipal(
    PCSTR realmName,
    DWORD numComponents,
    PCSTR *components,
    PVMKDC_PRINCIPAL *ppRetPrincipal)
{
    PVMKDC_PRINCIPAL pPrincipal = NULL;
    DWORD dwError = 0;
    DWORD i = 0;

    dwError = VmKdcAllocatePrincipal(&pPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    pPrincipal->type = VMKDC_NT_PRINCIPAL;

    dwError = VmKdcAllocateDataString(realmName, &pPrincipal->realm);
    BAIL_ON_VMKDC_ERROR(dwError);

    pPrincipal->numComponents = numComponents;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_DATA *) * numComponents,
                                  (PVOID*)&pPrincipal->components);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<numComponents; i++) {
        dwError = VmKdcAllocateDataString(components[i], &pPrincipal->components[i]);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_PRINCIPAL(pPrincipal);
    }
    *ppRetPrincipal = pPrincipal;
    return dwError;
}

DWORD
VmKdcParsePrincipalName(
    PVMKDC_CONTEXT pContext,
    PCSTR pPrincipalName,
    PVMKDC_PRINCIPAL *ppRetPrincipal)
{
    DWORD dwError = 0;
    PVMKDC_PRINCIPAL pPrincipal = NULL;
    PVMKDC_DATA pRealm = NULL;
    PSTR p = NULL;
    PSTR q = NULL;
    PSTR pNewPrincipalName = NULL;
    int count = 0;
    size_t dwLen = 0;
    BOOLEAN bAtFound = FALSE;
    CHAR cSave = '\0';
    PCSTR pszRealm = NULL;

    if (pPrincipalName == NULL ||
        *pPrincipalName == '/' ||
        *pPrincipalName == '@' ||
        *pPrincipalName == '\0')
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwLen = strlen(pPrincipalName) + 1;
    dwError = VmKdcAllocateMemory(sizeof(CHAR) + dwLen, (PVOID*)&pNewPrincipalName);
    BAIL_ON_VMKDC_ERROR(dwError);
    memcpy(pNewPrincipalName, pPrincipalName, dwLen);
    pNewPrincipalName[dwLen] = '\0';

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_PRINCIPAL), (PVOID*)&pPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* count the number of name components */
    p = (PSTR) pNewPrincipalName;
    do
    {
        p++;
        if (*p == '/' || *p == '@' || *p == '\0')
        {
            count++;
        }
    }
    while (*p && *p != '@');

    /* allocate an array for the name components */
    dwError = VmKdcAllocateMemory(
                  sizeof(VMKDC_DATA *) * count,
                  (PVOID*)&pPrincipal->components);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* allocate and copy the name components */
    count = 0;
    p = (PSTR) pNewPrincipalName;
    q = p;
    do
    {
        p++;
        if (*p == '/' || *p == '@' || *p == '\0')
        {
            dwLen = p - q;
            cSave = q[dwLen];
            q[dwLen] = '\0';
            dwError = VmKdcAllocateDataString(
                          q,
                          &pPrincipal->components[count]);
            q[dwLen] = cSave;
            BAIL_ON_VMKDC_ERROR(dwError);

            count++;
            if (*p == '@')
            {
                bAtFound = TRUE;
                break;
            }

            q = p + 1;
        }
    }
    while (*p && *p != '@');

    if (bAtFound)
    {
        /* allocate and copy the realm */
        dwError = VmKdcAllocateDataString(
                      p+1,
                      &pRealm);
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    else
    {
        /* get the default realm */
        pszRealm = pContext->pGlobals->pszDefaultRealm;
        BAIL_ON_VMKDC_INVALID_POINTER(pszRealm, dwError);
        dwError = VmKdcAllocateDataString(
                      pszRealm,
                      &pRealm);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    pPrincipal->type = VMKDC_NT_PRINCIPAL;
    pPrincipal->name = pNewPrincipalName;
    pPrincipal->realm = pRealm;
    pPrincipal->numComponents = count;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_PRINCIPAL(pPrincipal);
    }

    *ppRetPrincipal = pPrincipal;
    return dwError;
}

DWORD
VmKdcUnparsePrincipalName(
    PVMKDC_PRINCIPAL pPrincipal,
    PSTR *ppRetPrincipalName)
{
    DWORD dwError = 0;
    PSTR principalName = NULL;
    PVMKDC_DATA data = NULL;
    DWORD allocLen = 0;
    DWORD index = 0;
    DWORD i = 0;
    PSTR ptr;
    DWORD len;

    BAIL_ON_VMKDC_INVALID_POINTER(pPrincipal, dwError);

    for (i=0; i<pPrincipal->numComponents; i++)
    {
        allocLen += VMKDC_GET_LEN_DATA(pPrincipal->components[i]) + 1;
    }
    allocLen += VMKDC_GET_LEN_DATA(pPrincipal->realm) + 1;

    dwError = VmKdcAllocateMemory(allocLen, (PVOID*)&principalName);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<pPrincipal->numComponents; i++)
    {
        if (i > 0)
        {
            principalName[index++] = '/';
        }
        data = pPrincipal->components[i];
        ptr = VMKDC_GET_PTR_DATA(data);
        len = VMKDC_GET_LEN_DATA(data);
        strncpy(&principalName[index], ptr, len);
#if 0
        /*
         * On Win32, calls strncpy_s(), which doesn't work as expected
         */
        dwError = VmKdcStringNCpyA(&principalName[index], len, ptr, len);
        BAIL_ON_VMKDC_ERROR(dwError);
#endif
        index += len;
    }
    principalName[index++] = '@';
    data = pPrincipal->realm;
    ptr = VMKDC_GET_PTR_DATA(data);
    len = VMKDC_GET_LEN_DATA(data);
    strncpy(&principalName[index], ptr, len);

    for (i=index; principalName[i]; i++)
    {
        VMDIR_ASCII_LOWER_TO_UPPER(principalName[i]);
    }

#if 0
    /*
     * On Win32, calls strncpy_s(), which doesn't work as expected
     */
    dwError = VmKdcStringNCpyA(&principalName[index], len, ptr, len);
    BAIL_ON_VMKDC_ERROR(dwError);
#endif

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_MEMORY(principalName);
    }
    *ppRetPrincipalName = principalName;

    return dwError;
}

DWORD
VmKdcGetPrincipalName(
    PVMKDC_PRINCIPAL pPrincipal,
    PSTR *ppRetPrincipalName)
{
    DWORD dwError = 0;
    BAIL_ON_VMKDC_INVALID_POINTER(pPrincipal, dwError);

    if (!pPrincipal->name)
    {
        dwError = VmKdcUnparsePrincipalName(pPrincipal, &pPrincipal->name);
        BAIL_ON_VMKDC_ERROR(dwError);
    } 

error:
    *ppRetPrincipalName = pPrincipal->name;
    return dwError;
}

DWORD
VmKdcCopyPrincipal(
    PVMKDC_PRINCIPAL pPrincipal,
    PVMKDC_PRINCIPAL *ppRetPrincipal)
{
    PVMKDC_PRINCIPAL pNewPrincipal = NULL;
    DWORD dwError = 0;
    DWORD i = 0;

    dwError = VmKdcAllocatePrincipal(&pNewPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    pNewPrincipal->type = pPrincipal->type;
    pNewPrincipal->numComponents = pPrincipal->numComponents;

    dwError = VmKdcAllocateData(VMKDC_GET_PTR_DATA(pPrincipal->realm),
                                VMKDC_GET_LEN_DATA(pPrincipal->realm),
                                &pNewPrincipal->realm);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_DATA *) * pNewPrincipal->numComponents,
                                 (PVOID*)&pNewPrincipal->components);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<pNewPrincipal->numComponents; i++) {
        dwError = VmKdcAllocateData(VMKDC_GET_PTR_DATA(pPrincipal->components[i]),
                                    VMKDC_GET_LEN_DATA(pPrincipal->components[i]),
                                    &pNewPrincipal->components[i]);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    if (dwError) {
        VMKDC_SAFE_FREE_PRINCIPAL(pNewPrincipal);
    }
    *ppRetPrincipal = pNewPrincipal;
    return dwError;
}

VOID
VmKdcPrintPrincipal(
    PVMKDC_PRINCIPAL pPrincipal)
{
    DWORD dwError = 0;
    DWORD i = 0;

    BAIL_ON_VMKDC_INVALID_POINTER(pPrincipal, dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintPrincipal: type <%d>",
             pPrincipal->name);
    for (i=0; i<pPrincipal->numComponents; i++)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintPrincipal: name[%d]      <%s>",
                 i, VMKDC_GET_PTR_DATA(pPrincipal->components[i]));
    }

error:
    return;
}
