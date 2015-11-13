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
 * Filename: parse.c
 *
 * Abstract: parse string into schema descriptors
 *
 * Globals
 *
 */

#include "includes.h"

static
DWORD
schemaParseParenthesisItems(
    PSTR**       pppszList,
    PSTR*        ppRest,
    PUSHORT      pusSize
    );

VOID
VmDirSchemaATDescContentFree(
    PVDIR_SCHEMA_AT_DESC pATDesc
    )
{
    if ( pATDesc )
    {
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszDefinition);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszName);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszOid);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszDesc);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszSup);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszSyntaxName);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszEqualityMRName);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszOrderingMRName);
        VMDIR_SAFE_FREE_STRINGA(pATDesc->pszSubStringMRName);

        if (pATDesc->ppszAliases)
        {
            VmDirFreeStringArrayA(pATDesc->ppszAliases);
            VMDIR_SAFE_FREE_MEMORY(pATDesc->ppszAliases);
        }

        if (pATDesc->pADAttributeSchemaEntry)
        {
            VmDirFreeEntry(pATDesc->pADAttributeSchemaEntry);
        }
    }

    return;
}

VOID
VmDirSchemaOCDescContentFree(
    PVDIR_SCHEMA_OC_DESC pOCDesc
    )
{
    if ( pOCDesc )
    {
        VMDIR_SAFE_FREE_STRINGA(pOCDesc->pszDefinition);
        VMDIR_SAFE_FREE_STRINGA(pOCDesc->pszName);
        VMDIR_SAFE_FREE_STRINGA(pOCDesc->pszOid);
        VMDIR_SAFE_FREE_STRINGA(pOCDesc->pszDesc);

        if (pOCDesc->ppszSupOCs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszSupOCs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszSupOCs);
        }

        if (pOCDesc->ppszMustATs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszMustATs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszMustATs);
        }

        if (pOCDesc->ppszMayATs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszMayATs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszMayATs);
        }

        if (pOCDesc->ppszAuxOCs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszAuxOCs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszAuxOCs);
        }

        if (pOCDesc->ppszAllowedParentOCs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszAllowedParentOCs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszAllowedParentOCs);
        }
        if (pOCDesc->ppszAllowedChildOCs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszAllowedChildOCs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszAllowedChildOCs);
        }

        if (pOCDesc->ppszMustRDNs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszMustRDNs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszMustRDNs);
        }

        if (pOCDesc->ppszMayRDNs)
        {
            VmDirFreeStringArrayA(pOCDesc->ppszMayRDNs);
            VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppszMayRDNs);
        }

        VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppSupOCs);
        VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppMustATs);
        VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppMayATs);
        VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppAllMayATs);
        VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppAllMustATs);
        VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppAllNotATs);
        VMDIR_SAFE_FREE_MEMORY(pOCDesc->ppAllowedAuxOCs);

        if (pOCDesc->pADClassSchemaEntry)
        {
            VmDirFreeEntry(pOCDesc->pADClassSchemaEntry);
        }
    }

    return;
}

VOID
VmDirSchemaContentDescContentFree(
    PVDIR_SCHEMA_CR_DESC pContentDesc
    )
{
    if ( pContentDesc )
    {
        VMDIR_SAFE_FREE_STRINGA(pContentDesc->pszDefinition);
        VMDIR_SAFE_FREE_STRINGA(pContentDesc->pszName);
        VMDIR_SAFE_FREE_STRINGA(pContentDesc->pszOid);
        VMDIR_SAFE_FREE_STRINGA(pContentDesc->pszDesc);

        if (pContentDesc->ppszAuxOCs)
        {
            VmDirFreeStringArrayA(pContentDesc->ppszAuxOCs);
            VMDIR_SAFE_FREE_MEMORY(pContentDesc->ppszAuxOCs);
        }

        if (pContentDesc->ppszMustATs)
        {
            VmDirFreeStringArrayA(pContentDesc->ppszMustATs);
            VMDIR_SAFE_FREE_MEMORY(pContentDesc->ppszMustATs);
        }

        if (pContentDesc->ppszMayATs)
        {
            VmDirFreeStringArrayA(pContentDesc->ppszMayATs);
            VMDIR_SAFE_FREE_MEMORY(pContentDesc->ppszMayATs);
        }

        if (pContentDesc->ppszNotATs)
        {
            VmDirFreeStringArrayA(pContentDesc->ppszNotATs);
            VMDIR_SAFE_FREE_MEMORY(pContentDesc->ppszNotATs);
        }
    }

    return;
}

