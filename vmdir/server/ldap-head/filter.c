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
#include "ldap_pvt.h"

#define START_ANY_ALLOC_SIZE    5

static int
ParseAva(
    VDIR_OPERATION *   op,
    VDIR_FILTER *      f,
    VDIR_LDAP_RESULT *       lr );

static int
ParseComplexFilterComponents(
    VDIR_OPERATION *   op,
    VDIR_FILTER **     f,
    VDIR_LDAP_RESULT *       lr );

static int
ParseSubStrings(
    VDIR_OPERATION *  op,
    VDIR_FILTER *     f,
    VDIR_LDAP_RESULT *      lr );

static size_t
RequiredSizeForStrFilter(
    VDIR_FILTER * f);

static VDIR_FILTER_COMPUTE_RESULT
TestAvaFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f);

static VDIR_FILTER_COMPUTE_RESULT
TestOneLevelFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f);

static VDIR_FILTER_COMPUTE_RESULT
TestPresentFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f);

static VDIR_FILTER_COMPUTE_RESULT
TestSubstringsFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f);

static VDIR_FILTER_COMPUTE_RESULT
TestSubstringAny(
    VDIR_BERVALUE *any_array,
    int any_size,
    char *attr_val,
    ber_len_t attr_len);

DWORD
VmDirConcatTwoFilters(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PSTR pszAttrFilterName1,
    PSTR pszAttrFilterVal1,
    PSTR pszAttrFilterName2,
    PSTR pszAttrFilterVal2,
    PVDIR_FILTER* ppFilter
    )
{
    DWORD dwError = ERROR_SUCCESS;
    PVDIR_FILTER pFilter1 = NULL;
    PVDIR_FILTER pFilter2 = NULL;
    PVDIR_FILTER pAnd = NULL;
    PVDIR_SCHEMA_CTX pSchemaCtxCurr = NULL;

    if ( !pszAttrFilterName1 || !pszAttrFilterVal1 || !pszAttrFilterName2 || !pszAttrFilterVal2 )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pSchemaCtx)
    {
        dwError = VmDirSchemaCtxAcquire(&pSchemaCtxCurr);
        BAIL_ON_VMDIR_ERROR(dwError);

        pSchemaCtx = pSchemaCtxCurr;
    }

    dwError = VmDirAllocateMemory(sizeof( VDIR_FILTER ), (PVOID *)&pAnd);
    BAIL_ON_VMDIR_ERROR(dwError);
    pAnd->choice = LDAP_FILTER_AND;

    dwError = VmDirAllocateMemory(sizeof( VDIR_FILTER ), (PVOID *)&pFilter1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof( VDIR_FILTER ), (PVOID *)&pFilter2);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Filter1
    pFilter1->choice = LDAP_FILTER_EQUALITY;
    pFilter1->filtComp.ava.type.lberbv.bv_val = pszAttrFilterName1;
    pFilter1->filtComp.ava.type.lberbv.bv_len = VmDirStringLenA(pszAttrFilterName1);

    if ((pFilter1->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc(
                                                     pSchemaCtx,
                                                     pFilter1->filtComp.ava.type.lberbv.bv_val)) == NULL)
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pFilter1->filtComp.ava.value.lberbv.bv_val = pszAttrFilterVal1;
    pFilter1->filtComp.ava.value.lberbv.bv_len = VmDirStringLenA(pszAttrFilterVal1);

    dwError = VmDirSchemaBervalNormalize(pSchemaCtx, pFilter1->filtComp.ava.pATDesc,
                                         &(pFilter1->filtComp.ava.value));
    BAIL_ON_VMDIR_ERROR(dwError);

    // Filter2
    pFilter2->choice = LDAP_FILTER_EQUALITY;
    pFilter2->filtComp.ava.type.lberbv.bv_val = pszAttrFilterName2;
    pFilter2->filtComp.ava.type.lberbv.bv_len = VmDirStringLenA(pszAttrFilterName2);

    if ((pFilter2->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc(
                                                     pSchemaCtx,
                                                     pFilter2->filtComp.ava.type.lberbv.bv_val)) == NULL)
    {
        dwError = ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pFilter2->filtComp.ava.value.lberbv.bv_val = pszAttrFilterVal2;
    pFilter2->filtComp.ava.value.lberbv.bv_len = VmDirStringLenA(pszAttrFilterVal2);

    dwError = VmDirSchemaBervalNormalize(pSchemaCtx, pFilter2->filtComp.ava.pATDesc,
                                         &(pFilter2->filtComp.ava.value));
    BAIL_ON_VMDIR_ERROR(dwError);

    pFilter1->next = pFilter2;
    pAnd->filtComp.complex = pFilter1;
    pAnd->next = NULL;

    *ppFilter = pAnd;

cleanup:
    if (pSchemaCtx == pSchemaCtxCurr && pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY( pAnd );
    VMDIR_SAFE_FREE_MEMORY( pFilter1 );
    VMDIR_SAFE_FREE_MEMORY( pFilter2 );

    goto cleanup;
}

int
AppendDNFilter(
    VDIR_OPERATION *     op)
{
    int        retVal = LDAP_SUCCESS;
    VDIR_FILTER *   f = NULL;
    VDIR_FILTER *   dnFilter = NULL;
    VDIR_BERVALUE   normDn = VDIR_BERVALUE_INIT;
    BOOLEAN         bPrePendDNFilter = TRUE;

    VmDirLog( LDAP_DEBUG_TRACE, "AppendDNFilter: Begin" );

    if (VmDirAllocateMemory( sizeof( VDIR_FILTER ), (PVOID *)&f ) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    f->choice = LDAP_FILTER_AND;
    if (VmDirAllocateMemory( sizeof( VDIR_FILTER ), (PVOID *)&dnFilter ) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    normDn.lberbv.bv_val = op->reqDn.lberbv.bv_val;
    normDn.lberbv.bv_len = op->reqDn.lberbv.bv_len;
    // Normalize DN value is owned by filter component, un-normalized value is owned by the caller.
    if ((retVal = VmDirNormalizeDN( &normDn, op->pSchemaCtx )) != 0)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "AppendDNFilter: DN normalization failed - code(%d)", retVal);
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    switch (op->request.searchReq.scope)
    {
        case LDAP_SCOPE_BASE:
            bPrePendDNFilter = TRUE;
            dnFilter->choice = LDAP_FILTER_EQUALITY;
            dnFilter->filtComp.ava.type.lberbv.bv_val = ATTR_DN;
            dnFilter->filtComp.ava.type.lberbv.bv_len = ATTR_DN_LEN;
            dnFilter->filtComp.ava.value = normDn;
            if ((dnFilter->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc( op->pSchemaCtx, ATTR_DN )) == NULL)
            {
                VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "AppendDNFilter: VmDirSchemaAttrNameToDesc failed for ATTR_DN");
                retVal = LDAP_OPERATIONS_ERROR;
                BAIL_ON_VMDIR_ERROR( retVal );
            }
            break;

        case LDAP_SCOPE_ONELEVEL:
            bPrePendDNFilter = FALSE;
            dnFilter->choice = FILTER_ONE_LEVEL_SEARCH;
            dnFilter->filtComp.parentDn = normDn;
            break;

        case LDAP_SCOPE_SUBTREE:
           bPrePendDNFilter = FALSE;
           dnFilter->choice = LDAP_FILTER_SUBSTRINGS;
           dnFilter->filtComp.subStrings.type.lberbv.bv_val = ATTR_DN;
           dnFilter->filtComp.subStrings.type.lberbv.bv_len = ATTR_DN_LEN;
           dnFilter->filtComp.subStrings.final = normDn;
           if (dnFilter->filtComp.subStrings.final.bvnorm_len == 0)
           {
               dnFilter->computeResult = FILTER_RES_TRUE;
           }
           if ((dnFilter->filtComp.subStrings.pATDesc = VmDirSchemaAttrNameToDesc( op->pSchemaCtx, ATTR_DN )) == NULL)
           {
               VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "AppendDNFilter: VmDirSchemaAttrNameToDesc failed for ATTR_DN");
               retVal = LDAP_OPERATIONS_ERROR;
               BAIL_ON_VMDIR_ERROR( retVal );
           }
           break;

        default:
            retVal = LDAP_UNWILLING_TO_PERFORM;
            BAIL_ON_VMDIR_ERROR( retVal );
    }

    if (bPrePendDNFilter)
    {   // generate "(&(DN/PDN=xxx)(YOUR ORIGINAL FILTER))" if DN is BASE/EQUALITY
        dnFilter->next = op->request.searchReq.filter;
        f->filtComp.complex = dnFilter;
    }
    else
    {   // generate "(&(YOUR ORIGINAL_FILTER)(DN=xxx*))" if DN is SUBTREE OR ONE_LEVEL
        op->request.searchReq.filter->next = dnFilter;
        f->filtComp.complex = op->request.searchReq.filter;
    }

    f->next = NULL;

    // op takes over f, dnFilter as well as normDn.lberbv.normbv_val
    op->request.searchReq.filter = f;
    retVal = LDAP_SUCCESS;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "AppendDNFilter: End" );
    return retVal;

