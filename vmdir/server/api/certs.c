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
VmDirGetCACerts(
    PVDIR_CONNECTION pConn,
    PCSTR pszDomainDN,
    PCSTR pszCACN, /* optional */
    BOOL  bDetail,
    PVMDIR_CA_CERT_ARRAY* ppCACertificates
    )
{
    DWORD dwError = 0;
    PCSTR pszClassFilter = "(objectClass=vmwCertificationAuthority)";
    PCSTR pszAttrEntryDN = "entryDN";
    PCSTR pszAttrCADN    = "cACertificateDN";
    PCSTR pszAttrCert    = "cACertificate";
    PCSTR pszAttrCrl     = "certificateRevocationList";
    PCSTR pszFilter      = NULL;
    PSTR pszComboFilter = NULL;
    PSTR pszSearchBaseDN= NULL;
    PVMDIR_CA_CERT_ARRAY pCertArray = NULL;
    PVDIR_OPERATION pSearchOp = NULL;
    PVDIR_FILTER pFilter = NULL;
    PVDIR_BERVALUE pbvAttrs = NULL;

    PCSTR attrs[] = { pszAttrEntryDN, pszAttrCADN, pszAttrCert, pszAttrCrl};
    int nCertCount = 0;
    int certIndex = 0;
    DWORD dwAttrCount = 0;
    DWORD dwIndex = 0;

    if (!pConn ||
        IsNullOrEmptyString(pszDomainDN) ||
        !ppCACertificates)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_SEARCH, pConn, &pSearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchOp->protocol = VDIR_OPERATION_PROTOCOL_REST;

    dwError = VmDirAllocateMemory(
                    sizeof(VMDIR_CA_CERT_ARRAY),
                    (PVOID*)&pCertArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                 &pszSearchBaseDN,
                 "cn=Configuration,%s",
                 pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszCACN)
    {
        dwError = VmDirAllocateStringPrintf(
                &pszComboFilter, "(&(CN=%s)%s)",
                pszCACN, pszClassFilter);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszFilter = pszComboFilter;
    }
    else
    {
        pszFilter = pszClassFilter;
    }

    dwAttrCount = sizeof(attrs)/sizeof(attrs[0]);
    dwError = VmDirAllocateMemory(
                  sizeof(VDIR_BERVALUE)*(dwAttrCount+1),
                  (PVOID*)&pbvAttrs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (; dwIndex < dwAttrCount; ++dwIndex)
    {
        dwError = VmDirStringToBervalContent(
                      attrs[dwIndex],
                      &pbvAttrs[dwIndex]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = StrFilterToFilter(pszFilter, &pFilter);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* populate pSearchOp */
    dwError = VmDirStringToBervalContent(pszSearchBaseDN, &pSearchOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSearchOp->request.searchReq.scope = LDAP_SCOPE_SUBTREE;

    pSearchOp->request.searchReq.filter = pFilter;
    pFilter = NULL;

    pSearchOp->request.searchReq.bStoreRsltInMem = TRUE;

    pSearchOp->request.searchReq.attrs = pbvAttrs;
    pbvAttrs = NULL;

    dwError = VmDirMLSearch(pSearchOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    nCertCount = pSearchOp->internalSearchEntryArray.iSize;
    if (nCertCount > 0)
    {
        pCertArray->dwCount = nCertCount;

        dwError = VmDirAllocateMemory(
                        sizeof(VMDIR_CA_CERT) * nCertCount,
                        (PVOID*)&pCertArray->pCACerts);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (certIndex=0; certIndex < nCertCount; certIndex++)
        {
            // The following assumes there's only one certificate for each CA
            // object. In the future if the whole chain is store, we will
            // update accordingly.

            // Copy CN
            dwError = VmDirCopySingleAttributeString(
                    &pSearchOp->internalSearchEntryArray.pEntry[certIndex],
                    pszAttrEntryDN, FALSE,
                    (PSTR*)&pCertArray->pCACerts[certIndex].pCN);
            BAIL_ON_VMDIR_ERROR( dwError );

            // Copy subject DN
            dwError = VmDirCopySingleAttributeString(
                    &pSearchOp->internalSearchEntryArray.pEntry[certIndex],
                    pszAttrCADN, FALSE,
                    (PSTR*)&pCertArray->pCACerts[certIndex].pSubjectDN);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (bDetail)
            {
                // Copy certificate
                dwError = VmDirCopySingleAttributeString(
                        &pSearchOp->internalSearchEntryArray.pEntry[certIndex],
                        pszAttrCert, FALSE,
                        (PSTR*)&pCertArray->pCACerts[certIndex].pCert);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirCopySingleAttributeString(
                        &pSearchOp->internalSearchEntryArray.pEntry[certIndex],
                        pszAttrCrl, TRUE,
                        (PSTR*)&pCertArray->pCACerts[certIndex].pCrl);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    *ppCACertificates = pCertArray;

cleanup:

    VmDirFreeOperation(pSearchOp);
    DeleteFilter(pFilter);

    VmDirFreeBervalArrayContent(pbvAttrs, dwAttrCount);
    VMDIR_SAFE_FREE_MEMORY(pbvAttrs);

    VMDIR_SAFE_FREE_MEMORY(pszSearchBaseDN);
    VMDIR_SAFE_FREE_MEMORY(pszComboFilter);

    return dwError;

error:

    if (pCertArray )
    {
        VmDirFreeCACertArray(pCertArray);
    }
    if (ppCACertificates)
    {
        *ppCACertificates = NULL;
    }

    goto cleanup;
}


VOID
VmDirFreeCACertArray(
    PVMDIR_CA_CERT_ARRAY pArray
    )
{
    unsigned int uCounter = 0;
    if (pArray)
    {
        if (pArray->pCACerts)
        {
            for(uCounter = 0; uCounter < pArray->dwCount; uCounter ++)
            {
                PVMDIR_CA_CERT pCursor = &pArray->pCACerts[uCounter];

                VMDIR_SAFE_FREE_MEMORY(pCursor->pCN);
                VMDIR_SAFE_FREE_MEMORY(pCursor->pSubjectDN);
                VMDIR_SAFE_FREE_MEMORY(pCursor->pCert);
                VMDIR_SAFE_FREE_MEMORY(pCursor->pCrl);
            }
            VmDirFreeMemory(pArray->pCACerts);
        }
        VmDirFreeMemory(pArray);
    }
}
