/*
 * Copyright 2012-2016 VMware, Inc.  All Rights Reserved.
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

struct _VMKDC_AUTHZ_PAC {
    krb5_context ctx;
    krb5_pac pac;
};

VOID
VmKdcFreeAuthzDataElem(
    PVMKDC_AUTHZDATA_ELEM pAuthzDataElem)
{
    if (pAuthzDataElem)
    {
        VMKDC_SAFE_FREE_DATA(pAuthzDataElem->ad_data);
        VMKDC_SAFE_FREE_MEMORY(pAuthzDataElem);
    }
}

DWORD
VmKdcMakeAuthzDataElem(
    DWORD ad_type,
    PVOID ptr,
    DWORD len,
    PVMKDC_AUTHZDATA_ELEM *ppAuthzDataElem)
{
    DWORD dwError = 0;
    PVMKDC_AUTHZDATA_ELEM pAuthzDataElem = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_AUTHZDATA_ELEM),
                                  (PVOID*)&pAuthzDataElem);
    BAIL_ON_VMKDC_ERROR(dwError);

    pAuthzDataElem->ad_type = ad_type;

    dwError = VmKdcAllocateData(ptr, len, &pAuthzDataElem->ad_data);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppAuthzDataElem = pAuthzDataElem;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_AUTHZDATA_ELEM(pAuthzDataElem);
    }

    return dwError;
}

VOID
VmKdcFreeAuthzData(
    PVMKDC_AUTHZDATA pAuthzData)
{
    size_t i = 0;

    if (pAuthzData)
    {
        if (pAuthzData->elem)
        {
            for (i=0; i<pAuthzData->count; i++)
            {
                VMKDC_SAFE_FREE_AUTHZDATA_ELEM(pAuthzData->elem[i]);
            }
            VMKDC_SAFE_FREE_MEMORY(pAuthzData->elem);
        }
        VMKDC_SAFE_FREE_MEMORY(pAuthzData);
    }
}

DWORD
VmKdcAllocateAuthzData(
    PVMKDC_AUTHZDATA *ppAuthzData)
{
    DWORD dwError = 0;
    PVMKDC_AUTHZDATA pAuthzData = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_AUTHZDATA),
                                  (PVOID *)&pAuthzData);
    BAIL_ON_VMKDC_ERROR(dwError);

    pAuthzData->count = 0;
    pAuthzData->elem = NULL;

    *ppAuthzData = pAuthzData;

error:

    if (dwError)
    {
        VMKDC_SAFE_FREE_AUTHZDATA(pAuthzData);
    }

    return dwError;
}

DWORD
VmKdcAddAuthzData(
    PVMKDC_AUTHZDATA pAuthzData,
    PVMKDC_DATA pData,
    VMKDC_AUTHZDATA_TYPE ad_type)
{
    DWORD dwError = 0;
    DWORD iSize = 0;

    iSize = (pAuthzData->count+1) * sizeof(PVMKDC_AUTHZDATA_ELEM);
    dwError = VmKdcReallocateMemory(
                      (PVOID)pAuthzData->elem,
                      (PVOID*)&pAuthzData->elem,
                      iSize);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcMakeAuthzDataElem(
                      ad_type,
                      VMKDC_GET_PTR_DATA(pData),
                      VMKDC_GET_LEN_DATA(pData),
                      &pAuthzData->elem[pAuthzData->count]);
    BAIL_ON_VMKDC_ERROR(dwError);

    pAuthzData->count += 1;

error:

    return dwError;
}

DWORD
VmKdcCopyAuthzData(
    PVMKDC_AUTHZDATA pAuthzDataSrc,
    PVMKDC_AUTHZDATA *ppAuthzDataDst)
{
    DWORD dwError = 0;
    PVMKDC_AUTHZDATA pAuthzDataDst = NULL;
    DWORD i = 0;

    dwError = VmKdcAllocateAuthzData(&pAuthzDataDst);
    BAIL_ON_VMKDC_ERROR(dwError);
   
    for (i=0; i<pAuthzDataSrc->count; i++)
    {
        dwError = VmKdcAddAuthzData(
                          pAuthzDataDst,
                          pAuthzDataSrc->elem[i]->ad_data,
                          pAuthzDataSrc->elem[i]->ad_type);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    *ppAuthzDataDst = pAuthzDataDst;

error:

    return dwError;
}

DWORD
VmKdcEncodeAuthzData(
    PVMKDC_AUTHZDATA pAuthzData,
    PVMKDC_DATA *ppRetData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;
    AuthorizationData heimData = {0};
    DWORD i = 0;
    unsigned char *dataBufPtr = NULL;
    size_t heimDataLen = 0;
    size_t dataBufLen = 0;
    int err = 0;

    heimData.len = pAuthzData->count;

    dwError = VmKdcAllocateMemory(sizeof(*heimData.val) * heimData.len,
                                  (PVOID*)&heimData.val);
    BAIL_ON_VMKDC_ERROR(dwError);

    for (i=0; i<pAuthzData->count; i++)
    {
        heimData.val[i].ad_type = pAuthzData->elem[i]->ad_type;
        heimData.val[i].ad_data.length = VMKDC_GET_LEN_DATA(pAuthzData->elem[i]->ad_data);
        heimData.val[i].ad_data.data = VMKDC_GET_PTR_DATA(pAuthzData->elem[i]->ad_data);
    }

    /*
     * Encode the AuthorizationData into Heimdal structure
     */
    ASN1_MALLOC_ENCODE(AuthorizationData,
                       dataBufPtr, dataBufLen,
                       &heimData, &heimDataLen,
                       err);
    if (err != 0 || dataBufLen != heimDataLen)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(dataBufPtr, (int) dataBufLen, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pAsnData;

error:
    VMKDC_SAFE_FREE_MEMORY(heimData.val);
    if (dataBufPtr)
    {
        free(dataBufPtr);
    }

    return dwError;
}

