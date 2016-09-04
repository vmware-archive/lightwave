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
 * Module Name: Directory Schema
 *
 * Filename: matchingrule.c
 *
 * Abstract:
 *
 * Matching rule and normalize function
 *
 */

#include "includes.h"

static
DWORD
VdirEqualityMatchingRuleLoad(
    VOID
    );

static
DWORD
VdirOrderingMatchingRuleLoad(
    VOID
    );

static
DWORD
VdirSubstrMatchingRuleLoad(
    VOID
    );

static
int
matchingrulePSyntaxOidCmp(
    const void *p1,
    const void *p2
    );

static
DWORD
VdirSchemaInPlaceDN2RDNs(
    PSTR    pszDN,
    PSTR**  pppszRDNs,
    int*    piNumRDNs
    );

static
DWORD
normalizeString(
    BOOLEAN     bIsCaseIgnore,
    PVDIR_BERVALUE     pBerv
    );

static
DWORD
normalizeCaseExact(
    PVDIR_BERVALUE     pBerv
    );

static
DWORD
normalizeCaseIgnore(
    PVDIR_BERVALUE     pBerv
    );

static
BOOLEAN
compareString(
    VDIR_SCHEMA_MATCH_TYPE type,
    PVDIR_BERVALUE    pAssert,
    PVDIR_BERVALUE    pBerv
    );

static
BOOLEAN
compareSubString(
    VDIR_SCHEMA_MATCH_TYPE type,
    PVDIR_BERVALUE    pAssert,
    PVDIR_BERVALUE    pBerv
    );

static
BOOLEAN
compareIntegerString(
    VDIR_SCHEMA_MATCH_TYPE type,
    PVDIR_BERVALUE    pAssert,
    PVDIR_BERVALUE    pBerv
    );

/*
 * We only support static list of matching rule definitions.
 * Call this after VdirSynatxLoad.
 */