VOID
VmDirSchemaStructureDescContentFree(
    PVDIR_SCHEMA_SR_DESC pStructureDesc
    )
{
    if ( pStructureDesc )
    {
        VMDIR_SAFE_FREE_STRINGA(pStructureDesc->pszRuleID);
        VMDIR_SAFE_FREE_STRINGA(pStructureDesc->pszName);
        VMDIR_SAFE_FREE_STRINGA(pStructureDesc->pszOid);
        VMDIR_SAFE_FREE_STRINGA(pStructureDesc->pszDesc);
        VMDIR_SAFE_FREE_STRINGA(pStructureDesc->pszNameform);

        if (pStructureDesc->ppszSupRulesID)
        {
            VmDirFreeStringArrayA(pStructureDesc->ppszSupRulesID);
            VMDIR_SAFE_FREE_MEMORY(pStructureDesc->ppszSupRulesID);
        }
    }

    return;
}

VOID
VmDirSchemaNameformDescContentFree(
    PVDIR_SCHEMA_NF_DESC pNameformDesc
    )
{
    if ( pNameformDesc )
    {
        VMDIR_SAFE_FREE_STRINGA(pNameformDesc->pszName);
        VMDIR_SAFE_FREE_STRINGA(pNameformDesc->pszOid);
        VMDIR_SAFE_FREE_STRINGA(pNameformDesc->pszDesc);
        VMDIR_SAFE_FREE_STRINGA(pNameformDesc->pszStructOC);

        if (pNameformDesc->ppszMustATs)
        {
            VmDirFreeStringArrayA(pNameformDesc->ppszMustATs);
            VMDIR_SAFE_FREE_MEMORY(pNameformDesc->ppszMustATs);
        }

        if (pNameformDesc->ppszMayATs)
        {
            VmDirFreeStringArrayA(pNameformDesc->ppszMayATs);
            VMDIR_SAFE_FREE_MEMORY(pNameformDesc->ppszMayATs);
        }
    }

    return;
}


