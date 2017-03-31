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

static void
IntersectCandidates(
    VDIR_CANDIDATES *  srcCandidates,
    VDIR_CANDIDATES ** dstCandidates);

static
DWORD
VmDirReallocCandidates(
    PVDIR_CANDIDATES    pCands
    );

static void
SubtractCandidates(
    VDIR_CANDIDATES *  srcCandidates,
    VDIR_CANDIDATES ** dstCandidates);

static void
UnionCandidates(
    VDIR_CANDIDATES ** srcCandidates,
    VDIR_CANDIDATES ** dstCandidates);

static int
_VmDirCompareEntryIds(
    const void * pEID1,
    const void * pEID2);

PVDIR_CANDIDATES
NewCandidates(
    int      startAllocSize,
    BOOLEAN  positive)
{
    VDIR_CANDIDATES * cans = NULL;
    int          retVal = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "NewCandidates: Begin" );

    assert( startAllocSize >= 0);

    retVal = VmDirAllocateMemory( sizeof( VDIR_CANDIDATES ), (PVOID *)&cans );
    BAIL_ON_VMDIR_ERROR( retVal );

    retVal = VmDirAllocateMemory( startAllocSize * sizeof( ENTRYID ), (PVOID *)&cans->eIds );
    BAIL_ON_VMDIR_ERROR( retVal );

    cans->size = 0;
    cans->max = startAllocSize;
    cans->positive = positive;
    cans->eIdsSorted = FALSE;

cleanup:
    VmDirLog( LDAP_DEBUG_TRACE, "NewCandidates: End" );
    return cans;

error:
    cans = NULL;
    goto cleanup;
}

void
DeleteCandidates(
    VDIR_CANDIDATES **cans)
{
    VmDirLog( LDAP_DEBUG_TRACE, "DeleteCandidates: Begin" );

    if (cans)
    {
        if (*cans)
        {
            VMDIR_SAFE_FREE_MEMORY( (*cans)->eIds );
            VMDIR_SAFE_FREE_MEMORY( *cans );
        }
        *cans = NULL;
    }

    VmDirLog( LDAP_DEBUG_TRACE, "DeleteCandidates: End" );
}

/*
 * add ENTRYID to candidate list
 */
