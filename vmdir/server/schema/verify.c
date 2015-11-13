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
 * Filename: verify.c
 *
 * Abstract: verify schema instance and LDAP entry integrity
 *
 * Globals
 *
 */

#include "includes.h"

// both NULL || both have same SUP
#define VMDIR_TWO_SUP_COMPATIBLE( pszNewSup, pszOldSup )                                            \
    ( ( !pszNewSup && !pszOldSup )  ||                                                              \
      ( ( pszNewSup && pszOldSup ) && (VmDirStringCompareA( pszNewSup, pszOldSup, FALSE) == 0) )    \
    )

// both NULL || same value || adding new syntax
#define VMDIR_TWO_SYNTAX_COMPATIBLE( pszNewSyntax, pszOldSyntax )                                               \
    ( ( !pszNewSyntax && !pszOldSyntax )  ||                                                                    \
      ( ( pszNewSyntax && pszOldSyntax ) && (VmDirStringCompareA( pszNewSyntax, pszOldSyntax, FALSE) == 0) ) || \
      ( pszNewSyntax && !pszOldSyntax )                                                                         \
    )

// both NULL || same value || adding new matching rule
#define VMDIR_TWO_MR_COMPATIBLE( pszNewMR, pszOldMR )                                           \
    ( ( !pszNewMR && !pszOldMR )  ||                                                            \
      ( ( pszNewMR && pszOldMR ) && (VmDirStringCompareA( pszNewMR, pszOldMR, FALSE) == 0) ) || \
      ( pszNewMR && !pszOldMR )                                                                 \
    )

// iSize == 0 means unlimited
// e.g. max size can only increase
#define VMDIR_TWO_SIZE_COMPATILBE( iSizeNew, iSizeOld ) \
    ( (iSizeOld == 0 && iSizeNew == 0 ) ||              \
      (iSizeOld > 0 && iSizeNew >= iSizeOld )           \
    )

// e.g. single value tag from TRUE to FALSE
#define VMDIR_TWO_BOOL_COMPATILBE_T2F( bONE, bTWO )     \
    ( (bTWO == bONE) || ( bTWO == TRUE && bONE == FALSE ) )

// e.g. obsolete tag from FALSE to TRUE
#define VMDIR_TWO_BOOL_COMPATILBE_F2T( bONE, bTWO )     \
    ( (bONE == bTWO) || ( bONE == TRUE && bTWO == FALSE ) )

static
BOOLEAN
_VmDirSchemaVerifyATNameUnique(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    );

static
BOOLEAN
_VmDirSchemaVerifyOCNameUnique(
    PVDIR_SCHEMA_OC_COLLECTION pOCCollection
    );

static
BOOLEAN
_VmDirSchemaCRNameUnique(
    PVDIR_SCHEMA_CR_COLLECTION pCRCollection
    );

static
BOOLEAN
_VmDirSchemaVerifySRIDUnique(
    PVDIR_SCHEMA_SR_COLLECTION pSRCollection
    );

static
BOOLEAN
_VmDirSchemaVerifyNameformUnique(
    PVDIR_SCHEMA_NF_COLLECTION pNFCollection
    );

static
BOOLEAN
_VmDirSchemaVerifyATDesc(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    );

static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaATCompatibleCheck(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_INSTANCE   pNewInstance
    );

static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaListGECheck(
    USHORT  usSizeOne,
    PSTR*   ppszOne,
    USHORT  usSizeTwo,
    PSTR*   ppszTwo
    );

static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaListEqualCheck(
    USHORT  usSizeOne,
    PSTR*   ppszOne,
    USHORT  usSizeTwo,
    PSTR*   ppszTwo
    );

static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaOCCompatibleCheck(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_INSTANCE   pNewInstance
    );

BOOLEAN
VmDirSchemaVerifyIntegrity(
    PVDIR_SCHEMA_INSTANCE    pInstance
    )
{
    DWORD dwError = 0;
    BOOLEAN bResult = TRUE;

    if (pInstance->ats.usNumATs == 0 ||
        pInstance->ocs.usNumOCs == 0)
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (! _VmDirSchemaVerifyATDesc(&pInstance->ats))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (! _VmDirSchemaVerifyATNameUnique(&pInstance->ats))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (! _VmDirSchemaVerifyOCNameUnique(&pInstance->ocs))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (! _VmDirSchemaCRNameUnique(&pInstance->contentRules))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (! _VmDirSchemaVerifySRIDUnique(&pInstance->structureRules))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (! _VmDirSchemaVerifyNameformUnique(&pInstance->nameForms))
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //TODO, more verify?  OID name space...etc

cleanup:

    return bResult;

error:

    bResult = FALSE;

    goto cleanup;
}