DWORD
VmDirSchemaParseStrToATDesc(
    const char*             pStr,
    PVDIR_SCHEMA_AT_DESC    pATDesc
    )
{
    DWORD   dwError = 0;
    BOOLEAN bHasOID = FALSE;
    PSTR    pToken = NULL;
    PSTR    pRest = NULL;
    char*   pBuf = NULL;

    if ( !pStr || !pATDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pStr, &pBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmdDirSchemaParseNormalizeElement( pBuf );

    pToken = VmDirStringTokA(pBuf, SCHEMA_GEN_TOKEN_SEP, &pRest);
    if (!pToken)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pToken, ATTRIBUTETYPS_TAG, FALSE) == 0)
    {    // ignore starting "attributetypes:" if exists
        GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
    }

    if (! IS_L_PARENTHESIS_STR(pToken))
    {
        dwError = ERROR_INVALID_ATTRIBUTETYPES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAVsnprintf( &pATDesc->pszDefinition, "( %s", pRest );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pToken = VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest);
         pToken;
         pToken = VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest))
    {
        VDIR_BERVALUE bv = VDIR_BERVALUE_INIT;
        bv.lberbv.bv_val = pToken;
        bv.lberbv.bv_len = VmDirStringLenA(pToken);

        if (!bHasOID && syntaxOID(&bv))
        {
            bHasOID = TRUE;
            dwError = VmDirAllocateStringA(pToken, &pATDesc->pszOid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "NAME", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);

            if (IS_EMPTY_PARENTHESIS_STR(pToken))
            {
                dwError = ERROR_INVALID_ATTRIBUTETYPES;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else if (! IS_L_PARENTHESIS_STR(pToken))
            { // NAME 'cn'
                dwError = VmDirAllocateStringA(pToken, &pATDesc->pszName);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            { // NAME ( 'cn' 'commonName'....)
                int iCnt = 0;
                size_t iSize = 5;

                dwError = VmDirAllocateMemory(
                        sizeof(PSTR) * (iSize+1),
                        (PVOID*)&pATDesc->ppszAliases);
                BAIL_ON_VMDIR_ERROR(dwError);

                for (pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest),iCnt=0;
                     pToken && !IS_R_PARENTHESIS_STR(pToken);
                     pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest),iCnt++)
                {
                    if (iCnt == iSize)
                    {
                        size_t iOldSize = iSize;
                        iSize = iSize * 2;
                        dwError = VmDirReallocateMemoryWithInit(
                                pATDesc->ppszAliases,
                                (PVOID*)&pATDesc->ppszAliases,
                                (iSize+1) * sizeof(PSTR),
                                (iOldSize) * sizeof(PSTR));
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }

                    dwError = VmDirAllocateStringA(pToken,
                            &pATDesc->ppszAliases[iCnt]);
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                if (!pToken || iCnt < 1)
                {
                    dwError = ERROR_INVALID_ATTRIBUTETYPES;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                // move pATDesc->ppszAliases[0] to pATDecs->pszName
                pATDesc->pszName = pATDesc->ppszAliases[0];
                pATDesc->ppszAliases[0] = NULL;

                // move pATDesc->ppszAliases[last] to pATDesc->ppszAliases[0]
                if (pATDesc->ppszAliases[iCnt-1])
                {
                    pATDesc->ppszAliases[0] = pATDesc->ppszAliases[iCnt-1];
                    pATDesc->ppszAliases[iCnt-1] = NULL;
                }
            }
        }
        else if (VmDirStringCompareA(pToken, "DESC", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_DESC_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pATDesc->pszDesc);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "OBSOLETE", FALSE) == 0)
        {   //TODO, semantics not supported yet.
            pATDesc->bObsolete = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "SUP", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken,
                    &pATDesc->pszSup);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "EQUALITY", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken,
                    &pATDesc->pszEqualityMRName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "ORDERING", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken,
                    &pATDesc->pszOrderingMRName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "SUBSTR", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken,
                    &pATDesc->pszSubStringMRName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "SYNTAX", FALSE) == 0)
        { // SYNTAX 1.2.3.4.5{512}  size limit is optional
            PSTR ptrLeft = NULL;
            PSTR ptrRight = NULL;

            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            ptrLeft  = VmDirStringChrA(pToken, '{');
            ptrRight = VmDirStringChrA(pToken, '}');

            if ((ptrLeft == NULL && ptrRight != NULL) ||
                (ptrLeft != NULL && ptrRight == NULL) )
            {
                dwError = ERROR_INVALID_ATTRIBUTETYPES;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (ptrLeft == NULL)
            {
                dwError = VmDirAllocateStringA(pToken,
                        &pATDesc->pszSyntaxName);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                *ptrLeft = '\0';
                *ptrRight = '\0';

                dwError = VmDirAllocateStringA(pToken,
                        &pATDesc->pszSyntaxName);
                BAIL_ON_VMDIR_ERROR(dwError);

                pATDesc->uiMaxSize = atoi(ptrLeft+1);

                *ptrLeft = '{';
                *ptrRight = '}';
            }
        }
        else if (VmDirStringCompareA(pToken, "SINGLE-VALUE", FALSE) == 0)
        {
            pATDesc->bSingleValue = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "COLLECTIVE", FALSE) == 0)
        {
            pATDesc->bCollective = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "NO-USER-MODIFICATION", FALSE) == 0)
        {   //TODO, semantic not supported yet.
            pATDesc->bNoUserModifiable = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "USAGE", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (VmDirStringCompareA(pToken, "userApplications", FALSE) == 0)
            {
                pATDesc->usage = VDIR_ATTRIBUTETYPE_USER_APPLICATIONS;
            }
            else if (VmDirStringCompareA(pToken, "directoryOperation", FALSE) == 0)
            {
                pATDesc->usage = VDIR_ATTRIBUTETYPE_DIRECTORY_OPERATION;
            }
            else if (VmDirStringCompareA(pToken, "distributedOperation", FALSE) == 0)
            {
                pATDesc->usage = VDIR_ATTRIBUTETYPE_DISTRIBUTED_OPERATION;
            }
            else if (VmDirStringCompareA(pToken, "dSAOperation", FALSE) == 0)
            {
                pATDesc->usage =    VDIR_ATTRIBUTETYPE_DSA_OPERATION;
            }
            else
            {
                dwError = ERROR_INVALID_ATTRIBUTETYPES;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
        else if (IS_R_PARENTHESIS_STR(pToken))
        {
            ;
        }
        else
        {
            dwError = ERROR_INVALID_ATTRIBUTETYPES;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pBuf);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VdirSchemaParseStrToATDesc:(%s)(%s)",
              VDIR_SAFE_STRING(pToken), VDIR_SAFE_STRING(pStr) );

    VmDirSchemaATDescContentFree(pATDesc);

    goto cleanup;
}