DWORD
VmDirAddToCandidates(
    PVDIR_CANDIDATES    pCands,
    ENTRYID             eId
    )
{
    DWORD   dwError = 0;

    assert(pCands);

    if (pCands->size == pCands->max)
    {
        dwError = VmDirReallocCandidates(pCands);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pCands->eIds[pCands->size] = eId;

    pCands->size++;

    VmDirLog( LDAP_DEBUG_TRACE, "AddToCandidates: eId = %lld", eId );

error:

    return dwError;
}

/* AndFilterResults: ANDs src filter processing result (computeResult and candidates list) with the dst filter
 * processing result (computeResult and candidates list). For an AND top level filter, this function is expected to be
 * called first for the "positive" filter components, and then for the "negative" filter components.
 *
 * Cases that are handled are:
 *     - both src and dst are positive candidates list. => Intersection needs to be done.
 *     - both src and dst are negative candidates list. => Union is needed.
 *     - dst is positive candidates list and src is negative candidates list. => Subtraction is needed.
 *
 * RFC 4511, Section 4.5.1.7:
 * A filter of the "and" choice is TRUE if all the filters in the SET OF evaluate to TRUE, FALSE if at least one filter
 * is FALSE, and Undefined otherwise.
 */
void
AndFilterResults(
    VDIR_FILTER * src,
    VDIR_FILTER * dst)
{
    VmDirLog( LDAP_DEBUG_TRACE, "AndFilterResults: Begin" );

    // First process computeResults, and then candidates lists.

    switch (src->computeResult)
    {
        case FILTER_RES_NORMAL:
            switch (dst->computeResult)
            {
                case FILTER_RES_NORMAL:
                    break; // normal processing of candidates lists is required.
                case FILTER_RES_TRUE:
                    if (src->candidates != NULL)
                    {
                        dst->candidates = src->candidates;
                        src->candidates = NULL;
                        dst->computeResult = FILTER_RES_NORMAL;
                    }
                    goto done;
                case FILTER_RES_FALSE:
                case FILTER_RES_UNDEFINED:
                    goto done;
            }
            break;
        case FILTER_RES_TRUE:
            switch (dst->computeResult)
            {
                case FILTER_RES_NORMAL:
                    // 1st time initialization of the top of the AND filter.
                    if (dst->candidates == NULL)
                    {
                        dst->computeResult = FILTER_RES_TRUE;
                    }
                    goto done;
                case FILTER_RES_TRUE:
                case FILTER_RES_FALSE:
                case FILTER_RES_UNDEFINED:
                    goto done;
            }
            break;
        case FILTER_RES_FALSE:
            DeleteCandidates( &(dst->candidates) );
            dst->computeResult = FILTER_RES_FALSE;
            goto done;
        case FILTER_RES_UNDEFINED:
            switch (dst->computeResult)
            {
                case FILTER_RES_NORMAL:
                    DeleteCandidates( &(dst->candidates) );
                    dst->computeResult = FILTER_RES_UNDEFINED;
                    goto done;
                case FILTER_RES_TRUE:
                    dst->computeResult = FILTER_RES_UNDEFINED;
                    goto done;
                case FILTER_RES_FALSE:
                case FILTER_RES_UNDEFINED:
                    goto done;
            }
            break;
    }

    // Handling a non-indexed attribute.
    if (src->candidates == NULL)
    {
        goto done;
    }
    // 1st time initialization of the top of the AND filter.
    if (dst->candidates == NULL)
    {
        dst->candidates = src->candidates;
        src->candidates = NULL;
        goto done;
    }
    if (dst->candidates->positive && src->candidates->positive)
    {  // Do an AND of +ve candidate lists.
        IntersectCandidates( src->candidates, &(dst->candidates) );
    }
    else if (dst->candidates->positive && !(src->candidates->positive))
    {  // Do an AND of a +ve candidate list with a -ve candidate list
        SubtractCandidates( src->candidates, &(dst->candidates) );
    }
    else if (!(dst->candidates->positive) && !(src->candidates->positive))
    {  // Do an OR of -ve candidate lists
        UnionCandidates( &(src->candidates), &(dst->candidates) );
    } else
    {
        assert( FALSE );
    }

done:
    VmDirLog( LDAP_DEBUG_TRACE, "AndFilterResults: End" );
}

/* NotFilterResults: Move the src filter candidates one level up in the filter tree to dst filter candidates,
 * and in this process negate the computeResult, if set, or negative the "positive" flag in src candidates.
 *
 */
void
NotFilterResults(
    VDIR_FILTER * src,
    VDIR_FILTER * dst)
{
    VmDirLog( LDAP_DEBUG_TRACE, "NotFilterResults: Begin" );

    switch( src->computeResult )
    {
        case FILTER_RES_NORMAL:
            dst->computeResult = FILTER_RES_NORMAL;
            break;
        case FILTER_RES_TRUE:
            dst->computeResult = FILTER_RES_FALSE;
            goto done;
        case FILTER_RES_FALSE:
            dst->computeResult = FILTER_RES_TRUE;
            goto done;
        case FILTER_RES_UNDEFINED:
            dst->computeResult = FILTER_RES_UNDEFINED;
            goto done;
        default:
            assert( FALSE );
    }
    if (src->candidates == NULL)
    {
        goto done;
    }
    dst->candidates = src->candidates;
    dst->candidates->positive = src->candidates->positive ? FALSE : TRUE;
    src->candidates = NULL;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "NotFilterResults: End" );
}