error:
    VmDirFreeBervalContent( &normDn );  // TODO, memory ownership is hard to understand/track.
                                        // with BerValue bv_val/bvnorm_val, and BerValue assignment
                                        // such as dnFilter->filtComp.parentDn = normDn.  It becomes
                                        // rather complicate to follow .....
    VMDIR_SAFE_FREE_MEMORY( f );
    VMDIR_SAFE_FREE_MEMORY( dnFilter );
    goto cleanup;
}

/*
 * From RFC 4511, section 4.5.1.7:
 *     A filter of the "and" choice is TRUE if all the filters in the SET OF evaluate to TRUE, FALSE if at least one
 *     filter is FALSE, and Undefined otherwise.
 *
 *     A filter of the "or" choice is FALSE if all the filters in the SET OF
 *     evaluate to FALSE, TRUE if at least one filter is TRUE, and Undefined otherwise.
 *
 *     A filter of the �not� choice is
 *     TRUE if the filter being negated is FALSE, FALSE if it is TRUE, and Undefined if it is Undefined.
 */

VDIR_FILTER_COMPUTE_RESULT
CheckIfEntryPassesFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f)
{
    VDIR_FILTER_COMPUTE_RESULT retVal = FILTER_RES_FALSE;

    VmDirLog( LDAP_DEBUG_TRACE, "CheckIfEntryPassesFilter: Begin, filter choice = %d", f->choice );

    /*
    // If filter's compute result is already known ...
    if (f->computeResult == FILTER_RES_TRUE || f->computeResult == FILTER_RES_FALSE || f->computeResult == FILTER_RES_UNDEFINED)
    {
        return f->computeResult;
    } */

    switch (f->choice)
    {
        case LDAP_FILTER_AND:
        case LDAP_FILTER_OR:
        {
            VDIR_FILTER * curr = NULL;
            BOOLEAN foundAnUndefined = FALSE;

            for ( curr = f->filtComp.complex; curr != NULL; curr = curr->next )
            {
                retVal = CheckIfEntryPassesFilter( op, e, curr );
                if ((f->choice == LDAP_FILTER_AND && retVal == FILTER_RES_FALSE) ||
                    (f->choice == LDAP_FILTER_OR && retVal == FILTER_RES_TRUE))
                {
                    goto done;
                }
                if (retVal == FILTER_RES_UNDEFINED)
                {
                    foundAnUndefined = TRUE;
                }
            }
            // Determine if filter is TRUE (for AND case)/FALSE (for OR case) or UNDEFINED
            if (foundAnUndefined)
            {
                retVal = FILTER_RES_UNDEFINED;
            }
            break;
        }
        case LDAP_FILTER_NOT:
            retVal = CheckIfEntryPassesFilter( op, e, f->filtComp.complex );
            switch (retVal)
            {
                case FILTER_RES_TRUE:
                    retVal = FILTER_RES_FALSE;
                    break;
                case FILTER_RES_FALSE:
                    retVal = FILTER_RES_TRUE;
                    break;
                default:
                    retVal = FILTER_RES_UNDEFINED;
                    break;
            }
            break;
        case LDAP_FILTER_EQUALITY:
            retVal = TestAvaFilter( op, e, f );
            break;
        case LDAP_FILTER_SUBSTRINGS:
            retVal = TestSubstringsFilter( op, e, f );
            break;
        case LDAP_FILTER_GE:
        case LDAP_FILTER_LE:
            retVal = TestAvaFilter( op, e, f );
            break;
        case LDAP_FILTER_PRESENT:
            retVal = TestPresentFilter( op, e, f );
            break;
        case FILTER_ONE_LEVEL_SEARCH:
            retVal = TestOneLevelFilter( op, e, f );
            break;
        default:
            break;
    }

done:
    VmDirLog( LDAP_DEBUG_TRACE, "CheckIfEntryPassesFilter: End, retVal = %s",
              retVal == FILTER_RES_TRUE ? "TRUE" : (retVal == FILTER_RES_FALSE ? "FALSE" : "UNDEFIEND") );
    return retVal;
}

void
DeleteFilter(
    VDIR_FILTER *     f)
{
    VmDirLog( LDAP_DEBUG_TRACE, "DeleteFilter: Begin" );

    if ( f == NULL )
    {
        return;
    }

    switch ( f->choice )
    {
        case LDAP_FILTER_AND:
        case LDAP_FILTER_OR:
        {
            VDIR_FILTER * curr = NULL;
            VDIR_FILTER * next = NULL;

            for ( curr = f->filtComp.complex; curr != NULL; curr = next )
            {
                next = curr->next;
                DeleteFilter( curr );
            }
            break;
        }

        case LDAP_FILTER_NOT:
            DeleteFilter( f->filtComp.complex );
            break;

        case LDAP_FILTER_EQUALITY:
        case LDAP_FILTER_GE:
        case LDAP_FILTER_LE:
            VmDirFreeBervalContent( &(f->filtComp.ava.value) );
            break;

        case LDAP_FILTER_SUBSTRINGS:
            VmDirFreeBervalContent( &(f->filtComp.subStrings.initial) );
            if (f->filtComp.subStrings.any != NULL)
            {
                int i = 0;
                for (i = 0; i<f->filtComp.subStrings.anySize; i++)
                {
                    VmDirFreeBervalContent( &(f->filtComp.subStrings.any[i]) );
                }
                VMDIR_SAFE_FREE_MEMORY(f->filtComp.subStrings.any);
            }
            VmDirFreeBervalContent( &(f->filtComp.subStrings.final) );
            break;

        case LDAP_FILTER_PRESENT:
            // nothing to free here. type points in ber.
            break;

        case FILTER_ONE_LEVEL_SEARCH:
            VmDirFreeBervalContent( &(f->filtComp.parentDn) );
            break;

        default:
          VmDirLog( LDAP_DEBUG_ANY, "DeleteFilter: unknown filter type=%lu", f->choice );
          break;
    }

    DeleteCandidates( &(f->candidates) );

    if (f->pBer)
    {
        ber_free(f->pBer, 1);
    }

    VMDIR_SAFE_FREE_MEMORY( f );
    VmDirLog( LDAP_DEBUG_TRACE, "DeleteFilter: End" );
}