DWORD
VmDirSchemaParseStrToOCDesc(
    const char*             pStr,
    PVDIR_SCHEMA_OC_DESC    pOCDesc
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasOID = FALSE;
    PSTR  pToken = NULL;
    PSTR  pRest = NULL;
    BOOLEAN bHasType = FALSE;
    char*  pBuf = NULL;

    if ( !pStr || !pOCDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pStr, &pBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmdDirSchemaParseNormalizeElement( pBuf );

    pToken = VmDirStringTokA(pBuf, SCHEMA_GEN_TOKEN_SEP, &pRest);
    if (!pToken)
    {
        dwError = ERROR_INVALID_OBJECTCLASSES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pToken, OBJECTCLASSES_TAG, FALSE) == 0)
    {    // ignore leading "objectclasses:" if exists
        GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
    }

    if (! IS_L_PARENTHESIS_STR(pToken))
    {
        dwError = ERROR_INVALID_OBJECTCLASSES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAVsnprintf( &pOCDesc->pszDefinition, "( %s", pRest );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest);
        pToken;
        pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest))
    {

        VDIR_BERVALUE bv = VDIR_BERVALUE_INIT;
        bv.lberbv.bv_val = pToken;
        bv.lberbv.bv_len = VmDirStringLenA(pToken);

        if (!bHasOID && syntaxOID(&bv))
        {
            bHasOID = TRUE;
            dwError = VmDirAllocateStringA(pToken, &pOCDesc->pszOid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "NAME", FALSE) == 0)
        {
            //TODO, handle ( ) case?
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pOCDesc->pszName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "DESC", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_DESC_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pOCDesc->pszDesc);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "OBSOLETE", FALSE) == 0)
        {   //TODO, semantics not supported yet.
            pOCDesc->bObsolete = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "SUP", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(
                    &pOCDesc->ppszSupOCs,
                    &pRest,
                    &pOCDesc->usNumSupOCs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "ABSTRACT", FALSE) == 0)
        {
            if (bHasType)
            {
                dwError = ERROR_INVALID_OBJECTCLASSES;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            pOCDesc->type = VDIR_OC_ABSTRACT;
            bHasType = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "STRUCTURAL", FALSE) == 0)
        {
            if (bHasType)
            {
                dwError = ERROR_INVALID_OBJECTCLASSES;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            pOCDesc->type = VDIR_OC_STRUCTURAL;
            bHasType = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "AUXILIARY", FALSE) == 0)
        {
            if (bHasType)
            {
                dwError = ERROR_INVALID_OBJECTCLASSES;
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            pOCDesc->type = VDIR_OC_AUXILIARY;
            bHasType = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "MUST", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(
                    &pOCDesc->ppszMustATs,
                    &pRest,
                    &pOCDesc->usNumMustATs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "MAY", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(
                    &pOCDesc->ppszMayATs,
                    &pRest,
                    &pOCDesc->usNumMayATs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (IS_R_PARENTHESIS_STR(pToken))
        {
            ;
        }
        else
        {
            dwError = ERROR_INVALID_OBJECTCLASSES;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

    }

    if (! bHasType)
    {   // default to STRUCTURAL
        pOCDesc->type = VDIR_OC_STRUCTURAL;
    }

    if (pOCDesc->type == VDIR_OC_STRUCTURAL && pOCDesc->usNumSupOCs == 0)
    {   // default top as parent to structural objectclass
        dwError = VmDirAllocateMemory(sizeof(PSTR) * 2, (PVOID*)&pOCDesc->ppszSupOCs);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(OC_TOP, &pOCDesc->ppszSupOCs[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        pOCDesc->usNumSupOCs = 1;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pBuf);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VdirSchemaParseStrToOCDesc:(%s)(%s)",
              VDIR_SAFE_STRING(pToken), VDIR_SAFE_STRING(pStr) );

    VmDirSchemaOCDescContentFree(pOCDesc);

    goto cleanup;
}

DWORD
VmDirSchemaParseStrToContentDesc(
    const char*            pStr,
    PVDIR_SCHEMA_CR_DESC   pContentDesc
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasOID = FALSE;
    PSTR  pToken = NULL;
    PSTR  pRest = NULL;
    char*  pBuf = NULL;

    if ( !pStr || !pContentDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pStr, &pBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmdDirSchemaParseNormalizeElement( pBuf );

    pToken = VmDirStringTokA(pBuf, SCHEMA_GEN_TOKEN_SEP, &pRest);
    if (!pToken)
    {
        dwError = ERROR_INVALID_DITCONTENTRULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pToken, "ditcontentrules:", FALSE) == 0)
    {    // ignore leading ditcontentrules:
        GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
    }

    if (! IS_L_PARENTHESIS_STR(pToken))
    {
        dwError = ERROR_INVALID_DITCONTENTRULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringAVsnprintf( &pContentDesc->pszDefinition, "( %s", pRest );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest);
        pToken;
        pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest))
    {

        VDIR_BERVALUE bv = VDIR_BERVALUE_INIT;
        bv.lberbv.bv_val = pToken;
        bv.lberbv.bv_len = VmDirStringLenA(pToken);

        if (!bHasOID && syntaxOID(&bv))
        {
            bHasOID = TRUE;
            dwError = VmDirAllocateStringA(pToken, &pContentDesc->pszOid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "NAME", FALSE) == 0)
        {
            //TODO, handle ( ) case?
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pContentDesc->pszName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "DESC", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_DESC_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pContentDesc->pszDesc);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "OBSOLETE", FALSE) == 0)
        {   //TODO, semantic not supported yet.
            pContentDesc->bObsolete = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "AUX", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(
                    &pContentDesc->ppszAuxOCs,
                    &pRest,
                    &pContentDesc->usNumAuxOCs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "MUST", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(
                    &pContentDesc->ppszMustATs,
                    &pRest,
                    &pContentDesc->usNumMustATs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "MAY", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(
                    &pContentDesc->ppszMayATs,
                    &pRest,
                    &pContentDesc->usNumMayATs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "NOT", FALSE) == 0)
        {   //TODO, semantic not supported yet.
            dwError = schemaParseParenthesisItems(
                    &pContentDesc->ppszNotATs,
                    &pRest,
                    &pContentDesc->usNumNotATs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (IS_R_PARENTHESIS_STR(pToken))
        {
            ;
        }
        else
        {
            dwError = ERROR_INVALID_DITCONTENTRULES;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pBuf);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VdirSchemaParseStrToContentDesc:(%s)(%s)",
              VDIR_SAFE_STRING(pToken), VDIR_SAFE_STRING(pStr) );

    VmDirSchemaContentDescContentFree(pContentDesc);

    goto cleanup;
}

DWORD
VmDirSchemaParseStrToStructureDesc(
    const char*            pStr,
    PVDIR_SCHEMA_SR_DESC   pStructureDesc
    )
{
    DWORD dwError = 0;
    BOOLEAN bHasRuleID = FALSE;
    PSTR  pToken = NULL;
    PSTR  pRest = NULL;
    char*  pBuf = NULL;

    if ( !pStr || !pStructureDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pStr, &pBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmdDirSchemaParseNormalizeElement( pBuf );

    pToken = VmDirStringTokA(pBuf, SCHEMA_GEN_TOKEN_SEP, &pRest);
    if (!pToken)
    {
        dwError = ERROR_INVALID_DITSTRUCTURERULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pToken, "ditstructurerules:", FALSE) == 0)
    {    // ignore leading ditstructurerules:
        GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
    }

    if (! IS_L_PARENTHESIS_STR(pToken))
    {
        dwError = ERROR_INVALID_DITCONTENTRULES;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest);
        pToken;
        pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest))
    {


        if (!bHasRuleID)
        {
            bHasRuleID = TRUE;
            dwError = VmDirAllocateStringA(pToken, &pStructureDesc->pszRuleID);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "NAME", FALSE) == 0)
        {
            //TODO, handle ( ) case?
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pStructureDesc->pszName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "DESC", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_DESC_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pStructureDesc->pszDesc);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "OBSOLETE", FALSE) == 0)
        {   //TODO, semantic not supported yet.
            pStructureDesc->bObsolete = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "FORM", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pStructureDesc->pszNameform);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "SUP", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(
                    &pStructureDesc->ppszSupRulesID,
                    &pRest,
                    &pStructureDesc->usNumSupRulesID);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (IS_R_PARENTHESIS_STR(pToken))
        {
            ;
        }
        else
        {
            dwError = ERROR_INVALID_DITSTRUCTURERULES;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pBuf);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VdirSchemaParseStrToStructureDesc:(%s)(%s)",
              VDIR_SAFE_STRING(pToken), VDIR_SAFE_STRING(pStr) );

    VmDirSchemaStructureDescContentFree(pStructureDesc);

    goto cleanup;
}