/*
 *  compare two instances to determine if
 *  1. semantic compatible
 *  2. schema cache patch required
 */
DWORD
VmDirSchemaInstancePatchCheck(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_INSTANCE   pInstance,
    PBOOLEAN                pbCompatible,
    PBOOLEAN                pbNeedPatch
    )
{
    DWORD       dwError = 0;
    VDIR_SCHEMA_COMPATIBLE_LEVEL atLevel = VDIR_SCHEMA_NOT_COMPATIBLE;
    VDIR_SCHEMA_COMPATIBLE_LEVEL ocLevel = VDIR_SCHEMA_NOT_COMPATIBLE;

    if ( !pSchema || !pInstance || !pbCompatible || !pbNeedPatch )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    atLevel = _VmDirSchemaATCompatibleCheck( pSchema, pInstance );
    ocLevel = _VmDirSchemaOCCompatibleCheck( pSchema, pInstance );
    // TODO, contentrule, structurerule, nameform

    if ( atLevel >= VDIR_SCHEMA_COMPATIBLE
         &&
         ocLevel >= VDIR_SCHEMA_COMPATIBLE
       )
    {
        *pbCompatible = TRUE;

        if ( atLevel >= VDIR_SCHEMA_UPDATE_ENTRY_CACHE
             ||
             ocLevel >= VDIR_SCHEMA_UPDATE_ENTRY_CACHE
           )
        {
            *pbNeedPatch = TRUE;
        }
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * schema attributetype definition compatible level
 */
static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaATCompatibleCheck(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_INSTANCE   pNewInstance
    )
{
    DWORD       dwError = 0;
    PSTR        pszLocalErrMsg = NULL;
    size_t      iCnt = 0;
    size_t      iIdx = 0;
    BOOLEAN     bNeedPatch = FALSE;
    VDIR_SCHEMA_COMPATIBLE_LEVEL    rtnCompLevel = VDIR_SCHEMA_NOT_COMPATIBLE;

    if ( pSchema->ats.usNumATs > pNewInstance->ats.usNumATs )
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "missing attributetypes");
    }

    for ( iCnt = 0, iIdx = 0; iCnt < pSchema->ats.usNumATs; iCnt++)
    {
        // ats.pATDortName sort by attribute name already
        PVDIR_SCHEMA_AT_DESC pAttrDesc  = pSchema->ats.pATSortName + iCnt;
        BOOLEAN              bFound     = FALSE;

        while ( iIdx < pNewInstance->ats.usNumATs && !bFound )
        {
            PVDIR_SCHEMA_AT_DESC pNewAttrDesc = pNewInstance->ats.pATSortName + iIdx;

            iIdx++;

            if ( VmDirStringCompareA( pAttrDesc->pszName , pNewAttrDesc->pszName, FALSE) == 0 )
            {
                //TODO, handle ppszAliases compare
                if ( ! VMDIR_TWO_SIZE_COMPATILBE(pNewAttrDesc->uiMaxSize, pAttrDesc->uiMaxSize)
                     ||
                     // ignore bCollective
                     // ignore bNoUserModifiable
                     // ignore oid
                     // ignore desc
                     // ignore usage
                     ! VMDIR_TWO_BOOL_COMPATILBE_F2T(pNewAttrDesc->bObsolete, pAttrDesc->bObsolete)
                     ||
                     ! VMDIR_TWO_BOOL_COMPATILBE_T2F(pNewAttrDesc->bSingleValue, pAttrDesc->bSingleValue)
                     ||
                     ! VMDIR_TWO_SUP_COMPATIBLE( pNewAttrDesc->pszSup, pAttrDesc->pszSup)
                     ||
                     ! VMDIR_TWO_SYNTAX_COMPATIBLE( pNewAttrDesc->pszSyntaxName, pAttrDesc->pszSyntaxName)
                     ||
                     ! VMDIR_TWO_MR_COMPATIBLE( pNewAttrDesc->pszEqualityMRName, pAttrDesc->pszEqualityMRName)
                     ||
                     ! VMDIR_TWO_MR_COMPATIBLE( pNewAttrDesc->pszOrderingMRName, pAttrDesc->pszOrderingMRName)
                     ||
                     ! VMDIR_TWO_MR_COMPATIBLE( pNewAttrDesc->pszSubStringMRName, pAttrDesc->pszSubStringMRName)
                   )
                {
                    dwError = ERROR_INVALID_SCHEMA;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                                                 "incompatible attributetypes (%s)", VDIR_SAFE_STRING(pAttrDesc->pszName));
                }
                else
                {
                    bFound = TRUE;

                    if ( (pAttrDesc->uiMaxSize > 0 && pNewAttrDesc->uiMaxSize > pAttrDesc->uiMaxSize)
                         ||
                         pNewAttrDesc->bNoUserModifiable != pAttrDesc->bNoUserModifiable
                         ||
                         pNewAttrDesc->bObsolete != pAttrDesc->bObsolete
                         ||
                         pNewAttrDesc->bSingleValue != pAttrDesc->bSingleValue
                         )
                    {   // schema change allowed scenario
                        bNeedPatch = TRUE;
                    }
                }
            }
        }

        if ( !bFound )
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                                         "missing attributetypes (%s)", VDIR_SAFE_STRING(pAttrDesc->pszName));
        }
    }

    if ( pNewInstance->ats.usNumATs > pSchema->ats.usNumATs )
    {
        bNeedPatch = TRUE;  // have new attribute definition
    }

    rtnCompLevel = bNeedPatch ? VDIR_SCHEMA_UPDATE_ENTRY_CACHE : VDIR_SCHEMA_COMPATIBLE ;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return rtnCompLevel;