DWORD
StrFilterToFilter(
    PCSTR pszString,
    PVDIR_FILTER *ppFilter
    )
{
    DWORD dwError = 0;
    int res = 0;
    BerElement *ber = NULL;
    PVDIR_OPERATION pOperation = NULL;
    VDIR_LDAP_RESULT lr = {0};
    PVDIR_FILTER pFilter = NULL;

    ber = ber_alloc_t(LBER_USE_DER);
    if (ber == NULL)
    {
        dwError = LDAP_NO_MEMORY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Private function from libldap.
    res = ldap_pvt_put_filter(ber, pszString);
    if (res)
    {
        dwError = LDAP_FILTER_ERROR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ber_rewind(ber);

    dwError = VmDirExternalOperationCreate(ber, -1, LDAP_REQ_SEARCH, NULL, &pOperation);
    BAIL_ON_VMDIR_ERROR(dwError);

    res = ParseFilter(pOperation, &pFilter, &lr);
    if (res)
    {
        dwError = LDAP_FILTER_ERROR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pFilter->pBer = ber;
    ber = NULL;

    *ppFilter = pFilter;

cleanup:
    if (ber)
    {
        ber_free(ber, 1);
    }
    if (pOperation)
    {
        VmDirFreeOperation(pOperation);
    }
    return dwError;

error:
    goto cleanup;
}

DWORD
FilterToStrFilter(
    PVDIR_FILTER f,
    PVDIR_BERVALUE strFilter
    )
{
    DWORD dwError = 0;

    VmDirLog(LDAP_DEBUG_TRACE, "FilterToStrFilter: Begin, filter type: %ld", f->choice);

    if (strFilter->lberbv.bv_val == NULL)
    {
        size_t requiredSize = RequiredSizeForStrFilter(f);
        dwError = VmDirAllocateMemory(requiredSize + 1, (PVOID *)&strFilter->lberbv.bv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        strFilter->bOwnBvVal = TRUE;
        strFilter->lberbv.bv_len = 0;
    }

    // TODO: how do we provide the proper buffer size to VmDirStringPrintFA
    // functions below ?

    switch ( f->choice )
    {
        case LDAP_FILTER_AND:
        case LDAP_FILTER_OR:
        {
            PVDIR_FILTER curr = NULL;

            if (f->choice == LDAP_FILTER_AND)
            {
                VmDirStringPrintFA( strFilter->lberbv.bv_val + strFilter->lberbv.bv_len, 3, "(&");
                strFilter->lberbv.bv_len += 2;
            }
            else
            {
                VmDirStringPrintFA( strFilter->lberbv.bv_val + strFilter->lberbv.bv_len, 3, "(|");
                strFilter->lberbv.bv_len += 2;
            }

            for ( curr = f->filtComp.complex; curr != NULL; curr = curr->next )
            {
                dwError = FilterToStrFilter(curr, strFilter);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            VmDirStringPrintFA( strFilter->lberbv.bv_val + strFilter->lberbv.bv_len, 2, ")");
            strFilter->lberbv.bv_len += 1;

            break;
        }

        case LDAP_FILTER_NOT:
            VmDirStringPrintFA( strFilter->lberbv.bv_val + strFilter->lberbv.bv_len, 3, "(!");
            strFilter->lberbv.bv_len += 2;

            dwError = FilterToStrFilter(f->filtComp.complex, strFilter);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirStringPrintFA( strFilter->lberbv.bv_val + strFilter->lberbv.bv_len, 2, ")");
            strFilter->lberbv.bv_len += 1;
            break;

        case LDAP_FILTER_EQUALITY:
            VmDirStringPrintFA(
                strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                1 + f->filtComp.ava.type.lberbv.bv_len + 1 + f->filtComp.ava.value.lberbv.bv_len + 1 + 1,
                "(%s=%s)",
                f->filtComp.ava.type.lberbv.bv_val,
                f->filtComp.ava.value.lberbv.bv_val);
            strFilter->lberbv.bv_len += 1 + f->filtComp.ava.type.lberbv.bv_len + 1 + f->filtComp.ava.value.lberbv.bv_len + 1;
            break;

        case LDAP_FILTER_SUBSTRINGS:
        {
            int i = 0;

            VmDirStringPrintFA(
                strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                1 + f->filtComp.subStrings.type.lberbv.bv_len + 1 + 1,
                "(%s=",
                f->filtComp.subStrings.type.lberbv.bv_val
            );
            strFilter->lberbv.bv_len += 1 + f->filtComp.subStrings.type.lberbv.bv_len + 1;

            if (f->filtComp.subStrings.initial.lberbv.bv_len != 0)
            {
                VmDirStringPrintFA(
                    strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                    f->filtComp.subStrings.initial.lberbv.bv_len + 1,
                    "%s",
                    f->filtComp.subStrings.initial.lberbv.bv_val
                );
                strFilter->lberbv.bv_len += f->filtComp.subStrings.initial.lberbv.bv_len;
            }

            strFilter->lberbv.bv_val[strFilter->lberbv.bv_len++] = '*';

            for (i = 0; i < f->filtComp.subStrings.anySize; i++)
            {
                VmDirStringPrintFA(
                    strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                    f->filtComp.subStrings.any[i].lberbv.bv_len + 1 + 1,
                    "%s*",
                    f->filtComp.subStrings.any[i].lberbv.bv_val
                );
                strFilter->lberbv.bv_len += f->filtComp.subStrings.any[i].lberbv.bv_len + 1;
            }
            if (strFilter->lberbv.bv_val[strFilter->lberbv.bv_len - 1] != '*')
            {
                strFilter->lberbv.bv_val[strFilter->lberbv.bv_len++] = '*';
            }
            if (f->filtComp.subStrings.final.lberbv.bv_len != 0)
            {
                VmDirStringPrintFA(
                    strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                    f->filtComp.subStrings.final.lberbv.bv_len + 1,
                    "%s",
                    f->filtComp.subStrings.final.lberbv.bv_val
                );
                strFilter->lberbv.bv_len += f->filtComp.subStrings.final.lberbv.bv_len;
            }
            VmDirStringPrintFA(
                strFilter->lberbv.bv_val + strFilter->lberbv.bv_len, 2, ")");
            strFilter->lberbv.bv_len += 1;

            break;
        }

        case LDAP_FILTER_GE:
            VmDirStringPrintFA(
                strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                1 + f->filtComp.ava.type.lberbv.bv_len + 2 + f->filtComp.ava.value.lberbv.bv_len + 1 + 1,
                "(%s>=%s)",
                f->filtComp.ava.type.lberbv.bv_val,
                f->filtComp.ava.value.lberbv.bv_val
            );
            strFilter->lberbv.bv_len += 1 + f->filtComp.ava.type.lberbv.bv_len + 2 + f->filtComp.ava.value.lberbv.bv_len + 1;
            break;

        case LDAP_FILTER_LE:
            VmDirStringPrintFA(
                strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                1 + f->filtComp.ava.type.lberbv.bv_len + 2 + f->filtComp.ava.value.lberbv.bv_len + 1 + 1,
                "(%s<=%s)",
                f->filtComp.ava.type.lberbv.bv_val,
                f->filtComp.ava.value.lberbv.bv_val
            );
            strFilter->lberbv.bv_len += 1 + f->filtComp.ava.type.lberbv.bv_len + 2 + f->filtComp.ava.value.lberbv.bv_len + 1;
            break;

        case LDAP_FILTER_PRESENT:
            VmDirStringPrintFA(
                strFilter->lberbv.bv_val + strFilter->lberbv.bv_len,
                1 + f->filtComp.present.lberbv.bv_len + 3 + 1,
                "(%s=*)",
                f->filtComp.present.lberbv.bv_val
            );
            strFilter->lberbv.bv_len += 1 + f->filtComp.present.lberbv.bv_len + 3;
            break;

        case FILTER_ONE_LEVEL_SEARCH:
            break;
        default:
          VmDirLog( LDAP_DEBUG_ANY, "FilterToStrFilter: unknown filter type=%lu", f->choice );
          break;
    }

cleanup:
    VmDirLog(LDAP_DEBUG_TRACE, "FilterToStrFilter: End %d", dwError);
    return dwError;

error:
    VmDirFreeBervalContent(strFilter);
    goto cleanup;
}

/* ParseFilter() parses filter present on the wire.
 *
 * From RFC 4511:
 *     Filter ::= CHOICE {
 *              and              [0] SET SIZE (1..MAX) OF filter Filter,
 *              or               [1] SET SIZE (1..MAX) OF filter Filter,
 *              not              [2] Filter,
 *              equalityMatch    [3] AttributeValueAssertion,
 *              substrings       [4] SubstringFilter,
 *              greaterOrEqual   [5] AttributeValueAssertion,
 *              lessOrEqual      [6] AttributeValueAssertion,
 *              present          [7] AttributeDescription,
 *              approxMatch      [8] AttributeValueAssertion,
 *              extensibleMatch  [9] MatchingRuleAssertion,
 *              ...  }
 *
 *        SubstringFilter ::= SEQUENCE {
 *              type              AttributeDescription,
 *              substrings      SEQUENCE SIZE (1..MAX) OF substring CHOICE {
 *                     initial  [0] AssertionValue,  -- can occur at most once
 *                     any      [1] AssertionValue,
 *                     final    [2] AssertionValue } -- can occur at most once
 *              }
 *
 *        MatchingRuleAssertion ::= SEQUENCE {
 *              matchingRule     [1] MatchingRuleId OPTIONAL,
 *              type             [2] AttributeDescription OPTIONAL,
 *              matchValue       [3] AssertionValue,
 *              dnAttributes     [4] BOOLEAN DEFAULT FALSE }
 */

int
ParseFilter(
    VDIR_OPERATION *  op,
    VDIR_FILTER **    filter,
    VDIR_LDAP_RESULT *      lr )
{
    ber_tag_t    tag = LBER_ERROR;
    ber_len_t    len = 0;
    int          retVal = LDAP_SUCCESS;
    VDIR_FILTER *     f = NULL;
    PSTR         pszLocalErrorMsg = NULL;
    BerElement * ber = op->ber;

    VmDirLog( LDAP_DEBUG_TRACE, "ParseFilter: Begin." );

    assert( ber != NULL && filter != NULL && lr != NULL );

    if (VmDirAllocateMemory( sizeof(VDIR_FILTER), (PVOID *)&f ) != 0)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR( retVal );
    }
    *filter = f;

    if ((f->choice = ber_peek_tag( ber, &len )) == LBER_ERROR)
    {
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error in parsing the filter");
    }

    VMDIR_LOG_DEBUG( LDAP_DEBUG_TRACE, "ParseFilter: filter type: %ld", f->choice );

    switch ( f->choice )
    {
        case LDAP_FILTER_AND:
        case LDAP_FILTER_OR:
            retVal = ParseComplexFilterComponents( op, &f->filtComp.complex, lr );
            BAIL_ON_VMDIR_ERROR( retVal );
            if ( f->filtComp.complex == NULL )
            {
                retVal = lr->errCode = LDAP_PROTOCOL_ERROR;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "No filter components in an AND/OR filter");
            }
            break;

        case LDAP_FILTER_NOT:
            tag = ber_skip_tag( ber, &len ); // Skip just peeked choice tag.
            retVal = ParseFilter( op, &f->filtComp.complex, lr );
            BAIL_ON_VMDIR_ERROR( retVal );
            if ( f->filtComp.complex == NULL )
            {
                retVal = lr->errCode = LDAP_PROTOCOL_ERROR;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "No filter components in a NOT filter");
            }
            break;

        case LDAP_FILTER_EQUALITY:
            retVal = ParseAva( op, f, lr );
            BAIL_ON_VMDIR_ERROR( retVal );
            break;

        case LDAP_FILTER_SUBSTRINGS:
            retVal = ParseSubStrings( op, f, lr );
            BAIL_ON_VMDIR_ERROR( retVal );
            break;

        case LDAP_FILTER_GE:
        case LDAP_FILTER_LE:
            retVal = ParseAva( op, f, lr );
            BAIL_ON_VMDIR_ERROR( retVal );
            break;

        case LDAP_FILTER_PRESENT:
            // 'm' => present (type) points in-place in ber
            if ( ber_scanf( ber, "m", &(f->filtComp.present.lberbv) ) == LBER_ERROR )
            {
                lr->errCode = LDAP_PROTOCOL_ERROR;
                retVal = LDAP_NOTICE_OF_DISCONNECT;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "Error in parsing the filter");
            }
            break;

        default:
            VmDirLog( LDAP_DEBUG_ANY, "ParseFilter: Filter type not supported", f->choice );

            // Move to the end of this filter component/element
            if (ber_scanf( ber, "x" ) == LBER_ERROR)
            {
                lr->errCode = LDAP_PROTOCOL_ERROR;
                retVal = LDAP_NOTICE_OF_DISCONNECT;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                "Error in parsing the filter");
            }

            f->computeResult = FILTER_RES_UNDEFINED;
            break;
    }

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "ParseFilter: End %d", retVal );
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return( retVal );