/*
 * convert nameform string into VDIR_SCHEMA_NAMEFORM_DESC
 NameFormDescription = "(" whsp
          numericoid whsp  ; NameForm identifier
          [ "NAME" qdescrs ]
          [ "DESC" qdstring ]
          [ "OBSOLETE" whsp ]
          "OC" woid         ; Structural ObjectClass
          "MUST" oids       ; AttributeTypes
          [ "MAY" oids ]    ; AttributeTypes
          whsp ")"
 */
DWORD
VmDirSchemaParseStrToNameformDesc(
    const char*                 pStr,
    PVDIR_SCHEMA_NF_DESC  pNameformDesc
    )
{
    DWORD       dwError = 0;
    BOOLEAN     bHasOID = FALSE;
    PSTR        pToken = NULL;
    PSTR        pRest = NULL;
    char*       pBuf = NULL;

    if ( !pStr || !pNameformDesc )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pStr, &pBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    VmdDirSchemaParseNormalizeElement( pBuf );

    pToken = VmDirStringTokA(pBuf, SCHEMA_GEN_TOKEN_SEP, &pRest);
    if (!pToken)
    {
        dwError = ERROR_INVALID_NAMEFORMS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pToken, NAMEFORM_TAG, FALSE) == 0)
    {
        GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
    }

    if (! IS_L_PARENTHESIS_STR(pToken))
    {
        dwError = ERROR_INVALID_NAMEFORMS;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for ( pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest);
          pToken;
          pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, &pRest)
        )
    {

        VDIR_BERVALUE bv = VDIR_BERVALUE_INIT;
        bv.lberbv.bv_val = pToken;
        bv.lberbv.bv_len = VmDirStringLenA(pToken);

        if (!bHasOID && syntaxOID(&bv))
        {
            bHasOID = TRUE;
            dwError = VmDirAllocateStringA(pToken, &pNameformDesc->pszOid);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "NAME", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pNameformDesc->pszName);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "DESC", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_DESC_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pNameformDesc->pszDesc);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "OBSOLETE", FALSE) == 0)
        {   //TODO, semantic not supported yet.
            pNameformDesc->bObsolete = TRUE;
        }
        else if (VmDirStringCompareA(pToken, "OC", FALSE) == 0)
        {
            GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, pRest);
            dwError = VmDirAllocateStringA(pToken, &pNameformDesc->pszStructOC);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "MUST", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(  &pNameformDesc->ppszMustATs,
                                                    &pRest,
                                                    &pNameformDesc->usNumMustATs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pToken, "MAY", FALSE) == 0)
        {
            dwError = schemaParseParenthesisItems(  &pNameformDesc->ppszMayATs,
                                                    &pRest,
                                                    &pNameformDesc->usNumMayATs);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (IS_R_PARENTHESIS_STR(pToken))
        {
            ;
        }
        else
        {
            dwError = ERROR_INVALID_NAMEFORMS;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }


cleanup:

    VMDIR_SAFE_FREE_MEMORY(pBuf);

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirSchemaParseStrToNameformDesc:(%s)(%s) failed",
              VDIR_SAFE_STRING(pToken), VDIR_SAFE_STRING(pStr) );

    VmDirSchemaNameformDescContentFree(pNameformDesc);

    goto cleanup;
}