DWORD
VdirMatchingRuleLoad(
    VOID
    )
{
    DWORD    dwError = 0;

    dwError = VdirEqualityMatchingRuleLoad();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirOrderingMatchingRuleLoad();
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirSubstrMatchingRuleLoad();
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
VdirEqualityMatchingRuleLoad(
    VOID
    )
{
    DWORD    dwError = 0;
    DWORD    dwCnt = 0;

    static VDIR_MATCHING_RULE_DESC gEqualityMatchingRuleTbl[] =
            VDIR_EQAULITY_MATCHING_RULE_INIT_TABLE_INITIALIZER;

    if (gVdirMatchingRuleGlobals.pEqualityMatchingRule)
    {
        return 0;
    }

    gVdirMatchingRuleGlobals.pEqualityMatchingRule = &gEqualityMatchingRuleTbl[0];
    gVdirMatchingRuleGlobals.usEqualityMRSize = sizeof(gEqualityMatchingRuleTbl)/sizeof(gEqualityMatchingRuleTbl[0]);

    qsort(gVdirMatchingRuleGlobals.pEqualityMatchingRule,
            gVdirMatchingRuleGlobals.usEqualityMRSize,
            sizeof(VDIR_MATCHING_RULE_DESC),
            matchingrulePSyntaxOidCmp);

    for (dwCnt = 0; dwCnt < gVdirMatchingRuleGlobals.usEqualityMRSize; dwCnt++)
    {
        PSTR pszOid = gVdirMatchingRuleGlobals.pEqualityMatchingRule[dwCnt].pszSyntaxOid;
        PVDIR_SYNTAX_DESC pSyntax = NULL;

        dwError = VdirSyntaxLookupByOid(pszOid, &pSyntax);
        BAIL_ON_VMDIR_ERROR(dwError);

        gVdirMatchingRuleGlobals.pEqualityMatchingRule[dwCnt].pSyntax = pSyntax;
    }

#ifdef LDAP_DEBUG
    VmDirLog( LDAP_DEBUG_TRACE, "Supported Equality MatchingRules" );
    for (dwCnt=0; dwCnt<gVdirMatchingRuleGlobals.usSize; dwCnt++)
    {
        VmDirLog( LDAP_DEBUG_TRACE, "[%3d][%28s](%s)\n", dwCnt,
                gVdirMatchingRuleGlobals.pEqualityMatchingRule[dwCnt].pszName,
                gVdirMatchingRuleGlobals.pEqualityMatchingRule[dwCnt].pszOid);
    }
#endif

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VdirOrderingMatchingRuleLoad(
    VOID
    )
{
    DWORD    dwError = 0;
    DWORD    dwCnt = 0;

    static VDIR_MATCHING_RULE_DESC gOrderingMatchingRuleTbl[] =
            VDIR_ORDERING_MATCHING_RULE_INIT_TABLE_INITIALIZER;

    if (gVdirMatchingRuleGlobals.pOrderingMatchingRule)
    {
        return 0;
    }

    gVdirMatchingRuleGlobals.pOrderingMatchingRule = &gOrderingMatchingRuleTbl[0];
    gVdirMatchingRuleGlobals.usOrderingMRSize = sizeof(gOrderingMatchingRuleTbl)/sizeof(gOrderingMatchingRuleTbl[0]);

    qsort(gVdirMatchingRuleGlobals.pOrderingMatchingRule,
            gVdirMatchingRuleGlobals.usOrderingMRSize,
            sizeof(VDIR_MATCHING_RULE_DESC),
            matchingrulePSyntaxOidCmp);

    for (dwCnt = 0; dwCnt < gVdirMatchingRuleGlobals.usOrderingMRSize; dwCnt++)
    {
        PSTR pszOid = gVdirMatchingRuleGlobals.pOrderingMatchingRule[dwCnt].pszSyntaxOid;
        PVDIR_SYNTAX_DESC pSyntax = NULL;

        dwError = VdirSyntaxLookupByOid(pszOid, &pSyntax);
        BAIL_ON_VMDIR_ERROR(dwError);

        gVdirMatchingRuleGlobals.pOrderingMatchingRule[dwCnt].pSyntax = pSyntax;
    }

#ifdef LDAP_DEBUG
    VmDirLog( LDAP_DEBUG_TRACE, "Supported Ordering MatchingRules" );
    for (dwCnt=0; dwCnt<gVdirMatchingRuleGlobals.usOrderingMRSize; dwCnt++)
    {
        VmDirLog( LDAP_DEBUG_TRACE, "[%3d][%28s](%s)\n", dwCnt,
                gVdirMatchingRuleGlobals.pOrderingMatchingRule[dwCnt].pszName,
                gVdirMatchingRuleGlobals.pOrderingMatchingRule[dwCnt].pszOid);
    }
#endif

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
VdirSubstrMatchingRuleLoad(
    VOID
    )
{
    DWORD    dwError = 0;
    DWORD    dwCnt = 0;

    static VDIR_MATCHING_RULE_DESC gSubstrMatchingRuleTbl[] =
            VDIR_SUBSTR_MATCHING_RULE_INIT_TABLE_INITIALIZER;

    if (gVdirMatchingRuleGlobals.pSubstrMatchingRule)
    {
        return 0;
    }

    gVdirMatchingRuleGlobals.pSubstrMatchingRule = &gSubstrMatchingRuleTbl[0];
    gVdirMatchingRuleGlobals.usSubstrMRSize = sizeof(gSubstrMatchingRuleTbl)/sizeof(gSubstrMatchingRuleTbl[0]);

    qsort(gVdirMatchingRuleGlobals.pSubstrMatchingRule,
            gVdirMatchingRuleGlobals.usSubstrMRSize,
            sizeof(VDIR_MATCHING_RULE_DESC),
            matchingrulePSyntaxOidCmp);

    for (dwCnt = 0; dwCnt < gVdirMatchingRuleGlobals.usSubstrMRSize; dwCnt++)
    {
        PSTR pszOid = gVdirMatchingRuleGlobals.pSubstrMatchingRule[dwCnt].pszSyntaxOid;
        PVDIR_SYNTAX_DESC pSyntax = NULL;

        dwError = VdirSyntaxLookupByOid(pszOid, &pSyntax);
        BAIL_ON_VMDIR_ERROR(dwError);

        gVdirMatchingRuleGlobals.pSubstrMatchingRule[dwCnt].pSyntax = pSyntax;
    }

#ifdef LDAP_DEBUG
    VmDirLog( LDAP_DEBUG_TRACE, "Supported Substr MatchingRules" );
    for (dwCnt=0; dwCnt<gVdirMatchingRuleGlobals.usSubstrMRSize; dwCnt++)
    {
        VmDirLog( LDAP_DEBUG_TRACE, "[%3d][%28s](%s)\n", dwCnt,
                gVdirMatchingRuleGlobals.pSubstrMatchingRule[dwCnt].pszName,
                gVdirMatchingRuleGlobals.pSubstrMatchingRule[dwCnt].pszOid);
    }
#endif

cleanup:
    return dwError;

error:
    goto cleanup;
}

PVDIR_MATCHING_RULE_DESC
VdirEqualityMRLookupBySyntaxOid(
    PCSTR    pszSyntaxOid
    )
{
    PVDIR_MATCHING_RULE_DESC pMatchingRule = NULL;
    VDIR_MATCHING_RULE_DESC  key = {0};

    if (!pszSyntaxOid)
    {
        return NULL;
    }

    key.pszSyntaxOid = (PSTR)pszSyntaxOid;

    pMatchingRule = (PVDIR_MATCHING_RULE_DESC) bsearch(
            &key,
            gVdirMatchingRuleGlobals.pEqualityMatchingRule,
            gVdirMatchingRuleGlobals.usEqualityMRSize,
            sizeof(VDIR_MATCHING_RULE_DESC),
            matchingrulePSyntaxOidCmp);

    return pMatchingRule;
}

PVDIR_MATCHING_RULE_DESC
VdirOrderingMRLookupBySyntaxOid(
    PCSTR    pszSyntaxOid
    )
{
    PVDIR_MATCHING_RULE_DESC pMatchingRule = NULL;
    VDIR_MATCHING_RULE_DESC  key = {0};

    if (!pszSyntaxOid)
    {
        return NULL;
    }

    key.pszSyntaxOid = (PSTR)pszSyntaxOid;

    pMatchingRule = (PVDIR_MATCHING_RULE_DESC) bsearch(
            &key,
            gVdirMatchingRuleGlobals.pOrderingMatchingRule,
            gVdirMatchingRuleGlobals.usOrderingMRSize,
            sizeof(VDIR_MATCHING_RULE_DESC),
            matchingrulePSyntaxOidCmp);

    return pMatchingRule;
}

PVDIR_MATCHING_RULE_DESC
VdirSubstrMRLookupBySyntaxOid(
    PCSTR    pszSyntaxOid
    )
{
    PVDIR_MATCHING_RULE_DESC pMatchingRule = NULL;
    VDIR_MATCHING_RULE_DESC  key = {0};

    if (!pszSyntaxOid)
    {
        return NULL;
    }

    key.pszSyntaxOid = (PSTR)pszSyntaxOid;

    pMatchingRule = (PVDIR_MATCHING_RULE_DESC) bsearch(
            &key,
            gVdirMatchingRuleGlobals.pSubstrMatchingRule,
            gVdirMatchingRuleGlobals.usSubstrMRSize,
            sizeof(VDIR_MATCHING_RULE_DESC),
            matchingrulePSyntaxOidCmp);

    return pMatchingRule;
}

static
int
matchingrulePSyntaxOidCmp(
    const void *p1,
    const void *p2
    )
{
    PVDIR_MATCHING_RULE_DESC pDesc1 = (PVDIR_MATCHING_RULE_DESC) p1;
    PVDIR_MATCHING_RULE_DESC pDesc2 = (PVDIR_MATCHING_RULE_DESC) p2;

    if ((pDesc1 == NULL || pDesc1->pszSyntaxOid == NULL) &&
        (pDesc2 == NULL || pDesc2->pszSyntaxOid == NULL))
    {
        return 0;
    }

    if (pDesc1 == NULL || pDesc1->pszSyntaxOid == NULL)
    {
        return -1;
    }

    if (pDesc2 == NULL || pDesc2->pszSyntaxOid == NULL)
    {
        return 1;
    }

    return VmDirStringCompareA(pDesc1->pszSyntaxOid, pDesc2->pszSyntaxOid, TRUE);
}

/*
 * 1. remove extra spaces (i.e. collapse spaces into one)
 * 2. if bIsCaseIgnore, change to lower case
 * 3. if NO change in value, *ppOutBerv is NULL
 */
static
DWORD
normalizeString(
    BOOLEAN     bIsCaseIgnore,
    PVDIR_BERVALUE     pBerv
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;
    DWORD dwNewSize = 0;
    BOOLEAN bIsValueChanged = FALSE;

    char plocalBuf[128] = {0};
    char* pBuf = &plocalBuf[0];

    assert(pBerv);

    if (pBerv->bvnorm_val && pBerv->lberbv.bv_val != pBerv->bvnorm_val)
    {
        // cleanup bvnorm val/len
        VMDIR_SAFE_FREE_MEMORY(pBerv->bvnorm_val);
        pBerv->bvnorm_len = 0;
    }

    if (pBerv->lberbv.bv_len > 127)
    {
        dwError = VmDirAllocateMemory(
                sizeof(char) * pBerv->lberbv.bv_len + 1,
                (PVOID*)&pBuf);
    }

    for (dwCnt = 0; dwCnt < pBerv->lberbv.bv_len; dwCnt++)
    {
        if (! VDIR_ASCII_SPACE(pBerv->lberbv.bv_val[dwCnt]))
        {
            break;
        }
    }

    if (dwCnt != 0)
    {   // consolidate leading spaces to just one
        pBuf[dwNewSize++] = ' ';
    }

    for (;dwCnt < pBerv->lberbv.bv_len; dwCnt++)
    {
        if (VDIR_ASCII_SPACE(pBerv->lberbv.bv_val[dwCnt]))
        {
            if ((dwCnt < pBerv->lberbv.bv_len - 1 &&
                 !VDIR_ASCII_SPACE(pBerv->lberbv.bv_val[dwCnt+1])) ||
                (dwCnt == pBerv->lberbv.bv_len -1))
            {
                pBuf[dwNewSize] = pBerv->lberbv.bv_val[dwCnt];
                if (bIsCaseIgnore)
                {
                    VIDR_ASCII_UPPER_TO_LOWER(pBuf[dwNewSize], bIsValueChanged);
                }
                dwNewSize++;

            }
            // skip extra middle and trailing spaces
        }
        else
        {
            pBuf[dwNewSize] = pBerv->lberbv.bv_val[dwCnt];
            if (bIsCaseIgnore)
            {
                VIDR_ASCII_UPPER_TO_LOWER(pBuf[dwNewSize], bIsValueChanged);
            }
            dwNewSize++;
        }
    }

    if (dwNewSize == pBerv->lberbv.bv_len && !bIsValueChanged)
    {   // value == normalized value, point to self
        pBerv->bvnorm_val = pBerv->lberbv.bv_val;
        pBerv->bvnorm_len = pBerv->lberbv.bv_len;
    }
    else
    {
        dwError = VmDirAllocateStringA(
                pBuf,
                &pBerv->bvnorm_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pBerv->bvnorm_len = dwNewSize;
    }

cleanup:

    if (pBuf != &plocalBuf[0])
    {
        VMDIR_SAFE_FREE_MEMORY(pBuf);
    }

    return dwError;

error:

    goto cleanup;
}

// This is NOT RFC 4518 compliance.
static
DWORD
normalizeCaseExact(
    PVDIR_BERVALUE     pBerv
    )
{
    return normalizeString(FALSE, pBerv);
}

static
DWORD
normalizeCaseIgnore(
    PVDIR_BERVALUE     pBerv
    )
{
    return normalizeString(TRUE, pBerv);
}

/*
 * For given DN (pszDN content will be modified),
 * 1. allocate ppszRDNs to hold PSTR to RDN
 * 2. change pszDN ',' to '\0' and have ppszRDNs[i] point to it
 * 3. set *piNumRDNs
 *
 * Caller should free *pppszRDNs but NOT **pppszRDNs, which point into pszDN.
 */
static
DWORD
VdirSchemaInPlaceDN2RDNs(
    PSTR    pszDN,
    PSTR**  pppszRDNs,
    int*    piNumRDNs
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszRDNs = NULL;
    int     iCnt = 0;
    PSTR    pChar = NULL;

    assert(pppszRDNs && piNumRDNs);

    for (iCnt = 0, pChar = pszDN; pChar; iCnt++)
    {
        pChar = VmDirStringChrA(pChar+1, ',');
    }

    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * (iCnt+1),
            (PVOID)&ppszRDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    *piNumRDNs = iCnt;

    for (iCnt = 0, pChar = pszDN, ppszRDNs[0] = pszDN; pChar; iCnt++)
    {
        pChar = VmDirStringChrA(pChar+1, ',');
        if (pChar)
        {
            *pChar = '\0';
            ppszRDNs[iCnt+1] = pChar+1;
        }
    }

    *pppszRDNs = ppszRDNs;

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(ppszRDNs);

    *pppszRDNs = NULL;
    *piNumRDNs = 0;

    goto cleanup;
}

DWORD
VmDirNormalizeDNWrapper(
    PVDIR_BERVALUE      pBerv
    )
{
    return VmDirNormalizeDN(pBerv, NULL);
}

/*
 * This is NOT RFC 4514 compliance.
 *
 * 1. separate into RDNs
 * 2. for each RDN, normalize value based on its syntax
 *    (does not handle multi-value RDN case)
 *    remove all leading/trailing spaces
 * 3. reconstruct DN based on RDNs
 */
DWORD
VmDirNormalizeDN(
    PVDIR_BERVALUE      pBerv,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD   dwError = 0;
    PSTR    pszTmpDN = NULL;
    PSTR*   ppszRDNs = NULL;
    PSTR*   ppszNormRDNs = NULL;
    int     iNumRDNs = 0;
    int     iCnt = 0;
    size_t  iNormDNSize = 0;

    PVDIR_SCHEMA_CTX    pCtx = pSchemaCtx;

    if (!pBerv || !pBerv->lberbv.bv_val)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Already normalized. => Nothing to be done.
    if ( pBerv->bvnorm_val != NULL )
    {
        dwError = 0;
        goto cleanup;
    }
    // Nothing to be normalized for the ROOT DN
    if (pBerv->lberbv.bv_len == 0)
    {
        pBerv->bvnorm_val = pBerv->lberbv.bv_val;
        pBerv->bvnorm_len = 0;
        dwError = 0;
        goto cleanup;
    }

    if (pCtx == NULL)
    {
        dwError = VmDirSchemaCtxAcquire(&pCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // make a local copy
    dwError = VmDirAllocateStringA(
            pBerv->lberbv.bv_val,
            &pszTmpDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdirSchemaInPlaceDN2RDNs(
            pszTmpDN,
            &ppszRDNs,
            &iNumRDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(PSTR) * (iNumRDNs + 1),
            (PVOID)&ppszNormRDNs);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt=0; ppszRDNs[iCnt] && iCnt < iNumRDNs; iCnt++)
    {
        size_t           iNameLen = 0 ;
        VDIR_BERVALUE    berval = VDIR_BERVALUE_INIT;
        PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;

        char* pChar = VmDirStringChrA(ppszRDNs[iCnt], '=');
        if (!pChar)
        {
            dwError = ERROR_INVALID_DN;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        *pChar = '\0';

        iNameLen = VmDirStringLenA(ppszRDNs[iCnt]);
        // attribute name - remove all leading/trailing spaces
        while (ppszRDNs[iCnt][iNameLen-1] == ' ')
        {
            ppszRDNs[iCnt][iNameLen-1] = '\0';
            iNameLen--;
            assert(iNameLen > 0); // MIN 1 char for name
        }
        while (ppszRDNs[iCnt][0] == ' ')
        {
            ppszRDNs[iCnt]++;
        }

        pATDesc = VmDirSchemaAttrNameToDesc(pCtx, ppszRDNs[iCnt]);
        if (!pATDesc)
        {
            dwError = ERROR_INVALID_DN;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        berval.lberbv.bv_val = pChar+1;
        berval.lberbv.bv_len = VmDirStringLenA(berval.lberbv.bv_val);
        dwError = VmDirSchemaBervalNormalize(pCtx, pATDesc, &berval);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (berval.bvnorm_len == 0)
        {
            dwError = ERROR_INVALID_DN;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (berval.lberbv.bv_val == berval.bvnorm_val)
        {
            dwError = VmDirAllocateStringA(
                    berval.lberbv.bv_val,
                    &ppszNormRDNs[iCnt]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {   // ppszNormRDNs takes over bvnorm_val
            ppszNormRDNs[iCnt] = berval.bvnorm_val;
        }

        iNormDNSize = iNormDNSize + berval.bvnorm_len + (pChar - ppszRDNs[iCnt]) + 2;
    }

    // Reconstruct normalized DN
    VMDIR_SAFE_FREE_MEMORY(pBerv->bvnorm_val);
    pBerv->bvnorm_len = 0;
    dwError = VmDirAllocateMemory(
            sizeof(char) * (iNormDNSize+1),
            (PVOID)&pBerv->bvnorm_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; ppszNormRDNs[iCnt] && iCnt < iNumRDNs; iCnt++)
    {
        size_t iValueLen = VmDirStringLenA(ppszNormRDNs[iCnt]);
        int iValueOffset = 0;

        // attribute value - remove leading/trailing spaces
        while (ppszNormRDNs[iCnt][iValueOffset] == ' ')
        {
            iValueOffset++;
            assert(iValueOffset < iValueLen);
        }
        while (ppszNormRDNs[iCnt][iValueLen-1] == ' ')
        {
            ppszNormRDNs[iCnt][iValueLen-1] = '\0';
            iValueLen--;
            assert(iValueLen > 0);
        }

        // attribute name to lower case
        {
            char* pToLower = NULL;
            for (pToLower = ppszRDNs[iCnt]; *pToLower != '\0'; pToLower++)
            {
                *pToLower = (char) tolower(*pToLower);
            }
        }

        VmDirStringCatA(pBerv->bvnorm_val, (iNormDNSize+1), ppszRDNs[iCnt]);
        VmDirStringCatA(pBerv->bvnorm_val, (iNormDNSize+1), "=");
        VmDirStringCatA(pBerv->bvnorm_val, (iNormDNSize+1), ppszNormRDNs[iCnt]+iValueOffset);
        if (iCnt + 1 < iNumRDNs)
        {
            VmDirStringCatA(pBerv->bvnorm_val, (iNormDNSize+1), ",");
        }
    }

    pBerv->bvnorm_len = VmDirStringLenA(pBerv->bvnorm_val);

cleanup:

    if (pCtx && pCtx != pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pCtx);
    }

    VMDIR_SAFE_FREE_MEMORY(pszTmpDN);
    VMDIR_SAFE_FREE_MEMORY(ppszRDNs); // ppszRDNs[i] is in place of pszTmpDN

    VmDirFreeStringArrayA(ppszNormRDNs);
    VMDIR_SAFE_FREE_MEMORY(ppszNormRDNs);

    return dwError;

error:

    goto cleanup;
}

static
BOOLEAN
compareString(
    VDIR_SCHEMA_MATCH_TYPE type,
    PVDIR_BERVALUE    pAssert,
    PVDIR_BERVALUE    pBerv
    )
{
    BOOLEAN bRtn = FALSE;
    size_t  dwMinLen = 0;
    int     iResult = 0;

    if (!pAssert || !pBerv || pAssert->bvnorm_len > pBerv->bvnorm_len)
    {
        goto done;
    }

    switch (type)
    {
    case VDIR_SCHEMA_MATCH_EQUAL:
        if (pAssert->bvnorm_len != pBerv->bvnorm_len)
        {
            goto done;
        }

        bRtn = (VmDirStringNCompareA(pAssert->bvnorm_val, pBerv->bvnorm_val, pAssert->lberbv.bv_len, TRUE) == 0);

        break;

    case VDIR_SCHEMA_MATCH_GE:
        dwMinLen = MIN(pAssert->bvnorm_len, pBerv->bvnorm_len);
        iResult = VmDirStringNCompareA(pAssert->bvnorm_val, pBerv->bvnorm_val, dwMinLen, TRUE);

        if ((iResult > 0) ||
            (iResult == 0 && pAssert->bvnorm_len >= pBerv->bvnorm_len))
        {
            bRtn = TRUE;
        }

        break;

    case VDIR_SCHEMA_MATCH_LE:
        dwMinLen = MIN(pAssert->bvnorm_len, pBerv->bvnorm_len);
        iResult = VmDirStringNCompareA(pAssert->bvnorm_val, pAssert->lberbv.bv_val, dwMinLen, TRUE);

        if ((iResult < 0) ||
            (iResult == 0 && pAssert->bvnorm_len <= pBerv->bvnorm_len))
        {
            bRtn = TRUE;
        }

        break;

    default:

        break;
    }

done:

    return bRtn;
}

static
BOOLEAN
compareSubString(
    VDIR_SCHEMA_MATCH_TYPE type,
    PVDIR_BERVALUE    pAssert,
    PVDIR_BERVALUE    pBerv
    )
{
    BOOLEAN bRtn = FALSE;
    DWORD dwCnt = 0;
    size_t dwDelta = 0;

    if (!pAssert || !pBerv || pAssert->bvnorm_len > pBerv->bvnorm_len)
    {
        goto done;
    }

    dwDelta = pBerv->bvnorm_len - pAssert->bvnorm_len;

    switch (type)
    {
    case VDIR_SCHEMA_MATCH_SUBSTR_INIT:

        bRtn = (VmDirStringNCompareA(pAssert->bvnorm_val,
                        pBerv->bvnorm_val,
                        pAssert->bvnorm_len, TRUE) == 0);

        break;

    case VDIR_SCHEMA_MATCH_SUBSTR_FINAL:

        bRtn = (VmDirStringNCompareA(pAssert->bvnorm_val,
                        pBerv->bvnorm_val + dwDelta,
                        pAssert->bvnorm_len, TRUE) == 0);

        break;

    case VDIR_SCHEMA_MATCH_SUBSTR_ANY:  // TODO, not efficient
        for (dwCnt = 1; dwCnt < dwDelta; dwCnt++)
        {
            if (VmDirStringNCompareA(pAssert->bvnorm_val,
                        pBerv->bvnorm_val + dwCnt,
                        pAssert->bvnorm_len, TRUE) == 0)
            {
                bRtn = TRUE;
                break;
            }
        }

        break;

    default:

        break;
    }


done:

    return bRtn;
}

/****************************************************************************
 * RFC 4517 - 3.3.16.  Integer
 *    Integer = ( HYPHEN LDIGIT *DIGIT ) / number
 *    number  = DIGIT / ( LDIGIT 1*DIGIT )
 *    DIGIT   = %x30 / LDIGIT       ; "0"-"9"
 *    LDIGIT  = %x31-39             ; "1"-"9"
 *    HYPHEN  = %x2D ; hyphen ("-")
 *
 ****************************************************************************
 *
 * Assume pAssert/pBerv already normalized and have correct syntax
 */
static
BOOLEAN
compareIntegerString(
    VDIR_SCHEMA_MATCH_TYPE type,
    PVDIR_BERVALUE    pAssert,
    PVDIR_BERVALUE    pBerv
    )
{
    BOOLEAN bRtn = FALSE;
    int     iCnt = 0;

    PCSTR   pAssertVal = NULL;
    size_t  iAssertValLen = 0;
    BOOLEAN bAssertSignPositive = TRUE;

    PCSTR   pBerVal = NULL;
    size_t  iBerValLen = 0;
    BOOLEAN bBerSignPositive = TRUE;

    if (!pAssert || !pAssert->lberbv.bv_val || !pBerv || !pBerv->lberbv.bv_val)
    {
        goto done;
    }

    pAssertVal    = BERVAL_NORM_VAL(*pAssert);
    iAssertValLen = BERVAL_NORM_LEN(*pAssert);
    pBerVal       = BERVAL_NORM_VAL(*pBerv);
    iBerValLen    = BERVAL_NORM_LEN(*pBerv);

    if (pAssertVal[0] == '-')
    {
        bAssertSignPositive = FALSE;
        pAssertVal = &pAssertVal[1];
        iAssertValLen--;
    }

    if (pBerVal[0] == '-')
    {
        bBerSignPositive = FALSE;
        pBerVal = &pBerVal[1];
        iBerValLen--;
    }

    if (type == VDIR_SCHEMA_MATCH_EQUAL ||
        type == VDIR_SCHEMA_MATCH_GE    ||
        type == VDIR_SCHEMA_MATCH_LE    )
    {
        // -0 , 0 case
        if (iAssertValLen == 1 && pAssertVal[0] == '0' &&
            iBerValLen    == 1 && pBerVal[0]    == '0')
        {
            bRtn = TRUE;
            goto done;
        }
    }

    switch (type)
    {
    case VDIR_SCHEMA_MATCH_EQUAL:

        if ((bAssertSignPositive != bBerSignPositive) ||
            (iAssertValLen != iBerValLen))
        {
            goto done;
        }

        bRtn = (memcmp(pAssertVal, pBerVal, iBerValLen) == 0);

        break;

    case VDIR_SCHEMA_MATCH_GE:

        if ((!bAssertSignPositive && bBerSignPositive))
        {
            goto done;
        }

        if ((bAssertSignPositive && !bBerSignPositive))
        {
            bRtn = TRUE;
            goto done;
        }

        // values have same sign
        if  (( bAssertSignPositive && iAssertValLen > iBerValLen) ||
             (!bAssertSignPositive && iAssertValLen < iBerValLen))
        {
            bRtn = TRUE;
            goto done;
        }

        // values have same sign and len
        if (iAssertValLen == iBerValLen)
        {
            bRtn = TRUE;
            for (iCnt=0; iCnt < iBerValLen; iCnt++)
            {
                if ( pAssertVal[iCnt] == pBerVal[iCnt] )
                {
                    continue;
                }
                if (( bAssertSignPositive && (pAssertVal[iCnt] < pBerVal[iCnt])) ||
                    (!bAssertSignPositive &&(pAssertVal[iCnt] > pBerVal[iCnt])))
                {
                    bRtn = FALSE;
                    goto done;
                }
                if (( bAssertSignPositive && (pAssertVal[iCnt] > pBerVal[iCnt])) ||
                    (!bAssertSignPositive &&(pAssertVal[iCnt] < pBerVal[iCnt])))
                {
                    bRtn = TRUE;
                    goto done;
                }
            }
            goto done;
        }

        break;

    case VDIR_SCHEMA_MATCH_LE:

        if ((bAssertSignPositive && !bBerSignPositive))
        {
            goto done;
        }

        if ((!bAssertSignPositive && bBerSignPositive))
        {
            bRtn = TRUE;
            goto done;
        }

        // values have same sign
        if  ((!bAssertSignPositive && iAssertValLen > iBerValLen) ||
             ( bAssertSignPositive && iAssertValLen < iBerValLen))
        {
            bRtn = TRUE;
            goto done;
        }

        // values have same sign and len
        if (iAssertValLen == iBerValLen)
        {
            bRtn = TRUE;
            for (iCnt=0; iCnt < iBerValLen; iCnt++)
            {
                if (( bAssertSignPositive && (pAssertVal[iCnt] > pBerVal[iCnt])) ||
                    (!bAssertSignPositive && (pAssertVal[iCnt] < pBerVal[iCnt])))
                {
                    bRtn = FALSE;
                    goto done;
                }
                else if (( bAssertSignPositive && (pAssertVal[iCnt] < pBerVal[iCnt])) ||
                         (!bAssertSignPositive && (pAssertVal[iCnt] > pBerVal[iCnt])))
                {
                    bRtn = TRUE;
                    goto done;
                }
                else
                {   // pAssertVal[iCnt] == pBerVal[iCnt] scenario
                    continue;
                }
            }
        }

        break;

    default:

        break;
    }

done:

    return bRtn;
}