DWORD
VmKdcDecodeAuthzData(
    PVMKDC_DATA pData,
    PVMKDC_AUTHZDATA *ppAuthzData)
{
    DWORD dwError = 0;
    PVMKDC_AUTHZDATA pAuthzData = NULL;
    AuthorizationData heimData = {0};
    unsigned char *dataBufPtr = NULL;
    size_t heimDataLen = 0;
    size_t dataBufLen = 0;
    int i = 0;
    PVMKDC_DATA pTmpData = NULL;

    memset(&heimData, 0, sizeof(heimData));
    dataBufPtr = VMKDC_GET_PTR_DATA(pData);
    dataBufLen = VMKDC_GET_LEN_DATA(pData);

    /*
     * Decode the data into a Heimdal AuthorizationData structure.
     */
    decode_AuthorizationData(dataBufPtr, dataBufLen, &heimData, &heimDataLen);
    if (heimDataLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateAuthzData(&pAuthzData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Translate the decoded AuthorizationData to a VMKDC_AUTHZDATA structure.
     */
    for (i=0; i<heimData.len; i++)
    {
        dwError = VmKdcAllocateData(
                          heimData.val->ad_data.data,
                          heimData.val->ad_data.length,
                          &pTmpData);
        BAIL_ON_VMKDC_ERROR(dwError);

        dwError = VmKdcAddAuthzData(
                          pAuthzData,
                          pTmpData,
                          heimData.val->ad_type);
        BAIL_ON_VMKDC_ERROR(dwError);

        VMKDC_SAFE_FREE_DATA(pTmpData);
        pData = NULL;
    }

    *ppAuthzData = pAuthzData;

error:
    VMKDC_SAFE_FREE_DATA(pTmpData);

    return dwError;
}

VOID
VmKdcPrintAuthzData(
    PVMKDC_AUTHZDATA pAuthzData)
{
#if 0
    VmKdcPrintPrincipal(pAuthzData->cname);
#endif
}

DWORD
VmKdcAllocatePAC(
    PVMKDC_CONTEXT pContext,
    PVMKDC_AUTHZ_PAC *ppPAC)
{
    DWORD dwError = 0;
    PVMKDC_AUTHZ_PAC pPAC = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_AUTHZ_PAC),
                                  (PVOID *)&pPAC);
    BAIL_ON_VMKDC_ERROR(dwError);

    pPAC->ctx = (krb5_context)pContext->pGlobals->pKrb5Ctx;
    pPAC->pac = NULL;

    *ppPAC = pPAC;

error:

    return dwError;
}

