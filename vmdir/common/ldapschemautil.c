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

static
DWORD
_VmDirReadOneDefFromFile(
    FILE*              fp,
    PVMDIR_STRING_LIST pStrList
    );

static
DWORD
 _VmDirAllocLdapSchemaStruct(
    PVMDIR_LDAP_SCHEMA_STRUCT* ppSchemaStruct
    );

static
DWORD
_VmDirRepairBadDefs(
    PSTR*       ppszRepairDef,
    PBOOLEAN    pbFix
    );

static
DWORD
_VmDirSchemaDefToStruct(
    PVMDIR_LDAP_SCHEMA_DEF_STR      pSchemaDefs,
    PVMDIR_LDAP_SCHEMA_STRUCT       pSchemaStruct
    );

static
VOID
_VmDirNoopHashMapClear(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID pUnused
    );

static
DWORD
_VmDirGetSchemaDefFromMessage(
    LDAP*           pLd,
    LDAPMessage*    pEntry,
    PCSTR           pszAttrName,
    PVMDIR_STRING_LIST    pStrList
    );

static
DWORD
_VmDirGetPartnerSchemaDef(
    LDAP*                       pLd,
    PVMDIR_LDAP_SCHEMA_DEF_STR  pSchemaDefStr
    );

static
DWORD
_VmDirMergeAT(
    PVMDIR_LDAP_SCHEMA_STRUCT  pCurrentSchema,
    PVMDIR_LDAP_SCHEMA_STRUCT  pNewSchema,
    PVMDIR_LDAP_SCHEMA_MOD_STR pModStr
    );

static
DWORD
_VmDirMergeOC(
    PVMDIR_LDAP_SCHEMA_STRUCT  pCurrentSchema,
    PVMDIR_LDAP_SCHEMA_STRUCT  pNewSchema,
    PVMDIR_LDAP_SCHEMA_MOD_STR pModStr
    );

static
DWORD
_VmDirMergeCR(
    PVMDIR_LDAP_SCHEMA_STRUCT  pCurrentSchema,
    PVMDIR_LDAP_SCHEMA_STRUCT  pNewSchema,
    PVMDIR_LDAP_SCHEMA_MOD_STR pModStr
    );

static
DWORD
_VmDirMergeObjectClasses(
    PVMDIR_LDAP_OBJECTCLASSES  pCurrentOC,
    PVMDIR_LDAP_OBJECTCLASSES  pNewOC,
    BOOLEAN*                   pbNeedUpdate
    );