error:
    if ( retVal == LDAP_NOTICE_OF_DISCONNECT )
    {
        DeleteFilter( f );
        *filter = NULL;
    }
    else
    {
        f->computeResult = FILTER_RES_UNDEFINED;
        retVal = LDAP_SUCCESS;
    }

    VMDIR_APPEND_ERROR_MSG(lr->pszErrMsg, pszLocalErrorMsg);

    goto cleanup;
}

/* ParseAva: Parses a filter Attribute Value Assertion from wire/ber into an ava filter component.
 *
 * RFC 4511: Section 4.5.1.7: SearchRequest.filter
 *
 * A filter item evaluates to Undefined when the server would not be able to determine whether the assertion
 * value matches an entry.
 * Examples include:
 *  - An attribute description in an equalityMatch, substrings, greaterOrEqual, lessOrEqual, approxMatch, or
 *  extensibleMatch filter is not recognized by the server.
 *  - The attribute type does not define the appropriate matching rule.
 *  - A MatchingRuleId in the extensibleMatch is not recognized by the server or is not valid for the attribute
 *  type.
 *  - The type of filtering requested is not implemented. - The assertion value is invalid.
 *  For example, if a server did not recognize the attribute type shoeSize, the filters (shoeSize=*),
 *  (shoeSize=12), (shoeSize>=12), and (shoeSize<=12) would each evaluate to Undefined.
 *
 */

