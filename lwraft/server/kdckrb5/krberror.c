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
VmKdcFreeKrbError(
    IN PVMKDC_KRB_ERROR pKrbError)
{
    if (pKrbError)
    {
        VMKDC_SAFE_FREE_DATA(pKrbError->crealm);
        VMKDC_SAFE_FREE_PRINCIPAL(pKrbError->cname);
        VMKDC_SAFE_FREE_DATA(pKrbError->realm);
        VMKDC_SAFE_FREE_PRINCIPAL(pKrbError->sname);
        VMKDC_SAFE_FREE_DATA(pKrbError->e_text);
        VMKDC_SAFE_FREE_DATA(pKrbError->e_data);
    }
    VMKDC_SAFE_FREE_MEMORY(pKrbError);
}

DWORD
VmKdcMakeKrbError(
    IN int                       pvno,
    IN OPTIONAL time_t           *ctime,
    IN time_t                    stime,
    IN VMKDC_KRB_ERR             error_code,
    IN OPTIONAL PCSTR            crealm,
    IN OPTIONAL PVMKDC_PRINCIPAL cname,
    IN PCSTR                     realm,
    IN PVMKDC_PRINCIPAL          sname,
    IN OPTIONAL PVMKDC_DATA      e_text,
    IN OPTIONAL PVMKDC_DATA      e_data,
    OUT PVMKDC_KRB_ERROR         *ppRetKrbError)
{
    DWORD dwError = 0;
    PVMKDC_KRB_ERROR pKrbError = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_KRB_ERROR), (PVOID*)&pKrbError);
    BAIL_ON_VMKDC_ERROR(dwError);

    pKrbError->pvno = pvno;
    pKrbError->msg_type = VMKDC_MESSAGE_TYPE_KRB_ERROR;
    pKrbError->ctime = ctime;
    pKrbError->cusec = 0;
    pKrbError->stime = stime;
    pKrbError->susec = 0;
    pKrbError->error_code = error_code;

    if (crealm)
    {
        dwError = VmKdcAllocateDataString(crealm, &pKrbError->crealm);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (cname)
    {
        dwError = VmKdcCopyPrincipal(cname, &pKrbError->cname);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (realm)
    {
        dwError = VmKdcAllocateDataString(realm, &pKrbError->realm);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (sname)
    {
        dwError = VmKdcCopyPrincipal(sname, &pKrbError->sname);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (e_text)
    {
        dwError = VmKdcAllocateData(VMKDC_GET_PTR_DATA(e_text),
                                    VMKDC_GET_LEN_DATA(e_text),
                                    &pKrbError->e_text);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    if (e_data)
    {
        dwError = VmKdcAllocateData(VMKDC_GET_PTR_DATA(e_data),
                                    VMKDC_GET_LEN_DATA(e_data),
                                    &pKrbError->e_data);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_KRB_ERROR(pKrbError);
    }
    *ppRetKrbError = pKrbError;
    return dwError;
}

DWORD
VmKdcBuildKrbErrorEData(
    IN  PVMKDC_KEY pKey,
    OUT PVMKDC_DATA *ppRetData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pData = NULL;
    PVMKDC_ETYPE_INFO2_ENTRY pEtypeInfo2Entry = NULL;
    PVMKDC_ETYPE_INFO2 pEtypeInfo2 = NULL;
    PVMKDC_METHOD_DATA pMethodData = NULL;
    PVMKDC_DATA pEtypeInfo2Data = NULL;
    PVMKDC_PADATA pPaDataEncTimestamp = NULL;
    PVMKDC_PADATA pPaDataEtypeInfo2 = NULL;

    /*
     * Construct an ETYPE-INFO2 preauth data
     */
    dwError = VmKdcMakeEtypeInfo2Entry(pKey->type, NULL, NULL, &pEtypeInfo2Entry);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ETYPE_INFO2),
                                  (PVOID*)&pEtypeInfo2);
    BAIL_ON_VMKDC_ERROR(dwError);

    pEtypeInfo2->length = 1;
    dwError = VmKdcAllocateMemory(sizeof(PVMKDC_ETYPE_INFO2_ENTRY) * pEtypeInfo2->length,
                                  (PVOID*)&pEtypeInfo2->entry);
    BAIL_ON_VMKDC_ERROR(dwError);

    pEtypeInfo2->entry[0] = pEtypeInfo2Entry;

    /*
     * Encode the ETYPE-INFO2 preauth data
     */
    dwError = VmKdcEncodeEtypeInfo2(pEtypeInfo2, &pEtypeInfo2Data);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcMakePaData(VMKDC_PADATA_ETYPE_INFO2,
                              VMKDC_GET_LEN_DATA(pEtypeInfo2Data),
                              VMKDC_GET_PTR_DATA(pEtypeInfo2Data),
                              &pPaDataEtypeInfo2);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Construct a ENC-TIMESTAMP preauth data
     */
    dwError = VmKdcMakePaData(VMKDC_PADATA_ENC_TIMESTAMP,
                              0,
                              NULL,
                              &pPaDataEncTimestamp);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Construct the METHOD-DATA sequence
     */
    dwError = VmKdcAllocateMemory(sizeof(VMKDC_METHOD_DATA),
                                  (PVOID*)&pMethodData);
    BAIL_ON_VMKDC_ERROR(dwError);

    pMethodData->length = 2;
    dwError = VmKdcAllocateMemory(sizeof(PVMKDC_PADATA) * pMethodData->length,
                                  (PVOID*)&pMethodData->padata);
    BAIL_ON_VMKDC_ERROR(dwError);

    pMethodData->padata[0] = pPaDataEncTimestamp;
    pMethodData->padata[1] = pPaDataEtypeInfo2;

    /*
     * Encode the METHOD-DATA sequence
     */
    dwError = VmKdcEncodeMethodData(pMethodData, &pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pData);
    }
    VMKDC_SAFE_FREE_ETYPE_INFO2(pEtypeInfo2);
    VMKDC_SAFE_FREE_METHOD_DATA(pMethodData);
    VMKDC_SAFE_FREE_DATA(pEtypeInfo2Data);

    return dwError;
}

DWORD
VmKdcEncodeKrbError(
    IN PVMKDC_KRB_ERROR pKrbError,
    OUT PVMKDC_DATA     *ppRetData)
{
    KRB_ERROR hErr = {0};
    PVMKDC_DATA pAsnData = NULL;
    size_t lenAlloc = 0;
    UCHAR *encErrBuf = NULL;
    size_t encErrBufLen = 0;
    int err = 0;
    DWORD dwError = 0;
    DWORD i = 0;
    Realm heimRealm = NULL;

    hErr.pvno = pKrbError->pvno;
    hErr.msg_type = pKrbError->msg_type;
    if (pKrbError->ctime)
    {
        /* ctime */
        hErr.ctime = (KerberosTime *)pKrbError->ctime;

        /* cusec */
        hErr.cusec = (krb5int32 *)&pKrbError->cusec;
    }

    /* stime */
    hErr.stime = pKrbError->stime;

    /* susec */
    hErr.susec = pKrbError->susec;

    /* error_code */
    hErr.error_code = pKrbError->error_code;

    /* crealm */
    if (pKrbError->crealm)
    {
        heimRealm = (Realm)VMKDC_GET_PTR_DATA(pKrbError->crealm);
        hErr.crealm = &heimRealm;
    }

    /* cname */
    if (pKrbError->cname)
    {
        hErr.cname = malloc(sizeof(*hErr.cname));
        if (!hErr.cname)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        hErr.cname->name_type = pKrbError->cname->type;
        hErr.cname->name_string.len = pKrbError->cname->numComponents;
        hErr.cname->name_string.val = malloc(sizeof(heim_general_string *)*hErr.cname->name_string.len);
        if (!hErr.cname->name_string.val)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        for (i=0; i<hErr.cname->name_string.len; i++)
        {
            hErr.cname->name_string.val[i] = VMKDC_GET_PTR_DATA(pKrbError->cname->components[i]);
        }
    }

    /* realm */
    if (pKrbError->realm)
    {
        hErr.realm = VMKDC_GET_PTR_DATA(pKrbError->realm);
    }

    /* sname */
    if (pKrbError->sname)
    {
        hErr.sname.name_type = pKrbError->sname->type;
        hErr.sname.name_string.len = pKrbError->sname->numComponents;
        hErr.sname.name_string.val = malloc(sizeof(heim_general_string *)*hErr.sname.name_string.len);
        if (!hErr.sname.name_string.val)
        {
            dwError = ERROR_NO_MEMORY;
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        for (i=0; i<hErr.sname.name_string.len; i++)
        {
            hErr.sname.name_string.val[i] = VMKDC_GET_PTR_DATA(pKrbError->sname->components[i]);
        }
    }

    /* e_text */
    if (pKrbError->e_text)
    {
        hErr.e_text = VMKDC_GET_PTR_DATA(pKrbError->e_text);
    }

    /* e_data */
    if (pKrbError->e_data)
    {
        dwError = VmKdcAllocateMemory(sizeof(*hErr.e_data), (PVOID*)&hErr.e_data);
        BAIL_ON_VMKDC_ERROR(dwError);

        hErr.e_data->data = VMKDC_GET_PTR_DATA(pKrbError->e_data);
        hErr.e_data->length = VMKDC_GET_LEN_DATA(pKrbError->e_data);
    }

    ASN1_MALLOC_ENCODE(KRB_ERROR, encErrBuf, lenAlloc, &hErr, &encErrBufLen, err);
    if (err != 0 || lenAlloc != encErrBufLen)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(encErrBuf, (DWORD) encErrBufLen, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }
    if (hErr.cname && hErr.cname->name_string.val)
    {
        free(hErr.cname->name_string.val);
        hErr.cname->name_string.val = NULL;
        free(hErr.cname);
        hErr.cname = NULL;
    }
    if (hErr.sname.name_string.val)
    {
        free(hErr.sname.name_string.val);
        hErr.sname.name_string.val = NULL;
    }
    if (encErrBuf)
    {
        free(encErrBuf);
        encErrBuf = NULL;
    }
    VMKDC_SAFE_FREE_MEMORY(hErr.e_data);

    return dwError;
}

DWORD
VmKdcBuildKrbError(
    IN int                       pvno,
    IN OPTIONAL time_t           *ctime,
    IN time_t                    stime,
    IN VMKDC_KRB_ERR             error_code,
    IN OPTIONAL PCSTR            crealm,
    IN OPTIONAL PVMKDC_PRINCIPAL cname,
    IN PCSTR                     realm,
    IN PVMKDC_PRINCIPAL          sname,
    IN OPTIONAL PVMKDC_DATA      e_text,
    IN OPTIONAL PVMKDC_DATA      e_data,
    OUT PVMKDC_DATA              *ppRetData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pData = NULL;
    PVMKDC_KRB_ERROR pKrbError = NULL;

    dwError = VmKdcMakeKrbError(
                  pvno,
                  ctime,
                  stime,
                  error_code,
                  crealm,
                  cname,
                  realm,
                  sname,
                  e_text,
                  e_data,
                  &pKrbError);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Encode the KRB-ERROR message
     */
    dwError = VmKdcEncodeKrbError(
                  pKrbError,
                  &pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pData);
    }
    VMKDC_SAFE_FREE_KRB_ERROR(pKrbError);

    return dwError;
}

VOID
VmKdcPrintKrbError(
    IN PVMKDC_KRB_ERROR pKrbError)
{
    CHAR timeFmtBuf[32];

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: pvno <%d>",
             pKrbError->pvno);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: msg_type <%d>",
             pKrbError->msg_type);
    if (pKrbError->ctime)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: ctime <%s>",
                 VmKdcCtimeTS(pKrbError->ctime, timeFmtBuf));
    }
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: cusec <%d>",
             pKrbError->cusec);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: stime <%s>",
             VmKdcCtimeTS(&pKrbError->stime, timeFmtBuf));
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: susec <%d>",
             pKrbError->susec);
    if (pKrbError->crealm)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: crealm <%s>",
                 VMKDC_GET_PTR_DATA(pKrbError->crealm));
    }
    if (pKrbError->cname)
    {
        VmKdcPrintPrincipal(pKrbError->cname);
    }
    if (pKrbError->realm)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: realm <%s>",
                 VMKDC_GET_PTR_DATA(pKrbError->realm));
    }
    if (pKrbError->sname)
    {
        VmKdcPrintPrincipal(pKrbError->sname);
    }
    if (pKrbError->e_text)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintKrbError: e_text <%s>",
                 VMKDC_GET_PTR_DATA(pKrbError->e_text));
    }
    if (pKrbError->e_data)
    {
        VmKdcPrintData(pKrbError->e_data);
    }
}