DWORD
VmKdcAddPACInfoPAC(
    PVMKDC_AUTHZ_PAC pPAC,
    PVMKDC_AUTHZ_PAC_INFO pPACInfo)
{
    DWORD dwError = 0;
    krb5_error_code sts = 0;
    krb5_data pac_data = {0};

    pac_data.length = VMKDC_GET_LEN_DATA(pPACInfo->pac_data);
    pac_data.data = VMKDC_GET_PTR_DATA(pPACInfo->pac_data);

    if (!pPAC->pac)
    {
        sts = krb5_heim_pac_init(pPAC->ctx,
                                 &pPAC->pac);
        if (sts)
        {
            dwError = ERROR_INVALID_PARAMETER;  /* TBD - get correct error code */
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    sts = krb5_heim_pac_add_buffer(
                  pPAC->ctx,
                  pPAC->pac,
                  pPACInfo->pac_type,
                  &pac_data);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;  /* TBD - get correct error code */
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:

    return dwError;
}
 
DWORD
VmKdcSignPAC(
    PVMKDC_AUTHZ_PAC pPAC,
    DWORD dwAuthTime, 
    PVMKDC_PRINCIPAL pPrincipal,
    PVMKDC_KEY pServerKey,
    PVMKDC_KEY pPrivateKey,
    PVMKDC_DATA *ppData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pData = NULL;
    time_t authtime = 0;
    krb5_principal_data principal_data;
    krb5_principal principal = &principal_data;
    krb5_keyblock server_key = {0};
    krb5_keyblock priv_key = {0};
    krb5_data data = {0};
    int i = 0;
    krb5_error_code sts = 0;

    authtime = dwAuthTime;

    principal->name.name_type = pPrincipal->type;
    principal->name.name_string.len = pPrincipal->numComponents;
    principal->name.name_string.val = malloc(sizeof(heim_general_string *)*principal->name.name_string.len);
    if (!principal->name.name_string.val)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    for (i=0; i<(int)principal->name.name_string.len; i++)
    {
        principal->name.name_string.val[i] = VMKDC_GET_PTR_DATA(pPrincipal->components[i]);
    }

    server_key.keytype = pServerKey->type;
    server_key.keyvalue.data = VMKDC_GET_PTR_DATA(pServerKey->data);
    server_key.keyvalue.length = VMKDC_GET_LEN_DATA(pServerKey->data);

    priv_key.keytype = pPrivateKey->type;
    priv_key.keyvalue.data = VMKDC_GET_PTR_DATA(pPrivateKey->data);
    priv_key.keyvalue.length = VMKDC_GET_LEN_DATA(pPrivateKey->data);

    sts = _krb5_heim_pac_sign(
                  pPAC->ctx,
                  pPAC->pac,
                  authtime,
                  principal,
                  &server_key,
                  &priv_key,
                  &data);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    dwError = VmKdcAllocateData(data.data,
                                data.length,
                                &pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppData = pData;

error:
    VMKDC_SAFE_FREE_MEMORY(principal->name.name_string.val);
    if (data.data)
    {
        free(data.data);
    }

    return dwError;
}
 
DWORD
VmKdcParsePAC(
    PVMKDC_AUTHZ_PAC pPAC,
    PVMKDC_DATA pData)
{
    DWORD dwError = 0;
    krb5_error_code sts = 0;

    sts = krb5_heim_pac_parse(
                  pPAC->ctx,
                  VMKDC_GET_PTR_DATA(pData),
                  VMKDC_GET_LEN_DATA(pData),
                  &pPAC->pac);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

error:

    return dwError;
}
 
DWORD
VmKdcVerifyPAC(
    PVMKDC_AUTHZ_PAC pPAC)
{
    return 0;
}
 
VOID
VmKdcDestroyPAC(
    PVMKDC_AUTHZ_PAC pPAC)
{
    if (pPAC)
    {
        if (pPAC->ctx && pPAC->pac)
        {
            krb5_heim_pac_free(pPAC->ctx,
                               pPAC->pac);
        }
        VMKDC_SAFE_FREE_MEMORY(pPAC);
    }
}


DWORD
VmKdcGetPACInfoPAC(
    PVMKDC_AUTHZ_PAC pPAC,
    VMKDC_AUTHZ_PAC_INFO_TYPE pacInfoType,
    PVMKDC_DATA *ppData)
{
    DWORD dwError = 0;
    krb5_error_code sts = 0;
    krb5_data data = {0};
    PVMKDC_DATA pData = NULL;

    sts = krb5_heim_pac_get_buffer(
                  pPAC->ctx,
                  pPAC->pac,
                  pacInfoType,
                  &data);
    if (sts)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    dwError = VmKdcAllocateData(data.data,
                                data.length,
                                &pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppData = pData;

error:

    return dwError;
}
 
DWORD
VmKdcAllocatePACInfo(
    VMKDC_AUTHZ_PAC_INFO_TYPE pacInfoType,
    PVMKDC_DATA pacInfoData,
    PVMKDC_AUTHZ_PAC_INFO *ppPACInfo)
{
    DWORD dwError = 0;
    PVMKDC_AUTHZ_PAC_INFO pPACInfo = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_AUTHZ_PAC_INFO),
                                  (PVOID *)&pPACInfo);
    BAIL_ON_VMKDC_ERROR(dwError);

    pPACInfo->pac_type = pacInfoType;

    dwError = VmKdcAllocateData(VMKDC_GET_PTR_DATA(pacInfoData),
                                VMKDC_GET_LEN_DATA(pacInfoData),
                                &pPACInfo->pac_data);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppPACInfo = pPACInfo;

error:

    return dwError;
}
 
VOID
VmKdcDestroyPACInfo(
    PVMKDC_AUTHZ_PAC_INFO pPACInfo)
{
    if (pPACInfo)
    {
        VMKDC_SAFE_FREE_DATA(pPACInfo->pac_data);
        VMKDC_SAFE_FREE_MEMORY(pPACInfo);
    }
}
