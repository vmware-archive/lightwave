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
VmKdcFreeEtypeInfo2Entry(
    PVMKDC_ETYPE_INFO2_ENTRY pEtypeInfo2Entry)
{
    if (pEtypeInfo2Entry)
    {
        VMKDC_SAFE_FREE_SALT(pEtypeInfo2Entry->salt);
        VMKDC_SAFE_FREE_DATA(pEtypeInfo2Entry->s2kparams);
        VMKDC_SAFE_FREE_MEMORY(pEtypeInfo2Entry);
    }
}

VOID
VmKdcFreeEtypeInfo2(
    PVMKDC_ETYPE_INFO2 pEtypeInfo2)
{
    DWORD i = 0;

    if (pEtypeInfo2)
    {
        if (pEtypeInfo2->entry)
        {
            for (i=0; i<pEtypeInfo2->length; i++)
            {
                VMKDC_SAFE_FREE_ETYPE_INFO2_ENTRY(pEtypeInfo2->entry[i]);
            }
            VMKDC_SAFE_FREE_MEMORY(pEtypeInfo2->entry);
        }
        VMKDC_SAFE_FREE_MEMORY(pEtypeInfo2);
    }
}

DWORD
VmKdcMakeEtypeInfo2Entry(
    VMKDC_ENCTYPE etype, 
    PVMKDC_SALT salt, 
    PVMKDC_DATA s2kparams, 
    PVMKDC_ETYPE_INFO2_ENTRY *ppRetEtypeInfo2Entry)
{
    DWORD dwError = 0;
    PVMKDC_ETYPE_INFO2_ENTRY pEtypeInfo2Entry = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ETYPE_INFO2_ENTRY),
                                  (PVOID*)&pEtypeInfo2Entry);
    BAIL_ON_VMKDC_ERROR(dwError);

    pEtypeInfo2Entry->etype = etype;

    if (salt)
    {
        if (salt->data)
        {
            dwError = VmKdcMakeSalt(salt->type,
                                    VMKDC_GET_LEN_DATA(salt->data),
                                    VMKDC_GET_PTR_DATA(salt->data),
                                    &pEtypeInfo2Entry->salt);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
        else
        {
            dwError = VmKdcMakeSalt(salt->type,
                                    0,
                                    NULL,
                                    &pEtypeInfo2Entry->salt);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    if (s2kparams)
    {
        dwError = VmKdcAllocateData(VMKDC_GET_PTR_DATA(s2kparams),
                                    VMKDC_GET_LEN_DATA(s2kparams),
                                    &pEtypeInfo2Entry->s2kparams);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    *ppRetEtypeInfo2Entry = pEtypeInfo2Entry;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ETYPE_INFO2_ENTRY(pEtypeInfo2Entry);
    }

    return dwError;
}

DWORD
VmKdcEncodeEtypeInfo2(
    PVMKDC_ETYPE_INFO2 pEtypeInfo2,
    PVMKDC_DATA *ppRetData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;
    ETYPE_INFO2 heimInfo;
    unsigned char *infoBufPtr = NULL;
    size_t heimInfoLen = 0;
    size_t infoBufLen = 0;
    int err = 0;
    DWORD i = 0;

    memset(&heimInfo, 0, sizeof(heimInfo));

    dwError = VmKdcAllocateMemory(pEtypeInfo2->length * sizeof(ETYPE_INFO2_ENTRY),
                                  (PVOID*)&heimInfo.val);
    BAIL_ON_VMKDC_ERROR(dwError);
    heimInfo.len = pEtypeInfo2->length;

    for (i=0; i<pEtypeInfo2->length; i++)
    {
        /* etype */
        heimInfo.val[i].etype = pEtypeInfo2->entry[i]->etype;

        /* salt (optional) */
        if (pEtypeInfo2->entry[i]->salt && pEtypeInfo2->entry[i]->salt->data)
        {
            heimInfo.val[i].salt = VMKDC_GET_PTR_DATA(pEtypeInfo2->entry[i]->salt->data);
        }

        /* s2kparams (optional) */
        if (pEtypeInfo2->entry[i]->s2kparams)
        {
            dwError = VmKdcAllocateMemory(sizeof(*heimInfo.val[i].s2kparams),
                                          (PVOID*)&heimInfo.val[i].s2kparams);
            BAIL_ON_VMKDC_ERROR(dwError);
            heimInfo.val[i].s2kparams->length = VMKDC_GET_LEN_DATA(pEtypeInfo2->entry[i]->s2kparams);
            heimInfo.val[i].s2kparams->data = VMKDC_GET_PTR_DATA(pEtypeInfo2->entry[i]->s2kparams);
        }
    }

    /*
     * Encode the ETYPE-INFO2 into Heimdal structure
     */
    ASN1_MALLOC_ENCODE(ETYPE_INFO2,
                       infoBufPtr, infoBufLen,
                       &heimInfo, &heimInfoLen,
                       err);
    if (err != 0 || infoBufLen != heimInfoLen)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(infoBufPtr, (int) infoBufLen, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }
    for (i=0; i<pEtypeInfo2->length; i++)
    {
        VMKDC_SAFE_FREE_MEMORY(heimInfo.val[i].s2kparams);
    }
    VMKDC_SAFE_FREE_MEMORY(heimInfo.val);
    if (infoBufPtr)
    {
        free(infoBufPtr);
        infoBufPtr = NULL;
    }
    return dwError;
}

VOID
VmKdcPrintEtypeInfo2Entry(
    PVMKDC_ETYPE_INFO2_ENTRY pEtypeInfo2Entry)
{
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintEtypeInfo2Entry: etype <%d>", pEtypeInfo2Entry->etype);
    if (pEtypeInfo2Entry->salt)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintEtypeInfo2Entry: salt <%d>", pEtypeInfo2Entry->salt);
    }
    if (pEtypeInfo2Entry->s2kparams)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintEtypeInfo2Entry: s2kparams:");
        VmKdcPrintData(pEtypeInfo2Entry->s2kparams);
    }
}

VOID
VmKdcPrintEtypeInfo2(
    PVMKDC_ETYPE_INFO2 pEtypeInfo2)
{
    DWORD i = 0;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintEtypeInfo2: length=%d", pEtypeInfo2->length);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintEtypeInfo2: entry=%p", pEtypeInfo2->entry);

    for (i=0; i<pEtypeInfo2->length; i++)
    {
        VmKdcPrintEtypeInfo2Entry(pEtypeInfo2->entry[i]);
    }
}