error:

    VmDirLog( LDAP_DEBUG_ANY, "_VmDirSchemaATCompatibleCheck failed (%d)(%s)",
                              dwError, VDIR_SAFE_STRING(pszLocalErrMsg));

    goto cleanup;
}

/*
 * list ONE is greater or equal than list TWO.  (both lists are sorted)
 * e.g. MUST attribute list
 * return VDIR_SCHEMA_COMPATIBLE         if ONE == TWO
 * return VDIR_SCHEMA_UPDATE_ENTRY_CACHE if ONE > TWO
 * reutrn VDIR_SCHEMA_NOT_COMPATIBLE     otherwise
 */
static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaListGECheck(
    USHORT  usSizeOne,
    PSTR*   ppszOne,
    USHORT  usSizeTwo,
    PSTR*   ppszTwo
    )
{
    VDIR_SCHEMA_COMPATIBLE_LEVEL    rtnCompLevel = VDIR_SCHEMA_NOT_COMPATIBLE;
    USHORT                          usCnt = 0;
    USHORT                          usIdx = 0;
    BOOLEAN                         bNeedPatch = FALSE;

    if ( usSizeOne >= usSizeTwo )
    {
        for (usCnt = 0, usIdx = 0; usCnt < usSizeTwo; usCnt++ )
        {
            BOOLEAN bFound = FALSE;

            while ( usIdx < usSizeOne && !bFound )
            {
                if ( VmDirStringCompareA( ppszOne[usIdx], ppszTwo[usCnt], FALSE ) == 0 )
                {
                    bFound = TRUE;
                }
                else
                {
                    bNeedPatch = TRUE;
                }

                usIdx++;
            }

            if ( !bFound )
            {   // stop if element in TWO not found in ONE
                break;
            }
        }

        if ( usCnt == usSizeTwo )
        {
            if ( bNeedPatch )
            {
                rtnCompLevel = VDIR_SCHEMA_UPDATE_ENTRY_CACHE;
            }
            else
            {
                rtnCompLevel = VDIR_SCHEMA_COMPATIBLE;
            }
        }
    }

    return rtnCompLevel;
}