DWORD
VmDirInitModStrContent(
    PVMDIR_LDAP_SCHEMA_MOD_STR  pModStr
    )
{
    DWORD   dwError = 0;

    dwError = VmDirStringListInitialize(&pModStr->pAddATStrList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirStringListInitialize(&pModStr->pDelATStrList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirStringListInitialize(&pModStr->pAddOCStrList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirStringListInitialize(&pModStr->pDelOCStrList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirStringListInitialize(&pModStr->pAddCRStrList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirStringListInitialize(&pModStr->pDelCRStrList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmDirFreeModStrContent(pModStr);
    goto cleanup;
}


VOID
VmDirFreeModStrContent(
    PVMDIR_LDAP_SCHEMA_MOD_STR  pModStr
    )
{
    if (pModStr)
    {
        VmDirStringListFree(pModStr->pAddATStrList);
        VmDirStringListFree(pModStr->pDelATStrList);
        VmDirStringListFree(pModStr->pAddOCStrList);
        VmDirStringListFree(pModStr->pDelOCStrList);
        VmDirStringListFree(pModStr->pAddCRStrList);
        VmDirStringListFree(pModStr->pDelCRStrList);
    }

    return;
}

/*
 * valid super/sub set relationship.
 * if super >= sub, return true; otherwise, return false;
 *
 * ppszSuper/ppszSub set could be NULL.
 */
BOOLEAN
VmDirIsSortedSuperSetof(
    PSTR* ppszSuper,
    PSTR* ppszSub
    )
{
    BOOLEAN bRtn = TRUE;
    int     iSizeSuper = 0, iSizeSub = 0, iCntSub = 0, iCntSuper = 0;

    for ( iSizeSuper=0; ppszSuper && ppszSuper[iSizeSuper]; iSizeSuper++) ;
    for ( iSizeSub=0; ppszSub     && ppszSub[iSizeSub]; iSizeSub++) ;

    if ( iSizeSuper >= iSizeSub )
    {
        while ( iCntSuper<iSizeSuper && iCntSub<iSizeSub )
        {
            DWORD dwCmp = VmDirStringCompareA(ppszSuper[iCntSuper], ppszSub[iCntSub], FALSE);
            if (dwCmp > 0)
            {
                iCntSuper++;
            }
            else if ( dwCmp == 0)
            {
                iCntSuper++;
                iCntSub++;
            }
            else
            {
                break;
            }
        }

        if (iCntSub == iSizeSub)
        {
            bRtn = TRUE;
        }
    }
    else
    {
        bRtn = FALSE;
    }

    return bRtn;
}

/*
 * pszSet could be NULL
 */
BOOLEAN
VmDirIsSortedSetIdentical(
    PSTR* ppszSet1,
    PSTR* ppszSet2
    )
{
    BOOLEAN bRtn = TRUE;
    int     iSize1 = 0, iSize2 = 0, iCnt = 0;

    for ( iSize1=0; ppszSet1 && ppszSet1[iSize1]; iSize1++) ;
    for ( iSize2=0; ppszSet2 && ppszSet2[iSize2]; iSize2++) ;

    if ( iSize1 == iSize2 )
    {
        for (iCnt=0; iCnt<iSize1; iCnt++)
        {
            if (VmDirStringCompareA(ppszSet1[iCnt], ppszSet2[iCnt], FALSE) != 0)
            {
                bRtn = FALSE;
                break;
            }
        }
    }
    else
    {
        bRtn = FALSE;
    }

    return bRtn;
}

VOID
VmDirFreeStrArray(
    PSTR*   ppszArray
    )
{
    int i = 0;

    if (ppszArray)
    {
        for (i=0; ppszArray[i] != NULL; i++)
        {
            VMDIR_SAFE_FREE_MEMORY(ppszArray[i]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppszArray);
    }
}

/*
 *  ppszSet is a NULL terminated PSTR array.
 *
 *  Assume newer version of schema can have more elements.
 *  We support down and up level join.
 *  Thus, if element in setNew is not found in setCur, outset is (setCur UNION setNew);
 *        otherwise, outset is NULL;
 *
 *  This means we based on current set and add missing element in the new set to it if needed.
 */
DWORD
VmDirMergeSet(
    PSTR* ppszSetCur,
    PSTR* ppszSetNew,
    PSTR** pppszOutSet
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszArray = NULL;
    int     iIdx = 0;
    int     iSize1 = 0, iSize2 = 0, iCnt1 = 0, iCnt2 = 0;

    if ( !pppszOutSet )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for ( iSize1=0; ppszSetCur && ppszSetCur[iSize1]; iSize1++) ;
    for ( iSize2=0; ppszSetNew && ppszSetNew[iSize2]; iSize2++) ;

    if ( iSize1 == 0 && iSize2 == 0 )
    {
        goto cleanup;
    }

    if (iSize1 > 0)
    {
        qsort(ppszSetCur, iSize1, sizeof(*ppszSetCur), VmDirQsortCaseIgnoreCompareString);
    }
    if (iSize2 > 0)
    {
        qsort(ppszSetNew, iSize2, sizeof(*ppszSetNew), VmDirQsortCaseIgnoreCompareString);
    }

    // if current set is not a super set of new set, union them into out set.
    if ( VmDirIsSortedSuperSetof(ppszSetCur, ppszSetNew) == FALSE )
    {
        dwError = VmDirAllocateMemory(sizeof(PSTR)*(iSize1+iSize2+1), (PVOID*)&ppszArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (iCnt1=0, iCnt2=0, iIdx=0; iCnt1<iSize1 || iCnt2<iSize2; iIdx++)
        {
            PSTR pszTmp = NULL;

            if (iCnt1<iSize1 && iCnt2<iSize2)
            {
                LONG i = VmDirStringCompareA(ppszSetCur[iCnt1], ppszSetNew[iCnt2], FALSE);

                if ( i == 0 )
                {
                    pszTmp = ppszSetCur[iCnt1];
                    iCnt1++;
                    iCnt2++;
                }
                else if ( i < 0 )
                {
                    pszTmp = ppszSetCur[iCnt1];
                    iCnt1++;
                }
                else
                {
                    pszTmp = ppszSetNew[iCnt2];
                    iCnt2++;
                }
            }
            else if (iCnt1<iSize1 && iCnt2>=iSize2)
            {
                pszTmp = ppszSetCur[iCnt1];
                iCnt1++;
            }
            else if (iCnt1>=iSize1 && iCnt2<iSize2)
            {
                pszTmp = ppszSetNew[iCnt2];
                iCnt2++;
            }

            dwError = VmDirAllocateStringA(pszTmp, (ppszArray+iIdx));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *pppszOutSet = ppszArray;

cleanup:
    return dwError;

error:
    VmDirFreeStrArray(ppszArray);
    goto cleanup;
}

/*
 * Read schema definition from file
 * We only care for
 *  attributetypes
 *  objectclasses
 *  ditcontentrules
 */
DWORD
VmDirReadSchemaFile(
    PCSTR                       pszSchemaFilePath,
    PVMDIR_LDAP_SCHEMA_DEF_STR  pSchemaDef
    )
{
    static PSTR pszATTag = "attributetypes:";
    static PSTR pszOCTag = "objectclasses:";
    static PSTR pszCRTag = "ditcontentrules:";

    DWORD dwError = 0;
    CHAR  pbuf[1024] = {0};
    FILE* fp = NULL;

    if ( !pszSchemaFilePath || !pSchemaDef )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifndef _WIN32
    fp=fopen(pszSchemaFilePath, "r");
#else
    if( fopen_s(&fp, pszSchemaFilePath, "r") != 0 )
    {
        fp = NULL;
    }
#endif
    if(NULL == fp)
    {
        dwError = errno;
        VMDIR_LOG_ERROR(    VMDIR_LOG_MASK_ALL,
                            "Open schema file (%s) failed. Error (%d)",
                            pszSchemaFilePath,
                            dwError);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (fgets(pbuf, sizeof(pbuf), fp) != NULL)
    {
        PSTR pszTag = NULL;

        if ((pbuf[0] == '\n')    ||
            (pbuf[0] == '#')     ||
            (pbuf[0] != ' ' && (pszTag = VmDirStringChrA(pbuf, ':')) == NULL))
        {
            continue;
        }

        if (VmDirStringNCompareA(pbuf, pszATTag, VmDirStringLenA(pszATTag), FALSE) == 0)
        {
            dwError = _VmDirReadOneDefFromFile(fp, pSchemaDef->pATStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringNCompareA(pbuf, pszOCTag, VmDirStringLenA(pszOCTag), FALSE) == 0)
        {
            dwError = _VmDirReadOneDefFromFile(fp, pSchemaDef->pOCStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringNCompareA(pbuf, pszCRTag, VmDirStringLenA(pszCRTag), FALSE) == 0 )
        {
            dwError = _VmDirReadOneDefFromFile(fp, pSchemaDef->pCRStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            continue;
        }
    }

cleanup:

    if (fp)
    {
        fclose(fp);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirGetSchemaFromLocalFile(
    PCSTR                       pszFile,
    PVMDIR_LDAP_SCHEMA_STRUCT*  ppLdapSchema
    )
{
    DWORD   dwError = 0;
    VMDIR_LDAP_SCHEMA_DEF_STR  schemaDefs = {0};
    PVMDIR_LDAP_SCHEMA_STRUCT  pSchemaStruct = NULL;

    if (!pszFile || !ppLdapSchema)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirAllocLdapSchemaStruct(&pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitLdapSchemaDefsContent(&schemaDefs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadSchemaFile(pszFile, &schemaDefs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSchemaDefToStruct(&schemaDefs, pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLdapSchema = pSchemaStruct;

cleanup:
    VmDirFreeLdapSchemaDefsContent(&schemaDefs);

    return dwError;

error:
    VmDirFreeLdapSchemaStruct(pSchemaStruct);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "[%s][%d] failed error=%d",__FUNCTION__,__LINE__,dwError);
    goto cleanup;
}


DWORD
VmDirGetSchemaFromPartner(
    LDAP*                       pLd,
    PVMDIR_LDAP_SCHEMA_STRUCT*  ppLdapSchema
    )
{
    DWORD               dwError = 0;
    VMDIR_LDAP_SCHEMA_DEF_STR   schemaDefs = {0};
    PVMDIR_LDAP_SCHEMA_STRUCT   pSchemaStruct = NULL;

    if (!pLd || !ppLdapSchema)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirAllocLdapSchemaStruct(&pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitLdapSchemaDefsContent(&schemaDefs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetPartnerSchemaDef(pLd, &schemaDefs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSchemaDefToStruct(&schemaDefs, pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLdapSchema = pSchemaStruct;

cleanup:
    VmDirFreeLdapSchemaDefsContent(&schemaDefs);

    return dwError;
error:
    VmDirFreeLdapSchemaStruct(pSchemaStruct);
    goto cleanup;
}

DWORD
VmDirGetSchemaFromDefStr(
    PVMDIR_LDAP_SCHEMA_DEF_STR  pSchemaDefs,
    PVMDIR_LDAP_SCHEMA_STRUCT*  ppLdapSchema
    )
{
    DWORD   dwError = 0;
    PVMDIR_LDAP_SCHEMA_STRUCT   pSchemaStruct = NULL;

    if (!pSchemaDefs || !ppLdapSchema)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirAllocLdapSchemaStruct(&pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSchemaDefToStruct(pSchemaDefs, pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLdapSchema = pSchemaStruct;

cleanup:
    return dwError;

error:
    VmDirFreeLdapSchemaStruct(pSchemaStruct);
    goto cleanup;
}

/*
 *  Merge new schema into existing one.
 */
DWORD
VmDirAnalyzeSchemaUpgrade(
    PVMDIR_LDAP_SCHEMA_STRUCT  pExistingSchema,
    PVMDIR_LDAP_SCHEMA_STRUCT  pNewSchema,
    PVMDIR_LDAP_SCHEMA_MOD_STR pModStr
    )
{
    DWORD   dwError = 0;

    if (!pExistingSchema || !pNewSchema || !pModStr)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirMergeAT(pExistingSchema,pNewSchema,pModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirMergeOC(pExistingSchema,pNewSchema,pModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirMergeCR(pExistingSchema,pNewSchema,pModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirFreeLdapSchemaStruct(
    PVMDIR_LDAP_SCHEMA_STRUCT pSchemaStruct
    )
{
    DWORD dwCnt = 0;

    if (pSchemaStruct)
    {
        for (dwCnt=0; dwCnt<pSchemaStruct->dwATSize; dwCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pATArray[dwCnt].pszOrgDef);
            VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pATArray[dwCnt].pszNormDef);
            if (pSchemaStruct->pATArray[dwCnt].pLdapAT)
            {
                ldap_attributetype_free(pSchemaStruct->pATArray[dwCnt].pLdapAT);
            }
        }
        VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pATArray);
        LwRtlHashMapClear(pSchemaStruct->pATMap, _VmDirNoopHashMapClear, NULL);
        LwRtlFreeHashMap(&pSchemaStruct->pATMap);

        for (dwCnt=0; dwCnt<pSchemaStruct->dwOCSize; dwCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pOCArray[dwCnt].pszOrgDef);
            VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pOCArray[dwCnt].pszNormDef);
            if (pSchemaStruct->pOCArray[dwCnt].pLdapOC)
            {
                ldap_objectclass_free(pSchemaStruct->pOCArray[dwCnt].pLdapOC);
            }
        }
        VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pOCArray);
        LwRtlHashMapClear(pSchemaStruct->pOCMap, _VmDirNoopHashMapClear, NULL);
        LwRtlFreeHashMap(&pSchemaStruct->pOCMap);

        for (dwCnt=0; dwCnt<pSchemaStruct->dwCRSize; dwCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pCRArray[dwCnt].pszOrgDef);
            VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pCRArray[dwCnt].pszNormDef);
            if (pSchemaStruct->pCRArray[dwCnt].pLdapCR)
            {
                ldap_contentrule_free(pSchemaStruct->pCRArray[dwCnt].pLdapCR);
            }
        }
        VMDIR_SAFE_FREE_MEMORY(pSchemaStruct->pCRArray);
        LwRtlHashMapClear(pSchemaStruct->pCRMap, _VmDirNoopHashMapClear, NULL);
        LwRtlFreeHashMap(&pSchemaStruct->pCRMap);

        VMDIR_SAFE_FREE_MEMORY(pSchemaStruct);
    }

    return;
}

/*
 * read one schema element definition from file and normalize its definition.
 */
static
DWORD
_VmDirReadOneDefFromFile(
    FILE*              fp,
    PVMDIR_STRING_LIST pStrList
    )
{
    DWORD   dwError = 0;
    size_t  iSize = VMDIR_SIZE_9216, iLen = 0;
    CHAR    pDescBuf[VMDIR_SIZE_9216+1] = {0};
    CHAR    pbuf[VMDIR_SIZE_1024] = {0};
    PCSTR   pPrefix = "( ";
    size_t  iPrefixLen = VmDirStringLenA(pPrefix);
    PSTR    pOut = NULL;

    dwError = VmDirStringNCatA(pDescBuf+iLen, iSize-iLen, pPrefix, iPrefixLen);
    BAIL_ON_VMDIR_ERROR(dwError);
    iLen += iPrefixLen;

    while (fgets(pbuf, sizeof(pbuf), fp) != NULL)
    {
        size_t len = VmDirStringLenA(pbuf)-1;
        if (pbuf[len] == '\n')
        {
            pbuf[len] = '\0';
        }

        if ( pbuf[0] == '#')
        {
            continue;
        }

        if ( pbuf[0] == ' ')
        {
            dwError = VmDirStringNCatA(pDescBuf+iLen, iSize-iLen, pbuf, VmDirStringLenA(pbuf));
            BAIL_ON_VMDIR_ERROR(dwError);
            iLen += VmDirStringLenA(pbuf);
        }
        else
        {
            VmdDirSchemaParseNormalizeElement( pDescBuf );
            dwError = VmDirAllocateStringA( pDescBuf, &pOut);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd( pStrList, pOut);
            BAIL_ON_VMDIR_ERROR(dwError);
            pOut = NULL;
            break;
        }
    }

cleanup:

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pOut);
    goto cleanup;
}

DWORD
VmDirInitLdapSchemaDefsContent(
    PVMDIR_LDAP_SCHEMA_DEF_STR  pSchemaDefs
    )
{
    DWORD   dwError = 0;

    dwError = VmDirStringListInitialize(&pSchemaDefs->pATStrList, 2048);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pSchemaDefs->pOCStrList, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pSchemaDefs->pCRStrList, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

VOID
VmDirFreeLdapSchemaDefsContent(
    PVMDIR_LDAP_SCHEMA_DEF_STR  pSchemaDefs
    )
{
    VmDirStringListFree(pSchemaDefs->pATStrList);
    VmDirStringListFree(pSchemaDefs->pOCStrList);
    VmDirStringListFree(pSchemaDefs->pCRStrList);

    return;
}

static
DWORD
 _VmDirAllocLdapSchemaStruct(
    PVMDIR_LDAP_SCHEMA_STRUCT* ppSchemaStruct
    )
{
    DWORD                       dwError = 0;
    PVMDIR_LDAP_SCHEMA_STRUCT   pSchemaStruct = NULL;

    dwError = VmDirAllocateMemory(sizeof(*pSchemaStruct), (PVOID*)&pSchemaStruct);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
                &pSchemaStruct->pATMap,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
                &pSchemaStruct->pOCMap,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlCreateHashMap(
                &pSchemaStruct->pCRMap,
                LwRtlHashDigestPstrCaseless,
                LwRtlHashEqualPstrCaseless,
                NULL
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppSchemaStruct = pSchemaStruct;

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 *  Legacy schema has bad syntax for a few definition.
 *  Correct them to pass ldap schema parse function call.
 */
static
DWORD
_VmDirRepairBadDefs(
    PSTR*       ppszRepairDef,
    PBOOLEAN    pbFix
    )
{
    static PCSTR   pPair[] =
            {
            "( VMWare.LKUP.attribute.27 NAME vmwLKUPLegacyIds DESC 'VMware Lookup Service - service identifier' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{256} )",
            "( VMWare.LKUP.attribute.27 NAME 'vmwLKUPLegacyIds' DESC 'VMware Lookup Service - service identifier' EQUALITY caseIgnoreMatch SYNTAX 1.3.6.1.4.1.1466.115.121.1.15{256} )",
            "( VMWare.STS.objectclass.25 NAME 'vmwExternalIdpUser' DESC 'VMWare external idp user' AUXILIARY MUST ( vmwSTSEntityId vmwSTSExternalIdpUserId ) )",
            "( VMWare.STS.objectclass.25 NAME 'vmwExternalIdpUser' DESC 'VMWare external idp user' AUXILIARY MUST ( vmwSTSEntityId $ vmwSTSExternalIdpUserId ) )",
            NULL
            };

    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszStr = NULL;

    for (dwCnt=0; pPair[dwCnt] != NULL; dwCnt+=2)
    {
        if (VmDirStringCompareA(*ppszRepairDef, pPair[dwCnt], FALSE) == 0)
        {
            dwError = VmDirAllocateStringA(pPair[dwCnt+1], &pszStr);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
    }

    if (pszStr)
    {
        VMDIR_SAFE_FREE_MEMORY(*ppszRepairDef);
        *ppszRepairDef = pszStr;
        *pbFix = TRUE;
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

static
DWORD
_VmDirSchemaDefToStruct(
    PVMDIR_LDAP_SCHEMA_DEF_STR      pSchemaDefs,
    PVMDIR_LDAP_SCHEMA_STRUCT       pSchemaStruct
    )
{
    DWORD       dwError = 0;
    DWORD       i = 0;
    int         iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;

    LDAPAttributeType* pLDAPAt = NULL;
    LDAPObjectClass*   pLDAPOc = NULL;
    LDAPContentRule*   pLDAPCr = NULL;

    pSchemaStruct->dwATSize = pSchemaDefs->pATStrList->dwCount;
    dwError = VmDirAllocateMemory(sizeof(*(pSchemaStruct->pATArray)) * pSchemaStruct->dwATSize,
                                  (PVOID*)&pSchemaStruct->pATArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSchemaStruct->dwOCSize = pSchemaDefs->pOCStrList->dwCount;
    dwError = VmDirAllocateMemory(sizeof(*(pSchemaStruct->pOCArray)) * pSchemaStruct->dwOCSize,
                                  (PVOID*)&pSchemaStruct->pOCArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    pSchemaStruct->dwCRSize = pSchemaDefs->pCRStrList->dwCount;
    dwError = VmDirAllocateMemory(sizeof(*(pSchemaStruct->pCRArray)) * pSchemaStruct->dwCRSize,
                                  (PVOID*)&pSchemaStruct->pCRArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<pSchemaDefs->pATStrList->dwCount; i++)
    {
        dwError = VmDirAllocateStringA(pSchemaDefs->pATStrList->pStringList[i],
                                       &pSchemaStruct->pATArray[i].pszOrgDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pSchemaStruct->pATArray[i].pszOrgDef,
                                       &pSchemaStruct->pATArray[i].pszNormDef);
        BAIL_ON_VMDIR_ERROR(dwError);
        VmdDirSchemaParseNormalizeElement(pSchemaStruct->pATArray[i].pszNormDef);

        dwError = _VmDirRepairBadDefs(&pSchemaStruct->pATArray[i].pszNormDef,
                                      &pSchemaStruct->pATArray[i].bLegacyFix);
        BAIL_ON_VMDIR_ERROR(dwError);

        pLDAPAt = ldap_str2attributetype(pSchemaStruct->pATArray[i].pszNormDef, &iCode, &pErr, flags);
        if (pLDAPAt)
        {
            pSchemaStruct->pATArray[i].pLdapAT = pLDAPAt;
            dwError = LwRtlHashMapInsert(pSchemaStruct->pATMap,
                        pLDAPAt->at_names[0], &(pSchemaStruct->pATArray[i]), NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VMDIR_ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (i=0; i<pSchemaDefs->pOCStrList->dwCount; i++)
    {
        dwError = VmDirAllocateStringA(pSchemaDefs->pOCStrList->pStringList[i],
                                       &pSchemaStruct->pOCArray[i].pszOrgDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pSchemaStruct->pOCArray[i].pszOrgDef,
                                       &pSchemaStruct->pOCArray[i].pszNormDef);
        BAIL_ON_VMDIR_ERROR(dwError);
        VmdDirSchemaParseNormalizeElement(pSchemaStruct->pOCArray[i].pszNormDef);

        dwError = _VmDirRepairBadDefs(&pSchemaStruct->pOCArray[i].pszNormDef,
                                      &pSchemaStruct->pOCArray[i].bLegacyFix);
        BAIL_ON_VMDIR_ERROR(dwError);

        pLDAPOc = ldap_str2objectclass(pSchemaStruct->pOCArray[i].pszNormDef, &iCode, &pErr, flags);
        if (pLDAPOc)
        {
            pSchemaStruct->pOCArray[i].pLdapOC = pLDAPOc;
            dwError = LwRtlHashMapInsert(pSchemaStruct->pOCMap,
                        pLDAPOc->oc_names[0], &(pSchemaStruct->pOCArray[i]), NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VMDIR_ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (i=0; i<pSchemaDefs->pCRStrList->dwCount; i++)
    {
        dwError = VmDirAllocateStringA(pSchemaDefs->pCRStrList->pStringList[i],
                                       &pSchemaStruct->pCRArray[i].pszOrgDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pSchemaStruct->pCRArray[i].pszOrgDef,
                                       &pSchemaStruct->pCRArray[i].pszNormDef);
        BAIL_ON_VMDIR_ERROR(dwError);
        VmdDirSchemaParseNormalizeElement(pSchemaStruct->pCRArray[i].pszNormDef);

        dwError = _VmDirRepairBadDefs(&pSchemaStruct->pCRArray[i].pszNormDef,
                                      &pSchemaStruct->pCRArray[i].bLegacyFix);
        BAIL_ON_VMDIR_ERROR(dwError);

        pLDAPCr = ldap_str2contentrule(pSchemaStruct->pCRArray[i].pszNormDef, &iCode, &pErr, flags);
        if (pLDAPCr)
        {
            pSchemaStruct->pCRArray[i].pLdapCR = pLDAPCr;
            dwError = LwRtlHashMapInsert(pSchemaStruct->pCRMap,
                        pLDAPCr->cr_names[0], &(pSchemaStruct->pCRArray[i]), NULL);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VMDIR_ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * Our hash map does not own key and value pair.
 */
static
VOID
_VmDirNoopHashMapClear(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID pUnused
    )
{
    return;
}

static
DWORD
_VmDirGetSchemaDefFromMessage(
    LDAP*           pLd,
    LDAPMessage*    pEntry,
    PCSTR           pszAttrName,
    PVMDIR_STRING_LIST    pStrList
    )
{
    DWORD           dwError = 0;
    DWORD           dwCnt = 0;
    PSTR            pszStr = NULL;
    struct berval** ppValues = NULL;

    if ((ppValues = ldap_get_values_len(pLd, pEntry, pszAttrName)) != NULL)
    {
        for (dwCnt = 0; dwCnt < (DWORD)ldap_count_values_len(ppValues); dwCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pszStr);
            dwError = VmDirAllocateStringA( ppValues[dwCnt]->bv_val, &pszStr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd( pStrList, pszStr);
            BAIL_ON_VMDIR_ERROR(dwError)
            pszStr = NULL;
        }
    }

cleanup:
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

static
DWORD
_VmDirGetPartnerSchemaDef(
    LDAP*                       pLd,
    PVMDIR_LDAP_SCHEMA_DEF_STR  pSchemaDefStr
    )
{
    static PSTR ppszSchemaEntryAttrs[] =
    {
        ATTR_ATTRIBUTETYPES,
        ATTR_OBJECTCLASSES,
        ATTR_DITCONTENTRULES,
        NULL
    };

    DWORD           dwError = 0;
    PCSTR           pcszFilter = "(objectclass=*)";
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;

    dwError = ldap_search_ext_s(
                pLd,
                SUB_SCHEMA_SUB_ENTRY_DN,
                LDAP_SCOPE_BASE,
                pcszFilter,
                ppszSchemaEntryAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) != 1)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);

    dwError = _VmDirGetSchemaDefFromMessage(
                pLd,
                pEntry,
                ATTR_ATTRIBUTETYPES,
                pSchemaDefStr->pATStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetSchemaDefFromMessage(
                pLd,
                pEntry,
                ATTR_OBJECTCLASSES,
                pSchemaDefStr->pOCStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetSchemaDefFromMessage(
                pLd,
                pEntry,
                ATTR_DITCONTENTRULES,
                pSchemaDefStr->pCRStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "[%s][%d] failed error=%d",__FUNCTION__,__LINE__,dwError);
    goto cleanup;
}

/*
 * For attribute types, we always maintain b/c for existing definition.
 * In fact, we never change their definition in new version of schema file.
 *
 * Thus, we only add new attribute definition during upgrade.
 * We do NOT make change to current attribute definition.
 */
static
DWORD
_VmDirMergeAT(
    PVMDIR_LDAP_SCHEMA_STRUCT  pCurrentSchema,
    PVMDIR_LDAP_SCHEMA_STRUCT  pNewSchema,
    PVMDIR_LDAP_SCHEMA_MOD_STR pModStr
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER             iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR             pair = {NULL, NULL};
    PVMDIR_LDAP_ATTRIBUTETYPES  pNewAT = NULL;
    PVMDIR_LDAP_ATTRIBUTETYPES  pCurrentAT = NULL;

    while (LwRtlHashMapIterate(pNewSchema->pATMap, &iter, &pair))
    {
        pNewAT = (PVMDIR_LDAP_ATTRIBUTETYPES)pair.pValue;

        dwError = LwRtlHashMapFindKey(
                    pCurrentSchema->pATMap,
                    (PVOID*)&pCurrentAT,
                    pNewAT->pLdapAT->at_names[0]);
        if (dwError == 0 && pCurrentAT)
        {
            if ( VmDirStringCompareA(pCurrentAT->pszNormDef, pNewAT->pszNormDef, FALSE) != 0)
            {
                VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                                  "[%s][%d] Incompatible attributetypes \n Current [%s]\nNew [%s]",
                                  __FUNCTION__,__LINE__, pCurrentAT->pszNormDef, pNewAT->pszNormDef);
            }

            if ( pCurrentAT->bLegacyFix)
            {
                dwError = VmDirStringListAddStrClone(pNewAT->pszNormDef, pModStr->pAddATStrList);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringListAddStrClone(pCurrentAT->pszOrgDef, pModStr->pDelATStrList);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

        }
        else
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "[%s][%d] New attributetypes %s",
                            __FUNCTION__,__LINE__,
                            VDIR_SAFE_STRING(pNewAT->pLdapAT->at_names[0]));

            dwError = VmDirStringListAddStrClone(pNewAT->pszNormDef, pModStr->pAddATStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * For objectclasses, we only add attributes to MAY list.
 * This function merge new MAY list into current MAY list.
 */
static
DWORD
_VmDirMergeObjectClasses(
    PVMDIR_LDAP_OBJECTCLASSES  pCurrentOC,
    PVMDIR_LDAP_OBJECTCLASSES  pNewOC,
    BOOLEAN*                   pbNeedUpdate
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszMergeSet = NULL;

    dwError = VmDirMergeSet(
                    pCurrentOC->pLdapOC->oc_at_oids_may,
                    pNewOC->pLdapOC->oc_at_oids_may,
                    &ppszMergeSet);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppszMergeSet)
    {
        VmDirFreeStrArray(pCurrentOC->pLdapOC->oc_at_oids_may);
        pCurrentOC->pLdapOC->oc_at_oids_may = ppszMergeSet;
        ppszMergeSet = NULL;

        VMDIR_SAFE_FREE_MEMORY(pCurrentOC->pszNormDef);
        pCurrentOC->pszNormDef = ldap_objectclass2str(pCurrentOC->pLdapOC);
        assert(pCurrentOC->pszNormDef);

        *pbNeedUpdate = TRUE;
    }

cleanup:
    VmDirFreeStrArray(ppszMergeSet);
    return dwError;

error:
    goto cleanup;
}


static
DWORD
_VmDirMergeOC(
    PVMDIR_LDAP_SCHEMA_STRUCT  pCurrentSchema,
    PVMDIR_LDAP_SCHEMA_STRUCT  pNewSchema,
    PVMDIR_LDAP_SCHEMA_MOD_STR pModStr
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER             iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR             pair = {NULL, NULL};
    PVMDIR_LDAP_OBJECTCLASSES   pNewOC = NULL;
    PVMDIR_LDAP_OBJECTCLASSES   pCurrentOC = NULL;

    while (LwRtlHashMapIterate(pNewSchema->pOCMap, &iter, &pair))
    {
        pNewOC = (PVMDIR_LDAP_OBJECTCLASSES)pair.pValue;

        dwError = LwRtlHashMapFindKey(
                    pCurrentSchema->pOCMap,
                    (PVOID*)&pCurrentOC,
                    pNewOC->pLdapOC->oc_names[0]);
        if (dwError == 0 && pCurrentOC)
        {
            BOOLEAN bNeedUpdate = FALSE;
            dwError = _VmDirMergeObjectClasses(pCurrentOC, pNewOC, &bNeedUpdate);
            BAIL_ON_VMDIR_ERROR(dwError);

            if ( bNeedUpdate )
            {
                dwError = VmDirStringListAddStrClone(pCurrentOC->pszNormDef, pModStr->pAddOCStrList);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringListAddStrClone(pCurrentOC->pszOrgDef, pModStr->pDelOCStrList);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                if ( pCurrentOC->bLegacyFix )
                {
                    dwError = VmDirStringListAddStrClone(pNewOC->pszNormDef, pModStr->pAddOCStrList);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirStringListAddStrClone(pCurrentOC->pszOrgDef, pModStr->pDelOCStrList);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
        }
        else
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "[%s][%d] New objectclasses %s",
                            __FUNCTION__,__LINE__,
                            VDIR_SAFE_STRING(pNewOC->pLdapOC->oc_names[0]));

            dwError = VmDirStringListAddStrClone(pNewOC->pszNormDef, pModStr->pAddOCStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * For contentrule, we only add attributes to MAY and AUX list.
 * This function merge
 *  1 new MAY list into current MAY list.
 *  1 new AUX list into current AUX list.
 */
//static
DWORD
_VmDirMergeContentrule(
    PVMDIR_LDAP_CONTENTRULES  pCurrentCR,
    PVMDIR_LDAP_CONTENTRULES  pNewCR,
    BOOLEAN*                  pbNeedUpdate
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszMergeMaySet = NULL;
    PSTR*   ppszMergeAuxSet = NULL;

    dwError = VmDirMergeSet(
                    pCurrentCR->pLdapCR->cr_at_oids_may,
                    pNewCR->pLdapCR->cr_at_oids_may,
                    &ppszMergeMaySet);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppszMergeMaySet)
    {
        VmDirFreeStrArray(pCurrentCR->pLdapCR->cr_at_oids_may);
        pCurrentCR->pLdapCR->cr_at_oids_may = ppszMergeMaySet;
        ppszMergeMaySet = NULL;

        VMDIR_SAFE_FREE_MEMORY(pCurrentCR->pszNormDef);
        pCurrentCR->pszNormDef = ldap_contentrule2str(pCurrentCR->pLdapCR);
        assert(pCurrentCR->pszNormDef);

        *pbNeedUpdate = TRUE;
    }

    dwError = VmDirMergeSet(
                    pCurrentCR->pLdapCR->cr_oc_oids_aux,
                    pNewCR->pLdapCR->cr_oc_oids_aux,
                    &ppszMergeAuxSet);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppszMergeAuxSet)
    {
        VmDirFreeStrArray(pCurrentCR->pLdapCR->cr_oc_oids_aux);
        pCurrentCR->pLdapCR->cr_oc_oids_aux = ppszMergeAuxSet;
        ppszMergeAuxSet = NULL;

        VMDIR_SAFE_FREE_MEMORY(pCurrentCR->pszNormDef);
        pCurrentCR->pszNormDef = ldap_contentrule2str(pCurrentCR->pLdapCR);
        assert(pCurrentCR->pszNormDef);

        *pbNeedUpdate = TRUE;
    }

cleanup:
    VmDirFreeStrArray(ppszMergeMaySet);
    VmDirFreeStrArray(ppszMergeAuxSet);
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirMergeCR(
    PVMDIR_LDAP_SCHEMA_STRUCT  pCurrentSchema,
    PVMDIR_LDAP_SCHEMA_STRUCT  pNewSchema,
    PVMDIR_LDAP_SCHEMA_MOD_STR pModStr
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER             iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR             pair = {NULL, NULL};
    PVMDIR_LDAP_CONTENTRULES    pNewCR = NULL;
    PVMDIR_LDAP_CONTENTRULES    pCurrentCR = NULL;

    while (LwRtlHashMapIterate(pNewSchema->pCRMap, &iter, &pair))
    {
        pNewCR = (PVMDIR_LDAP_CONTENTRULES)pair.pValue;

        dwError = LwRtlHashMapFindKey(
                    pCurrentSchema->pCRMap,
                    (PVOID*)&pCurrentCR,
                    pNewCR->pLdapCR->cr_names[0]);
        if (dwError == 0 && pCurrentCR)
        {
            BOOLEAN bNeedUpdate = FALSE;
            dwError = _VmDirMergeContentrule(pCurrentCR, pNewCR, &bNeedUpdate);
            BAIL_ON_VMDIR_ERROR(dwError);

            if ( bNeedUpdate )
            {
                dwError = VmDirStringListAddStrClone(pCurrentCR->pszNormDef, pModStr->pAddCRStrList);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringListAddStrClone(pCurrentCR->pszOrgDef, pModStr->pDelCRStrList);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else
        {
            VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "[%s][%d] New contentrule %s",
                                            __FUNCTION__,__LINE__,
                                            VDIR_SAFE_STRING(pNewCR->pLdapCR->cr_names[0]));

            dwError = VmDirStringListAddStrClone(pNewCR->pszNormDef, pModStr->pAddCRStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

