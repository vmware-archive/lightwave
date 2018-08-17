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

#include <includes.h>

static
int
VMCAOpenSSLGetNIDIndex(
    X509_NAME           *pszSubjName,
    DWORD               dwNIDType,
    int                 iPos
    );


DWORD
VMCAOpenSSLGetValuesFromSubjectName(
    PCSTR                           pszPKCS10Request,
    DWORD                           dwNIDType,
    PDWORD                          pdwNumValues,
    PSTR                            **pppszValues
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwIdx = 0;
    DWORD                           dwNumValues = 0;
    PSTR                            *ppszValues = NULL;
    PSTR                            *ppszValuesTemp = NULL;
    PSTR                            pszValueString = NULL;
    X509_REQ                        *pCSR = NULL;
    X509_NAME                       *pszSubjName = NULL;
    X509_NAME_ENTRY                 *pEntry = NULL;
    ASN1_STRING                     *pEntryAsn1 = NULL;
    int                             iPos = -1;
    size_t                          szNumDNs = 0;
    size_t                          szEntryLength = 0;

    if (IsNullOrEmptyString(pszPKCS10Request) ||
        !pdwNumValues ||
        !pppszValues)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAPEMToCSR(
                    pszPKCS10Request,
                    &pCSR);
    BAIL_ON_VMCA_ERROR(dwError);

    pszSubjName = X509_REQ_get_subject_name(pCSR);
    if(pszSubjName == NULL)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    szNumDNs = X509_NAME_entry_count(pszSubjName);
    if (szNumDNs == 0)
    {
        dwError = VMCA_INVALID_CSR_FIELD;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(PSTR) * (DWORD)szNumDNs,
                        (PVOID *)&ppszValuesTemp);
    BAIL_ON_VMCA_ERROR(dwError);

    for (;;)
    {
        iPos = VMCAOpenSSLGetNIDIndex(
                        pszSubjName,
                        dwNIDType,
                        iPos);
        if (iPos == -1)
        {
            break;
        }

        pEntry = X509_NAME_get_entry(
                        pszSubjName,
                        iPos);
        if (pEntry == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        pEntryAsn1 = X509_NAME_ENTRY_get_data(pEntry);
        if (pEntryAsn1 == NULL)
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        szEntryLength = ASN1_STRING_to_UTF8(
                            (unsigned char **)&pszValueString,
                            pEntryAsn1);
        if (!pszValueString || szEntryLength != strlen(pszValueString))
        {
            dwError = VMCA_CERT_DECODE_FAILURE;
            BAIL_ON_VMCA_ERROR(dwError);
        }

        dwError = VMCAAllocateStringA(
                            pszValueString,
                            &ppszValuesTemp[dwIdx]);
        BAIL_ON_VMCA_ERROR(dwError);
        ++dwIdx;
        ++dwNumValues;

        if (pszValueString)
        {
            OPENSSL_free(pszValueString);
            pszValueString = NULL;
        }
    }

    dwError = VMCACopyStringArrayA(
                        &ppszValues,
                        dwNumValues,
                        ppszValuesTemp,
                        (DWORD)szNumDNs);
    BAIL_ON_VMCA_ERROR(dwError);

    *pppszValues = ppszValues;
    *pdwNumValues = dwNumValues;


cleanup:

    if (pCSR)
    {
        X509_REQ_free(pCSR);
    }
    if (pszValueString)
    {
        OPENSSL_free(pszValueString);
    }
    VMCAFreeStringArrayA(ppszValuesTemp, szNumDNs);

    return dwError;

error:

    VMCAFreeStringArrayA(ppszValues, dwNumValues);
    if (pppszValues)
    {
        *pppszValues = NULL;
    }
    if (pdwNumValues)
    {
        *pdwNumValues = 0;
    }

    goto cleanup;
}

DWORD
VMCAOpenSSLGetSANEntries(
    PCSTR                           pszPKCS10Request,
    PDWORD                          pdwNumSANDNSEntries,
    PSTR                            **pppszSANDNSEntires,
    PDWORD                          pdwNumSANIPEntries,
    PSTR                            **pppszSANIPEntires
    )
{
    DWORD                           dwError = 0;
    DWORD                           dwIdx = 0;
    DWORD                           dwNumSANEntries = 0;
    DWORD                           dwNumSANDNSEntries = 0;
    DWORD                           dwNumSANIPEntries = 0;
    PSTR                            pszSANName = NULL;
    PSTR                            *ppszSANDNSEntriesTemp = NULL;
    PSTR                            *ppszSANIPEntriesTemp = NULL;
    PSTR                            *ppszSANDNSEntries = NULL;
    PSTR                            *ppszSANIPEntries = NULL;
    X509_REQ                        *pCSR = NULL;
    STACK_OF(X509_EXTENSION)        *pExts = NULL;
    GENERAL_NAMES                   *pSANNames = NULL;

    if (IsNullOrEmptyString(pszPKCS10Request) ||
        !pdwNumSANDNSEntries ||
        !pppszSANDNSEntires ||
        !pdwNumSANIPEntries ||
        !pppszSANIPEntires)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAPEMToCSR(
                    pszPKCS10Request,
                    &pCSR);
    BAIL_ON_VMCA_ERROR(dwError);

    pExts = X509_REQ_get_extensions(pCSR);
    if (pExts == NULL)
    {
        dwError = ERROR_SUCCESS;
        goto error;
    }

    pSANNames = X509V3_get_d2i(pExts, NID_subject_alt_name, NULL, NULL);
    if (pSANNames == NULL)
    {
        dwError = ERROR_SUCCESS;
        goto error;
    }

    dwNumSANEntries = sk_GENERAL_NAME_num(pSANNames);
    if (dwNumSANEntries == 0)
    {
        dwError = ERROR_SUCCESS;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                        sizeof(PSTR) * dwNumSANEntries,
                        (PVOID *)&ppszSANDNSEntriesTemp);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateMemory(
                        sizeof(PSTR) * dwNumSANEntries,
                        (PVOID *)&ppszSANIPEntriesTemp);
    BAIL_ON_VMCA_ERROR(dwError);

    for (dwIdx = 0; dwIdx < dwNumSANEntries; ++dwIdx)
    {
        const GENERAL_NAME *pCurrentName = sk_GENERAL_NAME_value(pSANNames, dwIdx);

        if (pCurrentName->type == GEN_DNS)
        {
            size_t length = ASN1_STRING_to_UTF8(
                                    (unsigned char **)&pszSANName,
                                    (ASN1_IA5STRING *)pCurrentName->d.ptr);

            if (!pszSANName || length != strlen(pszSANName))
            {
                dwError = VMCA_CERT_DECODE_FAILURE;
                BAIL_ON_VMCA_ERROR(dwError);
            }

            dwError = VMCAAllocateStringA(
                                pszSANName,
                                &ppszSANDNSEntriesTemp[dwNumSANDNSEntries]);
            BAIL_ON_VMCA_ERROR(dwError);
            ++dwNumSANDNSEntries;
        }
        else if (pCurrentName->type == GEN_IPADD)
        {
            if (((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->length != 4)
            {
                dwError = VMCA_ERROR_SAN_IPADDR_INVALID;
                BAIL_ON_VMCA_ERROR(dwError);
            }

            dwError = VMCAAllocateStringPrintfA(
                            &pszSANName,
                            "%u.%u.%u.%u",
                            ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[0],
                            ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[1],
                            ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[2],
                            ((ASN1_OCTET_STRING *)pCurrentName->d.ptr)->data[3]);
            BAIL_ON_VMCA_ERROR(dwError);

            dwError = VMCAAllocateStringA(
                                pszSANName,
                                &ppszSANIPEntriesTemp[dwNumSANIPEntries]);
            BAIL_ON_VMCA_ERROR(dwError);
            ++dwNumSANIPEntries;
        }

        if (pszSANName)
        {
            OPENSSL_free(pszSANName);
            pszSANName = NULL;
        }
    }

    if (ppszSANDNSEntriesTemp && dwNumSANDNSEntries)
    {
        dwError = VMCACopyStringArrayA(
                            &ppszSANDNSEntries,
                            dwNumSANDNSEntries,
                            ppszSANDNSEntriesTemp,
                            dwNumSANEntries);
        BAIL_ON_VMCA_ERROR(dwError);

        *pdwNumSANDNSEntries = dwNumSANDNSEntries;
        *pppszSANDNSEntires = ppszSANDNSEntries;
    }

    if (ppszSANIPEntriesTemp && dwNumSANIPEntries)
    {
        dwError = VMCACopyStringArrayA(
                            &ppszSANIPEntries,
                            dwNumSANIPEntries,
                            ppszSANIPEntriesTemp,
                            dwNumSANEntries);
        BAIL_ON_VMCA_ERROR(dwError);

        *pdwNumSANIPEntries = dwNumSANIPEntries;
        *pppszSANIPEntires = ppszSANIPEntries;
    }


cleanup:

    if (pszSANName)
    {
        OPENSSL_free(pszSANName);
        pszSANName = NULL;
    }
    if (pSANNames)
    {
        sk_GENERAL_NAME_pop_free(pSANNames, GENERAL_NAME_free);
        pSANNames = NULL;
    }
    if (pExts)
    {
        sk_GENERAL_NAME_pop_free(pExts, X509_EXTENSION_free);
        pExts = NULL;
    }
    if (pCSR)
    {
        X509_REQ_free(pCSR);
        pCSR = NULL;
    }
    VMCAFreeStringArrayA(ppszSANDNSEntriesTemp, dwNumSANEntries);
    VMCAFreeStringArrayA(ppszSANIPEntriesTemp, dwNumSANEntries);

    return dwError;

error:

    if (pdwNumSANDNSEntries)
    {
        *pdwNumSANDNSEntries = 0;
    }
    VMCAFreeStringArrayA(ppszSANDNSEntries, dwNumSANDNSEntries);
    if (pppszSANDNSEntires)
    {
        *pppszSANDNSEntires = NULL;
    }
    if (pdwNumSANIPEntries)
    {
        *pdwNumSANIPEntries = 0;
    }
    VMCAFreeStringArrayA(ppszSANIPEntries, dwNumSANIPEntries);
    if (pppszSANIPEntires)
    {
        *pppszSANIPEntires = NULL;
    }

    goto cleanup;
}


static
int
VMCAOpenSSLGetNIDIndex(
    X509_NAME   *pszSubjName,
    DWORD       dwNIDType,
    int         iPosIn
    )
{
    int         iPosOut = 0;

    switch (dwNIDType) {
    case VMCA_OPENSSL_NID_O:
        iPosOut = X509_NAME_get_index_by_NID(
                        pszSubjName,
                        NID_organizationName,
                        iPosIn);
        break;
    case VMCA_OPENSSL_NID_CN:
        iPosOut = X509_NAME_get_index_by_NID(
                        pszSubjName,
                        NID_commonName,
                        iPosIn);
        break;
    default:
        iPosOut = -1;
    }

    return iPosOut;
}