static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaListEqualCheck(
    USHORT  usSizeOne,
    PSTR*   ppszOne,
    USHORT  usSizeTwo,
    PSTR*   ppszTwo
    )
{
    VDIR_SCHEMA_COMPATIBLE_LEVEL    rtnCompLevel = VDIR_SCHEMA_NOT_COMPATIBLE;
    USHORT                          usCnt = 0;

    if ( usSizeOne == usSizeTwo )
    {
        for ( usCnt = 0; usCnt < usSizeOne; usCnt++ )
        {
            if ( VmDirStringCompareA( ppszOne[usCnt], ppszTwo[usCnt], FALSE ) != 0 )
            {
                break;
            }
        }

        if ( usCnt == usSizeOne )
        {
            rtnCompLevel = VDIR_SCHEMA_COMPATIBLE;
        }
    }

    return rtnCompLevel;
}

/*
 * schema objectclasses definition compatible level
 */
static
VDIR_SCHEMA_COMPATIBLE_LEVEL
_VmDirSchemaOCCompatibleCheck(
    PVDIR_SCHEMA_INSTANCE   pSchema,
    PVDIR_SCHEMA_INSTANCE   pNewInstance
    )
{
    DWORD       dwError = 0;
    PSTR        pszLocalErrMsg = NULL;
    size_t      iCnt = 0;
    size_t      iIdx = 0;
    BOOLEAN     bNeedPatch = FALSE;
    VDIR_SCHEMA_COMPATIBLE_LEVEL    rtnCompLevel = VDIR_SCHEMA_NOT_COMPATIBLE;

    if ( pSchema->ocs.usNumOCs > pNewInstance->ocs.usNumOCs )
    {
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "missing objectclasses");
    }

    for ( iCnt = 0, iIdx = 0; iCnt < pSchema->ocs.usNumOCs; iCnt++)
    {
        // ocs.pOCSortName sort by attribute name already
        PVDIR_SCHEMA_OC_DESC pOCDesc  = pSchema->ocs.pOCSortName + iCnt;
        BOOLEAN              bFound     = FALSE;

        while ( iIdx < pNewInstance->ocs.usNumOCs && !bFound )
        {
            PVDIR_SCHEMA_OC_DESC pNewOCDesc = pNewInstance->ocs.pOCSortName + iIdx;

            iIdx++;

            if ( VmDirStringCompareA( pOCDesc->pszName , pNewOCDesc->pszName, FALSE) == 0 )
            {
                VDIR_SCHEMA_COMPATIBLE_LEVEL  supCompLevel =  // must equal
                            _VmDirSchemaListEqualCheck( pOCDesc->usNumSupOCs, pOCDesc->ppszSupOCs,
                                                        pNewOCDesc->usNumSupOCs, pNewOCDesc->ppszSupOCs);
                VDIR_SCHEMA_COMPATIBLE_LEVEL  mustATCompLevel = // MUST list can be less
                             _VmDirSchemaListGECheck( pOCDesc->usNumMustATs, pOCDesc->ppszMustATs,
                                                      pNewOCDesc->usNumMustATs, pNewOCDesc->ppszMustATs);
                VDIR_SCHEMA_COMPATIBLE_LEVEL  mayATCompLevel =  // MAY list can be more
                             _VmDirSchemaListGECheck( pNewOCDesc->usNumMayATs, pNewOCDesc->ppszMayATs,
                                                      pOCDesc->usNumMayATs, pOCDesc->ppszMayATs);
                VDIR_SCHEMA_COMPATIBLE_LEVEL  auxOCCompLevel =  // AUX list can be more
                             _VmDirSchemaListGECheck( pNewOCDesc->usNumAuxOCs, pNewOCDesc->ppszAuxOCs,
                                                      pOCDesc->usNumAuxOCs, pOCDesc->ppszAuxOCs);

                if ( pNewOCDesc->type != pOCDesc->type
                     ||
                     supCompLevel == VDIR_SCHEMA_NOT_COMPATIBLE
                     ||
                     mustATCompLevel == VDIR_SCHEMA_NOT_COMPATIBLE
                     ||
                     mayATCompLevel == VDIR_SCHEMA_NOT_COMPATIBLE
                     ||
                     auxOCCompLevel == VDIR_SCHEMA_NOT_COMPATIBLE
                     // ignore bCollective
                     // ignore bNoUserModifiable
                     // ignore pszOid
                     // ignore pszDesc
                   )
                {
                    dwError = ERROR_INVALID_SCHEMA;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                                                 "incompatible objectclass/contentrule (%s)",
                                                 VDIR_SAFE_STRING(pOCDesc->pszName));
                }
                else
                {
                    bFound = TRUE;

                    if ( supCompLevel == VDIR_SCHEMA_UPDATE_ENTRY_CACHE
                         ||
                         mustATCompLevel == VDIR_SCHEMA_UPDATE_ENTRY_CACHE
                         ||
                         mayATCompLevel == VDIR_SCHEMA_UPDATE_ENTRY_CACHE
                         ||
                         auxOCCompLevel == VDIR_SCHEMA_UPDATE_ENTRY_CACHE
                       )
                    {
                        bNeedPatch = TRUE;
                    }
                }
            }
        }

        if ( !bFound )
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                                         "missing objectclasses (%s)", VDIR_SAFE_STRING(pOCDesc->pszName));
        }
    }

    if ( pNewInstance->ocs.usNumOCs > pSchema->ocs.usNumOCs )
    {
        bNeedPatch = TRUE;  // have new objectclass definition
    }

    rtnCompLevel = bNeedPatch ? VDIR_SCHEMA_UPDATE_ENTRY_CACHE : VDIR_SCHEMA_COMPATIBLE ;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return rtnCompLevel;