static int
ParseAva(
    VDIR_OPERATION *     op,
    VDIR_FILTER *        f,
    VDIR_LDAP_RESULT *         lr )
{
    int          retVal = LDAP_SUCCESS;
    PSTR         pszLocalErrorMsg = NULL;
    BerElement * ber = op->ber;
PSTR        pszOCType = "objectclass";
PSTR        pszContainer = "container";
PSTR        pszVMWContainer = "vmwContainer";

    VmDirLog( LDAP_DEBUG_TRACE, "ParseAva: Begin" );

    // 'm' => in-place in ber
    if (ber_scanf( ber, "{mm}", &(f->filtComp.ava.type.lberbv), &(f->filtComp.ava.value.lberbv) ) == LBER_ERROR )
    {
        VmDirLog( LDAP_DEBUG_ANY, "  ParseAva ber_scanf" );
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error decoding attribute value assertion");
    }

///////////////////////////////////////////////////////
// HACK to change vmwContainer to container
// i.e. from "obejctclass=vmwContainer" to "obejctclass=container"
// should retire this after CM move to "container"
///////////////////////////////////////////////////////
if ( f->filtComp.ava.type.lberbv_len == VmDirStringLenA(pszOCType)
     &&
     VmDirStringCompareA( f->filtComp.ava.type.lberbv_val,pszOCType, FALSE) == 0
     &&
     f->filtComp.ava.value.lberbv_len == VmDirStringLenA(pszVMWContainer)
     &&
     VmDirStringCompareA( f->filtComp.ava.value.lberbv_val,pszVMWContainer, FALSE) == 0
   )
{
    VmDirStringCpyA( f->filtComp.ava.value.lberbv_val,  // original filter buffer for "vmwContainer"
                     f->filtComp.ava.value.lberbv_len,  // original filter buffer size
                     pszContainer);
    f->filtComp.ava.value.lberbv_len = VmDirStringLenA(pszContainer);
}

    if ((f->filtComp.ava.pATDesc = VmDirSchemaAttrNameToDesc( op->pSchemaCtx,
                                                              f->filtComp.ava.type.lberbv.bv_val)) == NULL)
    {
        f->computeResult = FILTER_RES_UNDEFINED; // See description in the function header.
    }
    else
    {
        if (VmDirSchemaBervalNormalize( op->pSchemaCtx, f->filtComp.ava.pATDesc,
                                        &(f->filtComp.ava.value) ) != LDAP_SUCCESS)
        {
            VmDirLog( LDAP_DEBUG_ANY, "  ParseAva attribute value normalization failed for a "
                      "filter component, filter type = %s", f->filtComp.ava.type.lberbv.bv_val  );
            retVal = lr->errCode = LDAP_OPERATIONS_ERROR;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Normalizing a filter attribute (%s)",
                                            VDIR_SAFE_STRING(f->filtComp.ava.pATDesc->pszName));
        }
    }
    retVal = LDAP_SUCCESS;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "ParseAva: End" );
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return retVal;

error:

    VMDIR_APPEND_ERROR_MSG(lr->pszErrMsg, pszLocalErrorMsg);

    goto cleanup;
}

static int
ParseComplexFilterComponents(
    VDIR_OPERATION *   op,
    VDIR_FILTER **     f,
    VDIR_LDAP_RESULT *       lr )
{
    int          retVal = LDAP_SUCCESS;
    VDIR_FILTER **    curr = NULL;
    ber_tag_t    tag = LBER_ERROR;
    ber_len_t    len = 0;
    char *       endOfFiltersMarker = NULL;
    BerElement * ber = op->ber;

    VmDirLog( LDAP_DEBUG_TRACE, "ParseComplexFilterComponents: Begin" );
    curr = f;
    for ( tag = ber_first_element( ber, &len, &endOfFiltersMarker ); tag != LBER_DEFAULT;
          tag = ber_next_element( ber, &len, endOfFiltersMarker ) )
    {
        retVal = ParseFilter( op, curr, lr );
        BAIL_ON_VMDIR_ERROR( retVal );
        curr = &(*curr)->next;
    }
    *curr = NULL;
    retVal = LDAP_SUCCESS;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "ParseComplexFilterComponents: End" );
    return( retVal );

error:
    goto cleanup;
}

/* RFC 4511: 4.5.1.7.2. SearchRequest.filter.substrings
 *  There SHALL be at most one �initial� and at most one �final� in the �substrings� of a SubstringFilter.
 *  If �initial� is present, it SHALL be the first element of �substrings�. If �final� is present, it SHALL
 *  be the last element of �substrings�.
 */