static
DWORD
schemaParseParenthesisItems(
    PSTR**       pppszList,
    PSTR*        ppRest,
    PUSHORT      pusSize
    )
{
    DWORD       dwError = 0;
    USHORT      usCnt = 0;
    size_t      iSize = 5;
    PSTR        pToken = NULL;
    PSTR*       ppszLocalList = NULL;

    dwError = VmDirAllocateMemory(sizeof(PSTR) * (iSize+1),
            (PVOID*) &ppszLocalList);
    BAIL_ON_VMDIR_ERROR(dwError);

    GET_NEXT_TOKEN(pToken, SCHEMA_GEN_TOKEN_SEP, *ppRest);

    if (IS_EMPTY_PARENTHESIS_STR(pToken))
    {   //TODO, is () allowed?
        dwError = ERROR_INVALID_SCHEMA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (! IS_L_PARENTHESIS(pToken))
    {
        dwError = VmDirAllocateStringA( pToken,
                                        &(ppszLocalList[0]) );
        BAIL_ON_VMDIR_ERROR(dwError);

        usCnt++;
    }
    else
    {
        usCnt = 0;

        if (! IS_L_PARENTHESIS_STR(pToken))
        {
            dwError = VmDirAllocateStringA( &(pToken[1]),
                                            &(ppszLocalList[usCnt++]) );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        for (pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, ppRest);
             pToken && !IS_R_PARENTHESIS_STR(pToken);
             pToken=VmDirStringTokA(NULL, SCHEMA_GEN_TOKEN_SEP, ppRest))
        {
            if (usCnt == iSize)
            {
                size_t iOldSize = iSize;
                iSize = iSize * 2;
                dwError = VmDirReallocateMemoryWithInit(    ppszLocalList,
                                                            (PVOID*) &ppszLocalList,
                                                            (iSize+1) * sizeof(PSTR),
                                                            (iOldSize) * sizeof(PSTR));
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            dwError = VmDirAllocateStringA( pToken,
                                            &(ppszLocalList)[usCnt++]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (!pToken)
        {
            dwError = ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    if ( usCnt == 0 && ppszLocalList )
    {
        VMDIR_SAFE_FREE_MEMORY( ppszLocalList );
    }

    *pppszList = ppszLocalList;
    *pusSize = usCnt;

cleanup:

    return dwError;

error:

    VmDirFreeStringArrayA( ppszLocalList);
    VMDIR_SAFE_FREE_MEMORY( ppszLocalList );

    goto cleanup;
}