error:

    VmDirLog( LDAP_DEBUG_ANY, "_VmDirSchemaOCCompatibleCheck failed (%d)(%s)",
                              dwError, VDIR_SAFE_STRING(pszLocalErrMsg));

    goto cleanup;
}

/*
 *  get a normalized form of AttributeTypes.
 */
VOID
VdirSchemaVerifyATDescPrint(
    PVDIR_SCHEMA_AT_DESC  pATDesc,
    PSTR*                 ppszOut
    )
{
    DWORD    dwError = 0;
    PSTR    pszBuf = NULL;
    PSTR    pszTmp = NULL;
    PSTR    pszAlias = NULL;

    if (pATDesc->ppszAliases)
    {
        PSTR*    ppTmp = NULL;
        size_t   iSize = 0;

        for (iSize = 0, ppTmp = pATDesc->ppszAliases;
            *ppTmp;
            iSize = iSize + VmDirStringLenA(*ppTmp) + 3, ppTmp++) ;

        dwError = VmDirAllocateMemory(
                iSize + 1,
                (PVOID*)&pszAlias);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (ppTmp = pATDesc->ppszAliases; *ppTmp; ppTmp++)
        {
            VmDirStringCatA(pszAlias, (iSize + 1), "'");
            VmDirStringCatA(pszAlias, (iSize + 1), *ppTmp);
            VmDirStringCatA(pszAlias, (iSize + 1), "' ");
        }
    }

    dwError = VmDirAllocateStringAVsnprintf(
            &pszBuf,
            "AttributeTypes: ( %s NAME ( '%s'%s ) SUP '%s' "
            "EQUALITY '%s' ORDERING '%s' SUBSTRING '%s' SYNTAX '%s'{%d}",
            pATDesc->pszOid,
            pATDesc->pszName,
            VDIR_SAFE_STRING(pszAlias),
            VDIR_SAFE_STRING(pATDesc->pszSup),
            VDIR_SAFE_STRING(pATDesc->pszEqualityMRName),
            VDIR_SAFE_STRING(pATDesc->pszOrderingMRName),
            VDIR_SAFE_STRING(pATDesc->pszSubStringMRName),
            VDIR_SAFE_STRING(pATDesc->pszSyntaxName),
            pATDesc->uiMaxSize);

    *ppszOut = pszBuf;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszTmp);
    VMDIR_SAFE_FREE_MEMORY(pszAlias);
    return;

error:

    VMDIR_SAFE_FREE_MEMORY(pszBuf);
    goto cleanup;
}