static int
ParseSubStrings(
    VDIR_OPERATION *  op,
    VDIR_FILTER *     f,
    VDIR_LDAP_RESULT *      lr )
{
    int          retVal = LDAP_SUCCESS;
    ber_tag_t    tag = LBER_ERROR;
    ber_len_t    len = 0;
    char *       endSubStringsMarker = NULL;
    ber_tag_t    subStringType = LBER_ERROR;
    BOOLEAN      foundInitial = FALSE;
    BOOLEAN      foundAny = FALSE;
    BOOLEAN      foundFinal = FALSE;
    PSTR         pszLocalErrorMsg = NULL;
    BerElement * ber = op->ber;

    VmDirLog( LDAP_DEBUG_TRACE, "ParseSubStrings: Begin" );

    // 'm' => in-place in ber
    if (ber_scanf( ber, "{m", &(f->filtComp.subStrings.type.lberbv) ) == LBER_ERROR )
    {
        VmDirLog( LDAP_DEBUG_ANY, "  ParseSubStrings ber_scanf" );
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Error decoding subStrings filter");
    }
    if ((f->filtComp.subStrings.pATDesc = VmDirSchemaAttrNameToDesc( op->pSchemaCtx,
                                                                     f->filtComp.subStrings.type.lberbv.bv_val)) == NULL)
    {
        f->computeResult = FILTER_RES_UNDEFINED; // See description in the function header.
    }

    // Get subStrings. ber_first_element => skip the sequence header, set the cursor at the 1st control in the SEQ
    // of SEQ
    for( tag = ber_first_element( ber, &len, &endSubStringsMarker ); tag != LBER_ERROR;
         tag = ber_next_element( ber, &len, endSubStringsMarker ) )
    {
        if ((subStringType = ber_peek_tag( ber, &len )) == LBER_ERROR)
        {
            lr->errCode = LDAP_PROTOCOL_ERROR;
            retVal = LDAP_NOTICE_OF_DISCONNECT;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                            "Error in parsing the filter");
        }

        VmDirLog( LDAP_DEBUG_TRACE, "ParseSubStrings: SubString type: %ld", subStringType );

        switch ( subStringType )
        {
            case LDAP_SUBSTRING_INITIAL:
                if (foundInitial || foundAny || foundFinal)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                            "Error in parsing the subString filter. INITIAL out-of-order or has already occurred once");

                }
                foundInitial = TRUE;
                // m => in-place
                if (ber_scanf( ber, "m", &(f->filtComp.subStrings.initial.lberbv) ) == LBER_ERROR)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error in parsing INITIAL subString string value.");
                }
                if (f->filtComp.subStrings.pATDesc && (VmDirSchemaBervalNormalize( op->pSchemaCtx,
                                                          f->filtComp.subStrings.pATDesc,
                                                          &(f->filtComp.subStrings.initial) ) != LDAP_SUCCESS))
                {
                    VmDirLog( LDAP_DEBUG_ANY, "  ParseAva attribute value normalization failed for a filter "
                              "component, filter type = %s", f->filtComp.subStrings.type.lberbv.bv_val  );
                    retVal = lr->errCode = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error normalizing a filter attribute value");
                }
                break;
            case LDAP_SUBSTRING_ANY:
                if (foundFinal)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error in parsing the subString filter. ANY out-of-order");
                }
                foundAny = TRUE;
                if (f->filtComp.subStrings.any == NULL)
                {
                    f->filtComp.subStrings.anySize = 0;
                    f->filtComp.subStrings.anyMax = START_ANY_ALLOC_SIZE;

                    if (VmDirAllocateMemory( f->filtComp.subStrings.anyMax * sizeof( VDIR_BERVALUE ),
                                             (PVOID *)&f->filtComp.subStrings.any ) != 0)
                    {
                        retVal = LDAP_OPERATIONS_ERROR;
                        BAIL_ON_VMDIR_ERROR( retVal );
                    }
                }
                else if (f->filtComp.subStrings.anySize == f->filtComp.subStrings.anyMax)
                {
                    f->filtComp.subStrings.anyMax = f->filtComp.subStrings.anySize * 2;

                    retVal = VmDirReallocateMemory( f->filtComp.subStrings.any, (PVOID*)(&(f->filtComp.subStrings.any)), f->filtComp.subStrings.anyMax * sizeof( VDIR_BERVALUE ) );
                    BAIL_ON_VMDIR_ERROR( retVal );
                }
                // m => in-place
                if (ber_scanf( ber, "m",
                               &(f->filtComp.subStrings.any[f->filtComp.subStrings.anySize].lberbv) ) == LBER_ERROR)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error in parsing INITIAL subString string value.");
                }
                if (f->filtComp.subStrings.pATDesc &&
                    (VmDirSchemaBervalNormalize( op->pSchemaCtx, f->filtComp.subStrings.pATDesc,
                       &(f->filtComp.subStrings.any[f->filtComp.subStrings.anySize]) ) != LDAP_SUCCESS))
                {
                    VmDirLog( LDAP_DEBUG_ANY, "  ParseAva attribute value normalization failed for a filter "
                              "component, filter type = %s", f->filtComp.subStrings.type.lberbv.bv_val  );
                    retVal = lr->errCode = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error normalizing a filter attribute value");
                }
                f->filtComp.subStrings.anySize++;
                break;
            case LDAP_SUBSTRING_FINAL:
                if (foundFinal)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error in parsing the subString filter. FINAL has already occurred once");
                }
                foundFinal = TRUE;
                if (ber_scanf( ber, "m", &(f->filtComp.subStrings.final.lberbv) ) == LBER_ERROR)
                {
                    lr->errCode = LDAP_PROTOCOL_ERROR;
                    retVal = LDAP_NOTICE_OF_DISCONNECT;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error in parsing INITIAL subString string value.");
                }
                if (f->filtComp.subStrings.pATDesc &&
                    (VmDirSchemaBervalNormalize( op->pSchemaCtx, f->filtComp.subStrings.pATDesc,
                                                 &(f->filtComp.subStrings.final) ) != LDAP_SUCCESS))
                {
                    VmDirLog( LDAP_DEBUG_ANY, "  ParseAva attribute value normalization failed for a filter "
                              "component, filter type = %s", f->filtComp.subStrings.type.lberbv.bv_val  );
                    retVal = lr->errCode = LDAP_OPERATIONS_ERROR;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                                    "Error normalizing a filter attribute (%s)",
                                                    VDIR_SAFE_STRING(f->filtComp.subStrings.pATDesc->pszName));
                }
                break;
            default:
                break;
        }
    }

    if ( ber_scanf( ber, "}") == LBER_ERROR )
    {
        VmDirLog( LDAP_DEBUG_ANY, "ParseSubStrings: ber_scanf failed" );
        lr->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                        "Decoding error while parsing the end of message.");
    }
    retVal = LDAP_SUCCESS;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "ParseSubStrings: End" );
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);

    return retVal;