/* OrFilterResults: ORs src filter processing result (computeResult and candidates list) with the dst filter
 * processing result (computeResult and candidates list). For an AND top level filter, this function is expected to be
 * called first for the "positive" filter components, and then for the "negative" filter components.
 *
 * Cases that are handled are:
 *     - both src and dst are positive candidates list. => Intersection needs to be done.
 *     - both src and dst are negative candidates list. => Union is needed.
 *     - dst is positive candidates list and src is negative candidates list. => Subtraction is needed.
 *
 * RFC 4511, Section 4.5.1.7:
 * A filter of the "or" choice is FALSE if all the filters in the SET OF evaluate to FALSE, TRUE if at least one filter
 * is TRUE, and Undefined otherwise.
 */
void
OrFilterResults(
    VDIR_FILTER * src,
    VDIR_FILTER * dst)
{
    VmDirLog( LDAP_DEBUG_TRACE, "OrFilterResults: Begin" );

    // First process computeResults, and then candidates lists.

    switch (src->computeResult)
    {
        case FILTER_RES_NORMAL:
            switch (dst->computeResult)
            {
                case FILTER_RES_NORMAL:
                    break; // normal processing of candidates lists is required.
                case FILTER_RES_TRUE:
                    goto done;
                case FILTER_RES_FALSE:
                case FILTER_RES_UNDEFINED:
                    dst->computeResult = FILTER_RES_NORMAL;
                    break;
            }
            break;
        case FILTER_RES_TRUE:
            DeleteCandidates( &(dst->candidates) );
            dst->computeResult = FILTER_RES_TRUE;
            goto done;
        case FILTER_RES_FALSE:
            switch (dst->computeResult)
            {
                case FILTER_RES_NORMAL:
                case FILTER_RES_TRUE:
                case FILTER_RES_FALSE:
                case FILTER_RES_UNDEFINED:
                    goto done;
            }
        case FILTER_RES_UNDEFINED:
            switch (dst->computeResult)
            {
                case FILTER_RES_NORMAL:
                case FILTER_RES_TRUE:
                    goto done;
                case FILTER_RES_FALSE:
                    dst->computeResult = FILTER_RES_UNDEFINED;
                    goto done;
                case FILTER_RES_UNDEFINED:
                    goto done;
            }
            break;
    }
    // Handling a non-indexed attribute or a -ve candidates list.
    if (src->candidates == NULL || src->candidates->positive == FALSE)
    {
        DeleteCandidates( &(dst->candidates) );
        dst->computeResult = FILTER_RES_TRUE;
        goto done;
    }
    // 1st time initialization of the top of the OR filter.
    if (dst->candidates == NULL)
    {
        dst->candidates = src->candidates;
        src->candidates = NULL;
        goto done;
    }
    if (dst->candidates->positive && src->candidates->positive)
    {  // Do an OR of +ve candidate lists
        UnionCandidates( &(src->candidates), &(dst->candidates) );
    } else
    {
        assert( FALSE );
    }

done:
    VmDirLog( LDAP_DEBUG_TRACE, "OrFilterResults: End" );
}

/* IntersectCandidates: Intersect 2 +ve candidates lists.
 *
 */