VOID
VdirSchemaVerifyOCDescPrint(
    PVDIR_SCHEMA_OC_DESC  pOCDesc,
    PSTR*                 ppszOut
    )
{
    DWORD    dwError = 0;
    PSTR    pszBuf = NULL;
    PSTR    pszTmp = NULL;
    PSTR    pszMust = NULL;
    PSTR    pszMay = NULL;
    PSTR    pszType = NULL;

    if (pOCDesc->type == VDIR_OC_ABSTRACT)
    {
        pszType = "ABSTRACT";
    }
    else if  (pOCDesc->type == VDIR_OC_STRUCTURAL)
    {
        pszType = "STRUCTURAL";
    }
    else if (pOCDesc->type == VDIR_OC_AUXILIARY)
    {
        pszType = "AUXILIARY";
    }
    else    pszType = "UNKNOWN";

    if (pOCDesc->ppszMustATs)
    {
        PSTR*    ppTmp = NULL;
        size_t   iSize = 0;

        for (iSize = 0, ppTmp = pOCDesc->ppszMustATs;
            *ppTmp;
            iSize = iSize + VmDirStringLenA(*ppTmp) + 3, ppTmp++) ;

        dwError = VmDirAllocateMemory(
                iSize + 1,
                (PVOID*)&pszMust);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (ppTmp = pOCDesc->ppszMustATs; *ppTmp; ppTmp++)
        {
            VmDirStringCatA(pszMust, (iSize + 1), *ppTmp);
            if (*(ppTmp+1))
            {
                VmDirStringCatA(pszMust, (iSize + 1), " $ ");
            }
        }
    }

    if (pOCDesc->ppszMayATs)
    {
        PSTR*    ppTmp = NULL;
        size_t   iSize = 0;

        for (iSize = 0, ppTmp = pOCDesc->ppszMayATs;
            *ppTmp;
            iSize = iSize + VmDirStringLenA(*ppTmp) + 3, ppTmp++) ;

        dwError = VmDirAllocateMemory(
                iSize + 1,
                (PVOID*)&pszMay);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (ppTmp = pOCDesc->ppszMayATs; *ppTmp; ppTmp++)
        {
            VmDirStringCatA(pszMay, (iSize + 1), *ppTmp);
            if (*(ppTmp+1))
            {
                VmDirStringCatA(pszMay, (iSize + 1), " $ ");
            }
        }
    }

    dwError = VmDirAllocateStringAVsnprintf(
            &pszBuf,
            "Objectclasses: ( %s NAME '%s' SUP '%s' %s "
            "MUAT ( %s ) MAY ( %s ) )",
            pOCDesc->pszOid,
            pOCDesc->pszName,
            VDIR_SAFE_STRING(pOCDesc->ppszSupOCs ? pOCDesc->ppszSupOCs[0] : ""),
            pszType,
            VDIR_SAFE_STRING(pszMust? &pszMust[2] : pszMust),
            VDIR_SAFE_STRING(pszMay? &pszMay[2] : pszMay));

    *ppszOut = pszBuf;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszTmp);
    VMDIR_SAFE_FREE_MEMORY(pszMust);
    VMDIR_SAFE_FREE_MEMORY(pszMay);

    return;

error:

    VMDIR_SAFE_FREE_MEMORY(pszBuf);

    goto cleanup;
}

/*
 * Make sure we have a unique name space in objectclass name definition
 */
