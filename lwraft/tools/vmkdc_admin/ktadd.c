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
 * Module Name: vmkdc_admin
 *
 * Filename: ktadd.c
 *
 * Abstract:
 *
 * vmkdc_admin kdadd module
 *
 */

#include "includes.h"

static
VOID
VmKdcFreeKeyTabEntry(
    PVMKDC_MIT_KEYTAB_FILE pKeyTabEntry)
{
    int i = 0;

    if (pKeyTabEntry)
    {
        for (i=0; i<pKeyTabEntry->nameComponentsLen; i++)
        {
            VMKDC_SAFE_FREE_STRINGA(pKeyTabEntry->nameComponents[i]);
        }
        VMKDC_SAFE_FREE_MEMORY(pKeyTabEntry->nameComponents);
        VMKDC_SAFE_FREE_KEY(pKeyTabEntry->key);
        VMKDC_SAFE_FREE_STRINGA(pKeyTabEntry->realm);
        VmKdcFreeMemory(pKeyTabEntry);
    }
}

static
DWORD
VmKdcMakeKeyTabEntry(
    PVMKDC_CONTEXT pContext,
    PCSTR pszUpnName,
    PVMKDC_KEY pKey,
    PVMKDC_MIT_KEYTAB_FILE *ppRetKeyTabEntry)
{
    DWORD dwError = 0;
    PVMKDC_MIT_KEYTAB_FILE pKeyTabEntry = NULL;
    PVMKDC_PRINCIPAL pPrincipal = NULL;
    int i = 0;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_MIT_KEYTAB_FILE),
                                  (PVOID*)&pKeyTabEntry);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcParsePrincipalName(pContext, pszUpnName, &pPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    pKeyTabEntry->princType = pPrincipal->type;

    dwError = VmKdcAllocateStringA(VMKDC_GET_PTR_DATA(pPrincipal->realm),
                                   &pKeyTabEntry->realm);
    BAIL_ON_VMKDC_ERROR(dwError);

    pKeyTabEntry->nameComponentsLen = pPrincipal->numComponents;

    dwError = VmKdcAllocateMemory(sizeof(char *) * pKeyTabEntry->nameComponentsLen,
                                  (PVOID*)&pKeyTabEntry->nameComponents);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<pPrincipal->numComponents; i++)
    {
        dwError = VmKdcAllocateStringA(VMKDC_GET_PTR_DATA(pPrincipal->components[i]),
                                       &pKeyTabEntry->nameComponents[i]);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    pKeyTabEntry->timeStamp = (int)time(NULL);

    dwError = VmKdcCopyKey(pKey, &pKeyTabEntry->key);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetKeyTabEntry = pKeyTabEntry;

error:
    if (dwError)
    {
        VmKdcFreeKeyTabEntry(pKeyTabEntry);
        pKeyTabEntry = NULL;
    }
    VMKDC_SAFE_FREE_PRINCIPAL(pPrincipal);

    return dwError;
}

DWORD
VmKdcAdminKtAdd(int argc, char *argv[], PROG_ARGS *args)
{
    DWORD   dwError = 0;
    PSTR    pszUpnName = NULL;
    PCSTR   pszDomainName = NULL;
    PCSTR   pszKeyTab = "/tmp/krb.keys";
    PBYTE	pMasterKeyBlob = NULL;
    DWORD	dwMasterKeySize = 0;
    PBYTE	pUpnKeyBlob = NULL;
    DWORD	dwUpnKeySize = 0;
    PVMKDC_DATA pUpnKeyData = NULL;
    PVMKDC_KEYSET pUpnKeySet = NULL;
    PVMKDC_KEY pMasterKey = NULL;
    PVMKDC_KEYTAB_HANDLE pKeyTab = NULL;
    PVMKDC_MIT_KEYTAB_FILE pKeyTabEntry = NULL;
    PVMKDC_PRINCIPAL pPrincipal = NULL;
    PVMKDC_CONTEXT pContext = NULL;
    int i = 0;

    if (args->keytab)
    {
        pszKeyTab = args->keytab;
    }

    if (argc != 1 || IsNullOrEmptyString(argv[0]))
    {
        ShowUsage(VmKdc_argv0, "Wrong number of principals specified");
    }

    dwError = VmKdcAdminInitContext(&pContext);
    BAIL_ON_VMKDC_ERROR(dwError);

    pszDomainName = gVmkdcGlobals.pszDefaultRealm;

    /* Get the UPN name by parsing/unparsing the input name */
    dwError = VmKdcParsePrincipalName(pContext, argv[0], &pPrincipal);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcUnparsePrincipalName(pPrincipal, &pszUpnName);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Get the encrypted principal keys */
    dwError = VmDirGetKrbUPNKey((PSTR)pszUpnName, &pUpnKeyBlob, &dwUpnKeySize);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Decode the principal keys */
    dwError = VmKdcAllocateData(pUpnKeyBlob,
                                dwUpnKeySize,
                                &pUpnKeyData);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcDecodeKeySet(pUpnKeyData, &pUpnKeySet);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Get the unencrypted master key */
    dwError = VmDirGetKrbMasterKey((PSTR)pszDomainName, &pMasterKeyBlob, &dwMasterKeySize);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Decode the master key */
    dwError = VmKdcDecodeMasterKey(pMasterKeyBlob,
                                   dwMasterKeySize,
                                   &pMasterKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Decrypt the principal keys */
    dwError = VmKdcDecryptKeySet(pContext, pMasterKey, pUpnKeySet);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* Open keytab */
    dwError = VmKdcParseKeyTabOpen((PSTR)pszKeyTab, "a", &pKeyTab);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<pUpnKeySet->numKeys; i++)
    {
        /* Construct a new keytab entry */
        dwError = VmKdcMakeKeyTabEntry(pContext,
                                       pszUpnName,
                                       pUpnKeySet->keys[i],
                                       &pKeyTabEntry);
        BAIL_ON_VMKDC_ERROR(dwError);

        /* Write the keytab entry */
        dwError = VmKdcParseKeyTabWrite(pKeyTab, pKeyTabEntry);
        BAIL_ON_VMKDC_ERROR(dwError);

        VmKdcFreeKeyTabEntry(pKeyTabEntry);
        pKeyTabEntry = NULL;
    }

error:
    if (pKeyTab)
    {
        VmKdcParseKeyTabClose(pKeyTab);
        pKeyTab = NULL;
    }
    if (pKeyTabEntry)
    {
        VmKdcFreeKeyTabEntry(pKeyTabEntry);
        pKeyTabEntry = NULL;
    }
    if (pContext)
    {
        VmKdcAdminDestroyContext(pContext);
        pContext = NULL;
    }
    VMKDC_SAFE_FREE_KEY(pMasterKey);
    VMKDC_SAFE_FREE_KEYSET(pUpnKeySet);
    VMKDC_SAFE_FREE_DATA(pUpnKeyData);
    VMKDC_SAFE_FREE_PRINCIPAL(pPrincipal);
    VMKDC_SAFE_FREE_MEMORY(pMasterKeyBlob);
    VMKDC_SAFE_FREE_MEMORY(pUpnKeyBlob);

    return dwError;
}
