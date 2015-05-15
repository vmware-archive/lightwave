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
 * Filename: adcompatibleschema.c
 *
 * Abstract:
 *
 * Simulate ADSI AD schema object search.
 *
 * AD tools like ADSI query schema information via these queries.
 *
 */

#include "includes.h"

#define ADSI_ATTR_CN                        "cn"
#define ADSI_ATTR_RDNATTRID                 "rDNAttid"
#define ADSI_ATTR_POSSIBLEINFERIORS         "possibleInferiors"
#define ADSI_ATTR_POSSSUPERIORS             "possSuperiors"
#define ADSI_ATTR_ISSINGLEVALUED            "isSingleValued"
#define ADSI_ATTR_ATTRIBUTESYNTAX           "attributeSyntax"
#define ADSI_ATTR_LDAPDISPLAYNAME           "ldapDisplayName"
#define ADSI_ATTR_SYSTEMONLY                "systemOnly"

#define AD_SYNTAX_UNDEFINED                 "2.5.5.0"

///////////////////////////////////////////////////////////////////////////////////////////
// NOTE, this table does NOT match very well between AD and Lotus syntax
// 1. there is no one to one mapping
// 2. RCF defined syntax space is larger than AD defined syntax space
// 3. AD syntax mix both syntax and matching rule concept together
//    (e.g. CaseIgnoreString(Teletex) "2.5.5.4" means Teletex syntax with CaseIgnoreMatch)
///////////////////////////////////////////////////////////////////////////////////////////
// Also, NOT all RFC defined syntaxes are supported now.  TODO
///////////////////////////////////////////////////////////////////////////////////////////
#define LOTUS_TO_AD_SYNTAX_INITIALIZER  \
{                                                                                   \
    "1.3.6.1.1.16.1",                   "2.5.5.0", /* UUID */                       \
    "1.3.6.1.4.1.1466.115.121.1.12",    "2.5.5.1", /* DN */                         \
    "1.3.6.1.4.1.1466.115.121.1.15",    "2.5.5.12", /* UNICODE STRING */            \
    "1.3.6.1.4.1.1466.115.121.1.24",    "2.5.5.11", /* GENERALIZED TIME */          \
    "1.3.6.1.4.1.1466.115.121.1.26",    "2.5.5.5", /* IA5 CASE */                   \
    "1.3.6.1.4.1.1466.115.121.1.27",    "2.5.5.9", /* INTEGER */                    \
    "1.3.6.1.4.1.1466.115.121.1.36",    "2.5.5.6", /* NUMERIC STRING */             \
    "1.3.6.1.4.1.1466.115.121.1.38",    "2.5.5.2", /* OID */                        \
    "1.3.6.1.4.1.1466.115.121.1.40",    "2.5.5.10", /* OCT STRING */                \
    "1.3.6.1.4.1.1466.115.121.1.43",    "2.5.5.13", /* PRESENTAATION ADDRESS */     \
    "1.3.6.1.4.1.1466.115.121.1.44",    "2.5.5.5", /* PRINTABLE STRING */           \
    "1.3.6.1.4.1.1466.115.121.1.5",     "2.5.5.10", /* BINARY */                    \
    "1.3.6.1.4.1.1466.115.121.1.53",    "2.5.5.11", /* UTC TIME */                  \
    "1.3.6.1.4.1.1466.115.121.1.7",     "2.5.5.8", /* BOOLEAN */                    \
}

static
DWORD
_VmDirLotusToADSyntax(
    PCSTR   pszLotusSynatx,
    PCSTR*  ppszADSyntax
    );

static
DWORD
_VmDirOCDescToADClassSchemaAttrList(
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PSTR**                  pppszAttrList
    );

static
DWORD
_VmDirATDescToADAttributeSchemaAttrList(
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PSTR**                  pppszAttrList
    );

static
DWORD
_VmDirSchemaADClassSchemaSetup(
    PVDIR_SCHEMA_CTX   pCtx
    );

static
DWORD
_VmDirSchemaADAttributeSchemaSetup(
    PVDIR_SCHEMA_CTX   pCtx
    );

/*
 * Called during schema instance creation.
 * For each objectclass and attributetype, create a corresponding entry
 * to represent its AD schemaobject counterpart.
 */