error:

    VMDIR_APPEND_ERROR_MSG(lr->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

static size_t
RequiredSizeForStrFilter(
    VDIR_FILTER * f)
{
    size_t strSize = 0;

    assert( f != NULL );

    VmDirLog( LDAP_DEBUG_TRACE, "RequiredSizeForStrFilter: Begin, filter choice = %d.", f->choice );

    switch ( f->choice )
    {
        case LDAP_FILTER_AND:
        case LDAP_FILTER_OR:
        {
            VDIR_FILTER * curr = NULL;
            strSize = 3; // for (&) or for (|)
            for ( curr = f->filtComp.complex; curr != NULL; curr = curr->next )
            {
                strSize += RequiredSizeForStrFilter(curr);
            }
            break;
        }

        case LDAP_FILTER_NOT:
            strSize = 3; // for (!)
            strSize += RequiredSizeForStrFilter( f->filtComp.complex );
            break;

        case LDAP_FILTER_EQUALITY:
            strSize = f->filtComp.ava.type.lberbv.bv_len + f->filtComp.ava.value.lberbv.bv_len + 3 /* (=) */;
            break;

        case LDAP_FILTER_SUBSTRINGS:
        {
            int i = 0;

            strSize = f->filtComp.subStrings.type.lberbv.bv_len + 3;   // (=)
            strSize += f->filtComp.subStrings.initial.lberbv.bv_len + 1; // xyz*
            for (i = 0; i < f->filtComp.subStrings.anySize; i++)
            {
                strSize += f->filtComp.subStrings.any[i].lberbv.bv_len + 2; // *xyz*
            }
            strSize += f->filtComp.subStrings.final.lberbv.bv_len + 1; // *xyz
            break;
        }

        case LDAP_FILTER_GE:
        case LDAP_FILTER_LE:
            strSize = f->filtComp.ava.type.lberbv.bv_len + f->filtComp.ava.value.lberbv.bv_len + 4 /* (>=) or (<=) */;
            break;

        case LDAP_FILTER_PRESENT:
            strSize = f->filtComp.present.lberbv.bv_len + 4 /* (=*) */;
            break;

        case FILTER_ONE_LEVEL_SEARCH:
            break;
        default:
          VmDirLog( LDAP_DEBUG_ANY, "RequiredSizeForStrFilter: unknown filter type=%lu", f->choice );
          break;
    }

    VmDirLog( LDAP_DEBUG_TRACE, "RequiredSizeForStrFilter: End, required str size = %d.", strSize );
    return strSize;
}

static VDIR_FILTER_COMPUTE_RESULT
TestAvaFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f)
{
    unsigned int                       j = 0;
    VDIR_FILTER_COMPUTE_RESULT         retVal = FILTER_RES_FALSE;
    VDIR_ATTRIBUTE *                 attr = NULL;
    // Filter is normalized (very early) during parsing.
    char *                      normFiltVal = BERVAL_NORM_VAL(f->filtComp.ava.value);
    ber_len_t                   normFiltValLen = BERVAL_NORM_LEN(f->filtComp.ava.value);
    char *                      normAttrVal = NULL;
    ber_len_t                   normAttrValLen = 0;
    PFN_VDIR_COMPARE_FUNCTION   pCompareFunc = NULL;
    VDIR_SCHEMA_MATCH_TYPE      matchType = 0;

    assert( op && e && f &&
            (f->choice == LDAP_FILTER_EQUALITY ||
             f->choice == LDAP_FILTER_GE       ||
             f->choice == LDAP_FILTER_LE));

    VmDirLog( LDAP_DEBUG_TRACE, "TestAvaFilter: Begin, filter attribute type = %s",
              f->filtComp.ava.type.lberbv.bv_val);

    if (f->computeResult != FILTER_RES_NORMAL)
    {
        return f->computeResult;
    }

    assert( f->filtComp.ava.pATDesc != NULL );

    switch (f->choice )
    {
        case LDAP_FILTER_EQUALITY:
            pCompareFunc = f->filtComp.ava.pATDesc->pEqualityMR ? f->filtComp.ava.pATDesc->pEqualityMR->pCompareFunc :
                                                                  NULL;
            matchType = VDIR_SCHEMA_MATCH_EQUAL;
            break;
        case LDAP_FILTER_GE:
            pCompareFunc = f->filtComp.ava.pATDesc->pOrderingMR ? f->filtComp.ava.pATDesc->pOrderingMR->pCompareFunc :
                                                                  NULL;
            if ( pCompareFunc == NULL )
            {
                retVal = FILTER_RES_UNDEFINED;
                goto done;
            }
            matchType = VDIR_SCHEMA_MATCH_GE;
            break;
        case LDAP_FILTER_LE:
            pCompareFunc = f->filtComp.ava.pATDesc->pOrderingMR ? f->filtComp.ava.pATDesc->pOrderingMR->pCompareFunc :
                                                                  NULL;
            if ( pCompareFunc == NULL )
            {
                retVal = FILTER_RES_UNDEFINED;
                goto done;
            }
            matchType = VDIR_SCHEMA_MATCH_LE;
            break;
        default:
            assert( FALSE );
            break;
    }

    for ( attr = e->attrs; attr != NULL; attr = attr->next )
    {
        if ((f->filtComp.ava.type.lberbv.bv_len != attr->type.lberbv.bv_len) ||
            (VmDirStringCompareA( f->filtComp.ava.type.lberbv.bv_val, attr->type.lberbv.bv_val, FALSE ) != 0))
        {
            continue;
        }

        for (j = 0; j < attr->numVals; j++)
        {
            // Normalize attribute value if not already normalized.
            if (attr->vals[j].bvnorm_val == NULL &&
                VmDirSchemaBervalNormalize( op->pSchemaCtx, f->filtComp.ava.pATDesc,
                                            &(attr->vals[j]) ) != LDAP_SUCCESS)
            {
                VmDirLog( LDAP_DEBUG_ANY, "  TestAvaFilter attribute value normalization failed for the "
                          "entry attribute type = %s", attr->type.lberbv.bv_val );
                retVal = FILTER_RES_UNDEFINED;
                goto done;

            }

            if (pCompareFunc)
            {
                if ((*pCompareFunc)( matchType, &attr->vals[j], &f->filtComp.ava.value ))
                {
                    retVal = FILTER_RES_TRUE;
                    goto done;
                }
            }
            else
            {
                normAttrVal = BERVAL_NORM_VAL(attr->vals[j]);
                normAttrValLen = BERVAL_NORM_LEN(attr->vals[j]);

                if ((normFiltValLen == normAttrValLen) && (memcmp( normFiltVal, normAttrVal, normFiltValLen) == 0))
                {
                    retVal = FILTER_RES_TRUE;
                    goto done;
                }
            }
        }
        retVal = FILTER_RES_FALSE;
        goto done;
    }
    retVal = FILTER_RES_FALSE;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "TestAvaFilter: End, retVal = %s",
              retVal == FILTER_RES_TRUE ? "TRUE" : (retVal == FILTER_RES_FALSE ? "FALSE" : "UNDEFIEND") );
    return retVal;
}

static VDIR_FILTER_COMPUTE_RESULT
TestOneLevelFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f)
{
    VDIR_FILTER_COMPUTE_RESULT  retVal = FILTER_RES_FALSE;
    VDIR_BERVALUE               parentDN = VDIR_BERVALUE_INIT;

    assert( op && e && f && f->choice == FILTER_ONE_LEVEL_SEARCH );

    VmDirLog( LDAP_DEBUG_TRACE, "TestOneLevelFilter: Begin");

    if (f->computeResult != FILTER_RES_NORMAL)
    {
        retVal = f->computeResult;
        goto done;
    }
    if (VmDirNormalizeDN( &(e->dn), op->pSchemaCtx ) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "TestOneLevelFilter: DN normalization failed." );
        retVal = FILTER_RES_FALSE;
        goto done;
    }
    if (VmDirGetParentDN( &(e->dn), &parentDN ) != 0)
    {
        VmDirLog( LDAP_DEBUG_ANY, "TestOneLevelFilter: VmDirGetParentDN() failed." );
        retVal = FILTER_RES_FALSE;
        goto done;
    }
    if (parentDN.bvnorm_len == f->filtComp.parentDn.bvnorm_len  &&
        ( parentDN.bvnorm_len == 0
          ||
          VmDirStringCompareA(parentDN.bvnorm_val, f->filtComp.parentDn.bvnorm_val, TRUE) == 0
        )
       )
    {
        retVal = FILTER_RES_TRUE;
        goto done;
    }
    retVal = FILTER_RES_FALSE;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "TestOneLevelFilter: End, retVal = %s",
              retVal == FILTER_RES_TRUE ? "TRUE" : (retVal == FILTER_RES_FALSE ? "FALSE" : "UNDEFIEND") );

    VmDirFreeBervalContent( &parentDN );
    return retVal;
}

static VDIR_FILTER_COMPUTE_RESULT
TestPresentFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f)
{
    VDIR_FILTER_COMPUTE_RESULT    retVal = FILTER_RES_FALSE;
    VDIR_ATTRIBUTE *            attr = NULL;

    assert( op && e && f && f->choice == LDAP_FILTER_PRESENT );

    VmDirLog( LDAP_DEBUG_TRACE, "TestPresentFilter: Begin, filter attribute type = %s",
              f->filtComp.present.lberbv.bv_val);

    if (f->computeResult != FILTER_RES_NORMAL && f->computeResult != FILTER_RES_TRUE)
    {
        return f->computeResult;
    }

    for ( attr = e->attrs; attr != NULL; attr = attr->next )
    {
        if ((f->filtComp.present.lberbv.bv_len == attr->type.lberbv.bv_len) &&
            (VmDirStringCompareA( f->filtComp.present.lberbv.bv_val, attr->type.lberbv.bv_val, FALSE ) == 0))
        {
            retVal = FILTER_RES_TRUE;
            goto done;
        }
    }
    retVal = FILTER_RES_FALSE;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "TestPresentFilter: End, retVal = %s",
              retVal == FILTER_RES_TRUE ? "TRUE" : (retVal == FILTER_RES_FALSE ? "FALSE" : "UNDEFIEND") );
    return retVal;
}