static void
IntersectCandidates(
    VDIR_CANDIDATES *  srcCandidates,
    VDIR_CANDIDATES ** dstCandidates)
{
    DWORD        dwError = 0;
    VDIR_CANDIDATES * resCandidates = NULL;
    int          i = 0;
    int          j = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "IntersectCandidates: Begin" );

    assert( srcCandidates != NULL && dstCandidates != NULL && *dstCandidates != NULL );
    assert( srcCandidates->positive == (*dstCandidates)->positive );

    if ((*dstCandidates)->size == 0)
    {
        goto done;
    }
    if (srcCandidates->size == 0)
    {
        VMDIR_SAFE_FREE_MEMORY( (*dstCandidates)->eIds );
        (*dstCandidates)->eIds = NULL;
        (*dstCandidates)->size = 0;
        goto done;
    }
    if ((*dstCandidates)->eIdsSorted == FALSE)
    {
        qsort ( (*dstCandidates)->eIds, (*dstCandidates)->size, sizeof( ENTRYID ), _VmDirCompareEntryIds );
        (*dstCandidates)->eIdsSorted = TRUE;
    }
    if (srcCandidates->eIdsSorted == FALSE)
    {
        qsort ( srcCandidates->eIds, srcCandidates->size, sizeof( ENTRYID ), _VmDirCompareEntryIds );
        srcCandidates->eIdsSorted = TRUE;
    }

    resCandidates = NewCandidates( (*dstCandidates)->size <= srcCandidates->size ?
                                   (*dstCandidates)->size : srcCandidates->size, srcCandidates->positive );

    for (i = 0, j = 0; (i < srcCandidates->size) && (j < (*dstCandidates)->size); )
    {
        if (srcCandidates->eIds[i] < (*dstCandidates)->eIds[j])
        {
            i++;
            continue;
        }
        if ((*dstCandidates)->eIds[j] < srcCandidates->eIds[i])
        {
            j++;
            continue;
        }
        dwError = VmDirAddToCandidates( resCandidates, (*dstCandidates)->eIds[j]);
        BAIL_ON_VMDIR_ERROR(dwError);
        i++;
        j++;
    }
    resCandidates->eIdsSorted = TRUE;

    DeleteCandidates( dstCandidates );
    *dstCandidates = resCandidates;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "IntersectCandidates: End" );

    return;

error:

    DeleteCandidates( dstCandidates );
    resCandidates->size = 0;
    *dstCandidates = resCandidates;

    goto done;
}

/* SubtractCandidates: Subtract a -ve candidates list (srcCandidates) from a +ve candidates list (dstCandidates).
 *
 */
static void
SubtractCandidates(
    VDIR_CANDIDATES *  srcCandidates,
    VDIR_CANDIDATES ** dstCandidates)
{
    DWORD        dwError = 0;
    VDIR_CANDIDATES * resCandidates = NULL;
    int          i = 0;
    int          j = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "SubtractCandidates: Begin" );

    assert( srcCandidates != NULL && dstCandidates != NULL && *dstCandidates != NULL );
    assert( srcCandidates->positive != (*dstCandidates)->positive );

    if ((*dstCandidates)->size == 0 || srcCandidates->size == 0)
    {
        goto done;
    }
    if ((*dstCandidates)->eIdsSorted == FALSE)
    {
        qsort ( (*dstCandidates)->eIds, (*dstCandidates)->size, sizeof( ENTRYID ), _VmDirCompareEntryIds );
        (*dstCandidates)->eIdsSorted = TRUE;
    }
    if (srcCandidates->eIdsSorted == FALSE)
    {
        qsort ( srcCandidates->eIds, srcCandidates->size, sizeof( ENTRYID ), _VmDirCompareEntryIds );
        srcCandidates->eIdsSorted = TRUE;
    }

    resCandidates = NewCandidates( (*dstCandidates)->size, (*dstCandidates)->positive );

    for (i = 0, j = 0; (i < srcCandidates->size) && (j < (*dstCandidates)->size); )
    {
        if (srcCandidates->eIds[i] < (*dstCandidates)->eIds[j])
        {
            i++;
            continue;
        }
        if ((*dstCandidates)->eIds[j] < srcCandidates->eIds[i])
        {
            dwError = VmDirAddToCandidates( resCandidates, (*dstCandidates)->eIds[j]);
            BAIL_ON_VMDIR_ERROR(dwError);
            j++;
            continue;
        }
        i++;
        j++;
    }
    for (; j < (*dstCandidates)->size; j++)
    {
        dwError = VmDirAddToCandidates( resCandidates, (*dstCandidates)->eIds[j]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    resCandidates->eIdsSorted = TRUE;

    DeleteCandidates( dstCandidates );
    *dstCandidates = resCandidates;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "SubtractCandidates: End" );

    return;

error:

    DeleteCandidates( dstCandidates );
    resCandidates->size = 0;
    *dstCandidates = resCandidates;

    goto done;
}

/* UnionCandidates: Take a union of 2 candidates lists.
 *
 */
