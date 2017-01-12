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
VmKdcFreeAuthenticator(
    PVMKDC_AUTHENTICATOR pAuthenticator)
{
    if (pAuthenticator)
    {
        VMKDC_SAFE_FREE_PRINCIPAL(pAuthenticator->cname);
        VMKDC_SAFE_FREE_KEY(pAuthenticator->subkey);
        VMKDC_SAFE_FREE_MEMORY(pAuthenticator);
    }
}

DWORD
VmKdcDecodeAuthenticator(
    PVMKDC_DATA pData,
    PVMKDC_AUTHENTICATOR *ppRetAuthenticator)
{
    DWORD dwError = 0;
    PVMKDC_AUTHENTICATOR pAuthenticator = NULL;
    Authenticator heimAuth = {0};
    unsigned char *authBufPtr = NULL;
    size_t heimAuthLen = 0;
    size_t authBufLen = 0;

    authBufLen = VMKDC_GET_LEN_DATA(pData);
    authBufPtr = (unsigned char *) VMKDC_GET_PTR_DATA(pData);

    /*
     * Decode the Authenticator into a Heimdal Authenticator structure.
     */
    decode_Authenticator(authBufPtr, authBufLen, &heimAuth, &heimAuthLen);
    if (heimAuthLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_AUTHENTICATOR), (PVOID*)&pAuthenticator);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* authenticator-vno */
    pAuthenticator->authenticator_vno = heimAuth.authenticator_vno;

    /* crealm, cname */
    dwError = VmKdcMakePrincipal(heimAuth.crealm,
                                 heimAuth.cname.name_string.len,
                                 (PCSTR *)heimAuth.cname.name_string.val,
                                 &pAuthenticator->cname);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* cksum */
    if (heimAuth.cksum)
    {
        /* TBD */
    }

    /* cusec */
    pAuthenticator->cusec = heimAuth.cusec;

    /* ctime */
    pAuthenticator->ctime = heimAuth.ctime;

    /* subkey */
    if (heimAuth.subkey)
    {
        dwError = VmKdcMakeKey((VMKDC_KEYTYPE) heimAuth.subkey->keytype,
                               0,
                               (PUCHAR) heimAuth.subkey->keyvalue.data,
                               (DWORD) heimAuth.subkey->keyvalue.length,
                               &pAuthenticator->subkey);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /* seq-number */
    if (heimAuth.seq_number)
    {
        /* TBD */
    }

    /* authorization_data */
    if (heimAuth.authorization_data)
    {
        /* TBD */
    }

    *ppRetAuthenticator = pAuthenticator;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_AUTHENTICATOR(pAuthenticator);
    }
    free_Authenticator(&heimAuth);

    return dwError;
}

VOID
VmKdcPrintAuthenticator(
    PVMKDC_AUTHENTICATOR pAuthenticator)
{
    VmKdcPrintPrincipal(pAuthenticator->cname);
}