static VDIR_FILTER_COMPUTE_RESULT
TestSubstringsFilter(
    VDIR_OPERATION * op,
    VDIR_ENTRY *     e,
    VDIR_FILTER *    f)
{
    unsigned int                  j = 0;
    VDIR_FILTER_COMPUTE_RESULT    retVal = FILTER_RES_FALSE;
    VDIR_ATTRIBUTE *       attr = NULL;
    // Filter is normalized (very early) during parsing.
    char *                 initialFiltVal = NULL;
    ber_len_t              initialFiltLen = 0;
    char *                 finalFiltVal = NULL;
    ber_len_t              finalFiltLen = 0;
    int                    anyFiltSize = 0;
    char *                 attrVal = NULL;
    ber_len_t              attrValLen = 0;
    int                    exist_initial_true = 0;
    int                    exist_final_true = 0;
    int                    exist_any_true = 0;

    assert( op && e && f && f->choice == LDAP_FILTER_SUBSTRINGS );

    VmDirLog( LDAP_DEBUG_TRACE, "TestSubstringsFilter: Begin, filter attribute type = %s",
              f->filtComp.subStrings.type.lberbv.bv_val);

    if (f->computeResult != FILTER_RES_NORMAL)
    {
        return f->computeResult;
    }

    if (f->filtComp.subStrings.initial.lberbv.bv_len != 0)
    {
        initialFiltVal = BERVAL_NORM_VAL(f->filtComp.subStrings.initial);
        initialFiltLen = BERVAL_NORM_LEN(f->filtComp.subStrings.initial);
        exist_initial_true = 1;
    }

    if (f->filtComp.subStrings.final.lberbv.bv_len != 0)
    {
        finalFiltVal = BERVAL_NORM_VAL(f->filtComp.subStrings.final);
        finalFiltLen = BERVAL_NORM_LEN(f->filtComp.subStrings.final);
        exist_final_true = 1;
    }

    anyFiltSize=f->filtComp.subStrings.anySize;
    if (anyFiltSize)
    {
        exist_any_true = 1;
    }

    // Check if it is the DN filter
    if (VmDirStringCompareA( f->filtComp.ava.type.lberbv.bv_val, ATTR_DN, FALSE ) == 0)
    {
        // Normalize DN if not already normalized
        if ((e->dn.bvnorm_val == NULL) && (retVal = VmDirNormalizeDN( &(e->dn), op->pSchemaCtx )) != 0)
        {
            VmDirLog( LDAP_DEBUG_ANY, "TestSubstringsFilter: DN normalization - code(%d)", retVal);
            retVal = FILTER_RES_UNDEFINED;
            goto done;
        }

        attrVal = BERVAL_NORM_VAL(e->dn);
        attrValLen = BERVAL_NORM_LEN(e->dn);

        //Search with substree scope on entryDn should create a FINAL substring filter.
        if (finalFiltLen && attrValLen >= finalFiltLen &&
            memcmp( finalFiltVal, attrVal + (attrValLen - finalFiltLen), finalFiltLen) == 0)
        {
            retVal = FILTER_RES_TRUE;
        }
        else
        {
            retVal = FILTER_RES_FALSE;
        }
        goto done;
    }

    assert( f->filtComp.subStrings.pATDesc != NULL );

    for ( attr = e->attrs; attr != NULL; attr = attr->next )
    {
        if ((f->filtComp.subStrings.type.lberbv.bv_len != attr->type.lberbv.bv_len) ||
            (VmDirStringCompareA( f->filtComp.subStrings.type.lberbv.bv_val, attr->type.lberbv.bv_val, FALSE ) != 0))
        {
            continue;
        }

        for (j = 0; j < attr->numVals; j++)
        {
            int eval_initial_true = 0;
            int eval_final_true = 0;
            int eval_any_true = 0;
            int exist_map = 0;
            int eval_map = 0;
            ber_len_t initial_match_len = 0;
            ber_len_t finial_match_len = 0;

            // Normalize attribute value if not already normalized.
            if (attr->vals[j].bvnorm_val == NULL &&
                VmDirSchemaBervalNormalize( op->pSchemaCtx, f->filtComp.subStrings.pATDesc,
                                            &(attr->vals[j]) ) != LDAP_SUCCESS)
            {
                VmDirLog( LDAP_DEBUG_ANY, "  TestSubstringsFilter attribute value normalization failed for the "
                          "entry attribute type = %s", attr->type.lberbv.bv_val );
                retVal = FILTER_RES_UNDEFINED;
                goto done;

            }

            attrVal = BERVAL_NORM_VAL(attr->vals[j]);
            attrValLen = BERVAL_NORM_LEN(attr->vals[j]);

            if (initialFiltLen && (attrValLen >= initialFiltLen) &&
                memcmp( initialFiltVal, attrVal, initialFiltLen) == 0)
            {
                eval_initial_true = 1;
                initial_match_len = initialFiltLen;
            }

            if (finalFiltLen && (attrValLen >= finalFiltLen) &&
                memcmp( finalFiltVal, attrVal + (attrValLen - finalFiltLen), finalFiltLen) == 0)
            {
                eval_final_true = 1;
                finial_match_len = finalFiltLen;
            }
            if (anyFiltSize > 0 && TestSubstringAny(f->filtComp.subStrings.any, anyFiltSize,
                                       (attrVal + initial_match_len),
                                       (attrValLen - initial_match_len - finial_match_len))==FILTER_RES_TRUE)
            {
                eval_any_true = 1;
            }
            exist_map = exist_any_true << 2 | exist_final_true << 1 | exist_initial_true;
            eval_map = eval_any_true << 2 | eval_final_true << 1 | eval_initial_true;
            if ( (exist_map & eval_map) == exist_map )
            {
                retVal = FILTER_RES_TRUE;
                goto done;
            }
        }
    }
    retVal = FILTER_RES_FALSE;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "TestSubstringsFilter: End, retVal = %s",
              retVal == FILTER_RES_TRUE ? "TRUE" : (retVal == FILTER_RES_FALSE ? "FALSE" : "UNDEFIEND") );
    return retVal;
}

/*
 *  Return FILTER_RES_TRUE iif attr_val passed all substring "ANY" elements
 *  any_array:  an array of "ANY" components
 *  any_size: the size of any_array
 *  attr_val: string form of the attribute value to be tested.
 *  attr_len: the length of attr_val
 *  e.g. filter cn=*abc*def* has two "ANY" elements: any_array[0]="abc" and any_array[1]="def" with any_size 2
 *       filter cn=xy*abc*def*wz has two "ANY" elements though it also has an initial "xy" and a final "wz".
*/
static VDIR_FILTER_COMPUTE_RESULT
TestSubstringAny(
    VDIR_BERVALUE *any_array,
    int any_size,
    char *attr_val,
    ber_len_t attr_len)
{
    int i = 0;

    char *remainVal = attr_val;
    ber_len_t remainLen = attr_len;

    for (i = 0; i < any_size; i++ )
    {
        char *p_cur = NULL;
        char *normFiltVal = BERVAL_NORM_VAL(any_array[i]);
        ber_len_t normFiltValLen = BERVAL_NORM_LEN(any_array[i]);

next_pos:
        if ( normFiltValLen > remainLen )
        {
            break;
        }

        p_cur = memchr( remainVal, *normFiltVal, remainLen );

        if( p_cur == NULL )
        {
            break;
        }

        remainVal = p_cur;
        remainLen -= (ber_len_t)(p_cur - remainVal);

        if ( normFiltValLen > remainLen )
        {
            break;
        }

        if ( memcmp( remainVal, normFiltVal, normFiltValLen ) != 0)
        {
            remainVal++;
            remainLen--;
            goto next_pos;
        }

        remainVal += normFiltValLen;
        remainLen -= normFiltValLen;
    }

    if (i == any_size)
    {
        return FILTER_RES_TRUE;
    }
    return FILTER_RES_FALSE;
}