DWORD
VdirSchemaADCompatibleSetup(
    PVDIR_SCHEMA_INSTANCE   pSchema
    )
{
    DWORD               dwError = 0;
    PVDIR_SCHEMA_CTX    pCtx = NULL;

    if ( !pSchema )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory( sizeof(*pCtx), (PVOID)&pCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    // this schema instance is not yet live and not accessible by any others.
    // so it is safe to access it w/o reference count mutex protection.
    pCtx->pSchema = pSchema;

    // setup classSchema entry
    dwError = _VmDirSchemaADClassSchemaSetup(pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // setup attributeSchema entry
    dwError = _VmDirSchemaADAttributeSchemaSetup(pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pCtx);

    return dwError;

error:

    goto cleanup;
}

/*
 * Trigger by following ADSI search :
 * -b "cn=schemacontext" -s one "(&(objectclass=attributeschema)(!(isdefunct=TRUE))"
 *
 * NOTE, we do NOT support paging control here.
 */
DWORD
VmDirADCompatibleSendAllAttributeSchema(
    PVDIR_OPERATION     pOp
    )
{
    DWORD                       dwError = 0;
    unsigned short              usCnt = 0;
    PVDIR_SCHEMA_AT_COLLECTION  pATCollection = NULL;

    if ( !pOp || !pOp->pSchemaCtx)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pATCollection = &(pOp->pSchemaCtx->pSchema->ats);

    for ( usCnt=0 ; usCnt < pATCollection->usNumATs; usCnt++ )
    {
        if ( pATCollection->pATSortName[usCnt].pADAttributeSchemaEntry )
        {
            dwError = VmDirSendSearchEntry( pOp,
                                            pATCollection->pATSortName[usCnt].pADAttributeSchemaEntry );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:

    return dwError;

error:
    VmDirLog( LDAP_DEBUG_ANY, "VmDirADCompatibleSendAllAttributeSchema: (%d)(%s)",
                              dwError, VDIR_SAFE_STRING(pOp->ldapResult.pszErrMsg) );

    if (pOp->ldapResult.errCode == 0)
    {
        pOp->ldapResult.errCode = dwError;
    }

    goto cleanup;
}

/*
 * Trigger by following ADSI search :
 * -b "cn=schemacontext" -s one "(&(objectclass=attributeschema)(lDAPDisplayName=XXX)"
 */
DWORD
VmDirADCompatibleSearchAttributeSchema(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PCSTR               pszName,
    PVDIR_ENTRY*        ppEntry             // Caller does NOT own *ppEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;

    if ( !pSchemaCtx || !ppEntry )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaAttrNameToDescriptor(  pSchemaCtx,
                                                pszName,
                                                &pATDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pATDesc->pADAttributeSchemaEntry;

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * Trigger by following ADSI search :
 * -b "cn=schemacontext" -s one "(&(lDAPDisplayName=VALUE_OF_OBJECTCLASS)(!(isdefunct=TRUE))"
 */
DWORD
VmDirADCompatibleSearchClassSchema(
    PVDIR_SCHEMA_CTX    pSchemaCtx,
    PCSTR               pszName,
    PVDIR_ENTRY*        ppEntry             // Caller does NOT own *ppEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_OC_DESC    pOCDesc = NULL;

    if ( !pSchemaCtx || !ppEntry )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaOCNameToDescriptor(    pSchemaCtx,
                                                pszName,
                                                &pOCDesc);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pOCDesc->pADClassSchemaEntry;

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * this pCtx->schema instance is not yet live and not accessible by any others.
 * so it is safe to access it w/o reference count mutex protection.
 */
static
DWORD
_VmDirSchemaADClassSchemaSetup(
    PVDIR_SCHEMA_CTX   pCtx
    )
{
    DWORD               dwError = 0;
    USHORT              usCnt = 0;
    PVDIR_ENTRY         pEntry = NULL;
    PSTR                pszDN = NULL;
    PSTR*               ppszAttrList = NULL;

    for (usCnt = 0; usCnt < pCtx->pSchema->ocs.usNumOCs; usCnt++)
    {
        PVDIR_SCHEMA_OC_DESC    pOCDesc = pCtx->pSchema->ocs.pOCSortName + usCnt;

        VMDIR_SAFE_FREE_MEMORY(pszDN);
        if (ppszAttrList)
        {
            VmDirFreeStringArrayA(ppszAttrList);
            VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
        }

        dwError = VmDirAllocateStringAVsnprintf(    &pszDN,
                                                    "cn=%s,%s",
                                                    pOCDesc->pszName,
                                                    SCHEMA_NAMING_CONTEXT_DN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirOCDescToADClassSchemaAttrList(pOCDesc, &ppszAttrList);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAttrListToNewEntry(  pCtx,   // inside, pCtx will be cloned
                                            pszDN,
                                            ppszAttrList,
                                            &pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCtx->pSchema->usNumSelfRef++;  // pEntry->pSchemaCtx self reference

        // OCDesc takes over pEntry
        pOCDesc->pADClassSchemaEntry = pEntry;
        pEntry = NULL;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDN);

    if (ppszAttrList)
    {
        VmDirFreeStringArrayA(ppszAttrList);
        VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
    }

    return dwError;

error:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}


/*
 * this pCtx->schema instance is not yet live and not accessible by any others.
 * so it is safe to access it w/o reference count mutex protection.
 */
static
DWORD
_VmDirSchemaADAttributeSchemaSetup(
    PVDIR_SCHEMA_CTX   pCtx
    )
{
    DWORD               dwError = 0;
    USHORT              usCnt = 0;
    PVDIR_ENTRY         pEntry = NULL;
    PSTR                pszDN = NULL;
    PSTR*               ppszAttrList = NULL;

    for (usCnt = 0; usCnt < pCtx->pSchema->ats.usNumATs; usCnt++)
    {
        PVDIR_SCHEMA_AT_DESC    pATDesc = pCtx->pSchema->ats.pATSortName + usCnt;

        VMDIR_SAFE_FREE_MEMORY(pszDN);
        if (ppszAttrList)
        {
            VmDirFreeStringArrayA(ppszAttrList);
            VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
        }

        dwError = VmDirAllocateStringAVsnprintf(    &pszDN,
                                                    "cn=%s,%s",
                                                    pATDesc->pszName,
                                                    SCHEMA_NAMING_CONTEXT_DN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirATDescToADAttributeSchemaAttrList(pATDesc, &ppszAttrList);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAttrListToNewEntry(  pCtx,   // inside, pCtx will be cloned
                                            pszDN,
                                            ppszAttrList,
                                            &pEntry);
        BAIL_ON_VMDIR_ERROR(dwError);

        pCtx->pSchema->usNumSelfRef++;  // pEntry->pSchemaCtx self reference

        // ATDesc takes over pEntry
        pATDesc->pADAttributeSchemaEntry = pEntry;
        pEntry = NULL;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDN);

    if (ppszAttrList)
    {
        VmDirFreeStringArrayA(ppszAttrList);
        VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
    }

    return dwError;

error:

    if (pEntry)
    {
        VmDirFreeEntry(pEntry);
    }

    goto cleanup;
}

/*
 * caller owns *ppszAttrList
 */
static
DWORD
_VmDirATDescToADAttributeSchemaAttrList(
    PVDIR_SCHEMA_AT_DESC    pATDesc,
    PSTR**                  pppszAttrList
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszAttrList = NULL;
    PCSTR   pszADSyntax = NULL;
    PSTR    pszSystemOnly = "FALSE";
    int     iCnt = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // cn                       - pATDesc->pszName
    // -------------------------- ADSI asks following attributes
    // isSingleValued           - pATDesc->bSingleValue
    // attributeSyntax          - pATDesc->pszSyntaxName + table lookup
    // systemFlags              - ignore
    // linkId                   - ignore
    // systenOnly               - pATDesc->usage+bNoUserModifiable
    // ldapDisplayName          - pATDesc->pszName
    ////////////////////////////////////////////////////////////////////////////////
    //                              5 attribute+value and 1 NULL terminator
    dwError = VmDirAllocateMemory(  sizeof(PSTR) * (5 * 2 + 1), (PVOID)&ppszAttrList );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ADSI_ATTR_CN, &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( pATDesc->pszName, &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ADSI_ATTR_ISSINGLEVALUED, &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( pATDesc->bSingleValue ? "TRUE" : "FALSE" , &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ADSI_ATTR_ATTRIBUTESYNTAX , &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirLotusToADSyntax( pATDesc->pszSyntaxName, &pszADSyntax );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( VDIR_SAFE_STRING(pszADSyntax) , &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ADSI_ATTR_LDAPDISPLAYNAME, &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( pATDesc->pszName, &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ADSI_ATTR_SYSTEMONLY, &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pATDesc->bNoUserModifiable || pATDesc->usage != VDIR_ATTRIBUTETYPE_USER_APPLICATIONS )
    {
        pszSystemOnly = "TRUE";
    }
    dwError = VmDirAllocateStringA( pszSystemOnly, &ppszAttrList[iCnt++] );
    BAIL_ON_VMDIR_ERROR(dwError);

    *pppszAttrList = ppszAttrList;

cleanup:

    return dwError;

error:

    if (ppszAttrList)
    {
        VmDirFreeStringArrayA(ppszAttrList);
        VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
    }

    goto cleanup;
}

/*
 * caller owns *ppszAttrList
 */
static
DWORD
_VmDirOCDescToADClassSchemaAttrList(
    PVDIR_SCHEMA_OC_DESC    pOCDesc,
    PSTR**                  pppszAttrList
    )
{
    DWORD   dwError = 0;
    PSTR*   ppszAttrList = NULL;
    int     iCnt = 0;
    int     iNumParent = 0;
    int     iNumChild = 0;
    int     iTmp = 0;

    // ADSI asks following attributes
    // cn                       - cn value
    // displaySpecification     - ignore
    // schemaIDGUID             - ignore
    // possibleInferiors        - construct from pOCDesc->ppszAllowedChildOCs
    // rDNAttid                 - construct from pOCDesc->ppszMustRDNs
    // possSuperiors            - construct from pOCDesc->ppszAllowedParentOCs
    // systemPossSuperiors      - ignore

    for (iNumParent = 0;
         pOCDesc->ppszAllowedParentOCs && pOCDesc->ppszAllowedParentOCs[iNumParent];
         iNumParent++)
    {}

    for (iNumChild = 0;
         pOCDesc->ppszAllowedChildOCs && pOCDesc->ppszAllowedChildOCs[iNumChild];
         iNumChild++)
    {}

    // size = total attribute value * 2  + 1 (NULL terminate)
    dwError = VmDirAllocateMemory(  sizeof(PSTR) * ((1+1+iNumParent+iNumChild) * 2 + 1),
                                    (PVOID)&ppszAttrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ADSI_ATTR_CN, &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( pOCDesc->pszName, &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA( ADSI_ATTR_RDNATTRID, &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    // use the first Must RDN value ( AD mode should have only one value )
    // otherwise, default to "cn"
    dwError = VmDirAllocateStringA( pOCDesc->usNumMustRDNs == 1 ?
                                        VDIR_SAFE_STRING( pOCDesc->ppszMustRDNs[0] ) : ADSI_ATTR_CN,
                                    &ppszAttrList[iCnt++]);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iTmp = 0; iTmp < iNumParent; iTmp++)
    {
        dwError = VmDirAllocateStringA( ADSI_ATTR_POSSSUPERIORS, &ppszAttrList[iCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirAllocateStringA(pOCDesc->ppszAllowedParentOCs[iTmp], &ppszAttrList[iCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (iTmp = 0; iTmp < iNumChild; iTmp++)
    {
        dwError = VmDirAllocateStringA( ADSI_ATTR_POSSIBLEINFERIORS, &ppszAttrList[iCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA( pOCDesc->ppszAllowedChildOCs[iTmp], &ppszAttrList[iCnt++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pppszAttrList = ppszAttrList;

cleanup:

    return dwError;

error:

    if (ppszAttrList)
    {
        VmDirFreeStringArrayA(ppszAttrList);
        VMDIR_SAFE_FREE_MEMORY(ppszAttrList);
    }

    goto cleanup;
}

/*
 * Map Lotus syntax to AD syntax
 */
static
DWORD
_VmDirLotusToADSyntax(
    PCSTR   pszLotusSyntax,
    PCSTR*  ppszADSyntax            // caller does NOT own *ppszADSyntax
    )
{
    DWORD   dwError = 0;
    PCSTR   pszLocalSyntax = NULL;
    PCSTR   pszLotusToADSyntaxTbl[] = LOTUS_TO_AD_SYNTAX_INITIALIZER;
    int     iSize = sizeof(pszLotusToADSyntaxTbl)/sizeof(pszLotusToADSyntaxTbl[0]);
    int     iCnt = 0;

    for (iCnt = 0; iCnt < iSize; iCnt += 2)
    {
        if ( VmDirStringCompareA( pszLotusSyntax, pszLotusToADSyntaxTbl[iCnt], FALSE ) == 0 )
        {
            pszLocalSyntax = pszLotusToADSyntaxTbl[iCnt+1];
            break;
        }
    }

    if ( pszLocalSyntax )
    {
        *ppszADSyntax = pszLocalSyntax;
    }
    else
    {
        // do not retrun ERROR_NO_SUCH_SYNTAX error
        // instead, return ADSI AD_SYNTAX_UNDEFINED
        *ppszADSyntax = AD_SYNTAX_UNDEFINED;
    }

    return dwError;

}