static void
UnionCandidates(
    VDIR_CANDIDATES ** srcCandidates,
    VDIR_CANDIDATES ** dstCandidates)
{
    DWORD        dwError = 0;
    VDIR_CANDIDATES * resCandidates = NULL;
    int          i = 0;
    int          j = 0;

    VmDirLog( LDAP_DEBUG_TRACE, "UnionCandidates: Begin" );

    assert( srcCandidates != NULL && *srcCandidates != NULL && dstCandidates != NULL && *dstCandidates != NULL );
    assert( (*srcCandidates)->positive == (*dstCandidates)->positive );

    if ((*srcCandidates)->size == 0)
    {
        goto done;
    }
    if ((*dstCandidates)->size == 0)
    {
        DeleteCandidates( dstCandidates );
        *dstCandidates = *srcCandidates;
        *srcCandidates = NULL;
        goto done;
    }
    if ((*dstCandidates)->eIdsSorted == FALSE)
    {
        qsort ( (*dstCandidates)->eIds, (*dstCandidates)->size, sizeof( ENTRYID ), _VmDirCompareEntryIds );
        (*dstCandidates)->eIdsSorted = TRUE;
    }
    if ((*srcCandidates)->eIdsSorted == FALSE)
    {
        qsort ( (*srcCandidates)->eIds, (*srcCandidates)->size, sizeof( ENTRYID ), _VmDirCompareEntryIds );
        (*srcCandidates)->eIdsSorted = TRUE;
    }

    resCandidates = NewCandidates( (*srcCandidates)->size + (*dstCandidates)->size, (*srcCandidates)->positive );

    for (i = 0, j = 0; (i < (*srcCandidates)->size) && (j < (*dstCandidates)->size); )
    {
        if ((*srcCandidates)->eIds[i] < (*dstCandidates)->eIds[j])
        {
            dwError = VmDirAddToCandidates( resCandidates, (*srcCandidates)->eIds[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
            i++;
            continue;
        }
        if ((*dstCandidates)->eIds[j] < (*srcCandidates)->eIds[i])
        {
            dwError = VmDirAddToCandidates( resCandidates, (*dstCandidates)->eIds[j]);
            BAIL_ON_VMDIR_ERROR(dwError);
            j++;
            continue;
        }
        dwError = VmDirAddToCandidates( resCandidates, (*dstCandidates)->eIds[j]);
        BAIL_ON_VMDIR_ERROR(dwError);
        i++;
        j++;
    }
    for (; i < (*srcCandidates)->size; i++)
    {
        dwError = VmDirAddToCandidates( resCandidates, (*srcCandidates)->eIds[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    for (; j < (*dstCandidates)->size; j++)
    {
        dwError = VmDirAddToCandidates( resCandidates, (*dstCandidates)->eIds[j]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    resCandidates->eIdsSorted = TRUE;

    DeleteCandidates( dstCandidates );
    *dstCandidates = resCandidates;

done:
    VmDirLog( LDAP_DEBUG_TRACE, "UnionCandidates: End" );

    return;

error:

    DeleteCandidates( dstCandidates );
    resCandidates->size = 0;
    *dstCandidates = resCandidates;

    goto done;
}

/*
 * double the size of candidate eid buffer
 */
static
DWORD
VmDirReallocCandidates(
    PVDIR_CANDIDATES    pCands
    )
{
    DWORD   dwError = 0;
    int     iOldSize = pCands->max * sizeof(ENTRYID);
    int     iNewSize = iOldSize * 2;

    dwError = VmDirReallocateMemoryWithInit(pCands->eIds, (PVOID*)&pCands->eIds, iNewSize, iOldSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    pCands->max *= 2;

error:

    return dwError;
}

static int
_VmDirCompareEntryIds(
    const void * pEID1,
    const void * pEID2)
{
    if ( *(ENTRYID *)pEID1 < *(ENTRYID *)pEID2 )
    {
        return -1;
    }
    else if ( *(ENTRYID *)pEID1 == *(ENTRYID *)pEID2 )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