static
BOOLEAN
_VmDirSchemaVerifyOCNameUnique(
    PVDIR_SCHEMA_OC_COLLECTION pOCCollection
    )
{
    int dwCnt = 0;

    // pOCSortByName is sorted by pszName by now
    for (dwCnt = 0; dwCnt < pOCCollection->usNumOCs - 1; dwCnt++)
    {
        if (VmDirStringCompareA(pOCCollection->pOCSortName[dwCnt  ].pszName,
                       pOCCollection->pOCSortName[dwCnt+1].pszName, FALSE) == 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "schemaVerifyOCNameUnique:(%s).",
                    VDIR_SAFE_STRING(pOCCollection->pOCSortName[dwCnt  ].pszName));

            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Make sure we have a unique name space in attribute name definition
 */
static
BOOLEAN
_VmDirSchemaVerifyATNameUnique(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    )
{
    int dwCnt = 0;

    // pATSortByName is sorted by pszName by now
    for (dwCnt = 0; dwCnt < pATCollection->usNumATs - 1; dwCnt++)
    {
        if (VmDirStringCompareA(pATCollection->pATSortName[dwCnt  ].pszName,
                       pATCollection->pATSortName[dwCnt+1].pszName, FALSE) == 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "schemaVerifyATNameUnique:(%s).",
                    VDIR_SAFE_STRING(pATCollection->pATSortName[dwCnt  ].pszName));

            return FALSE;
        }
    }

    return TRUE;
}

/*
 * Make sure we have a unique name space in content rule definition
 */
static
BOOLEAN
_VmDirSchemaCRNameUnique(
    PVDIR_SCHEMA_CR_COLLECTION pCRCollection
    )
{
    int dwCnt = 0;

    if (pCRCollection->pContentSortName)
    {
        // pContentSortName is sorted by pszName by now
        for (dwCnt = 0; dwCnt < pCRCollection->usNumContents - 1; dwCnt++)
        {
            if (VmDirStringCompareA(pCRCollection->pContentSortName[dwCnt  ].pszName,
                           pCRCollection->pContentSortName[dwCnt+1].pszName, FALSE) == 0)
            {
                VmDirLog( LDAP_DEBUG_ANY, "schemaVerifyCRNameUnique:(%s).",
                        VDIR_SAFE_STRING(pCRCollection->pContentSortName[dwCnt  ].pszName));

                return FALSE;
            }
        }
    }

    return TRUE;
}

/*
 * Make sure we have a unique ID space in structure rule definition
 */
static
BOOLEAN
_VmDirSchemaVerifySRIDUnique(
    PVDIR_SCHEMA_SR_COLLECTION pSRCollection
    )
{
    int dwCnt = 0;

    if (pSRCollection->pStructureSortRuleID)
    {
        // pStructureSortRuleID is sorted by pszRuleID by now
        for (dwCnt = 0; dwCnt < pSRCollection->usNumStructures - 1; dwCnt++)
        {
            if (VmDirStringCompareA(pSRCollection->pStructureSortRuleID[dwCnt  ].pszRuleID,
                           pSRCollection->pStructureSortRuleID[dwCnt+1].pszRuleID, FALSE) == 0)
            {
                VmDirLog( LDAP_DEBUG_ANY, "schemaVerifySRIDUnique:(%s).",
                        VDIR_SAFE_STRING(pSRCollection->pStructureSortRuleID[dwCnt  ].pszRuleID));

                return FALSE;
            }
        }
    }

    return TRUE;
}

/*
 * Make sure we have a unique name in nameform definition
 */
static
BOOLEAN
_VmDirSchemaVerifyNameformUnique(
    PVDIR_SCHEMA_NF_COLLECTION pNFCollection
    )
{
    int         dwCnt = 0;
    BOOLEAN     bRtn = TRUE;

    if (pNFCollection->pNameFormSortName)
    {
        // pNameFormSortName is sorted by pszName by now
        for (dwCnt = 0; dwCnt < pNFCollection->usNumNameForms - 1; dwCnt++)
        {
            if ( VmDirStringCompareA(  pNFCollection->pNameFormSortName[dwCnt  ].pszName,
                                       pNFCollection->pNameFormSortName[dwCnt+1].pszName,
                                       FALSE) == 0
               )
            {
                VmDirLog( LDAP_DEBUG_ANY, "schemaVerifyNFIDUnique:(%s).",
                          VDIR_SAFE_STRING(pNFCollection->pNameFormSortName[dwCnt  ].pszName));

                bRtn = FALSE;
                break;
            }
        }
    }

    return bRtn;
}

/*
 * TODO,
 */
static
BOOLEAN
_VmDirSchemaVerifyATDesc(
    PVDIR_SCHEMA_AT_COLLECTION pATCollection
    )
{
    DWORD dwError = 0;
    DWORD dwCnt = 0;
    BOOLEAN bResult = TRUE;

    for (dwCnt = 0; dwCnt < pATCollection->usNumATs; dwCnt++)

    {
        PVDIR_SCHEMA_AT_DESC pATDesc = pATCollection->pATSortName + dwCnt;

        if (!pATDesc->pszOid    ||
            !pATDesc->pszName    ||
            !pATDesc->pszSyntaxName)
        {
            VmDirLog( LDAP_DEBUG_ANY, "schemaVerifyATDesc:(%s).",
                    VDIR_SAFE_STRING(pATDesc->pszName));

            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    return bResult;

error:

    bResult = FALSE;

    goto cleanup;
}
