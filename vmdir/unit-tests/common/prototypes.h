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
#ifndef COMMON_H_
#define COMMON_H_

///////////////////////
// mergesort.c
///////////////////////
VOID
VmDirMergeSort_OneNodeInput(
    VOID    **state
    );

VOID
VmDirMergeSort_TwoNodesInput(
    VOID    **state
    );

VOID
VmDirMergeSort_ThreeNodesInput(
    VOID    **state
    );

VOID
VmDirMergeSort_AscendingOrder(
    VOID    **state
    );

VOID
VmDirMergeSort_DescendingOrder(
    VOID    **state
    );

//////////////////
// linkedlist.c
/////////////////
VOID
VmDirLinkedListAppendListToTail_ValidInput(
    VOID    **state
    );

VOID
VmDirLinkedListAppendListToTail_InvalidInput(
    VOID    **state
    );
//////////////////////
// sortedlinkedlist.c
//////////////////////
VOID
VmDirSortedLinkedListInsert_AscendingOneElement(
    VOID    **state
    );

VOID
VmDirSortedLinkedListInsert_AscendingTwoElements(
    VOID    **state
    );

VOID
VmDirSortedLinkedListInsert_AscendingThreeElements(
    VOID    **state
    );

VOID
VmDirSortedLinkedListInsert_Ascending(
    VOID    **state
    );

VOID
VmDirSortedLinkedListInsert_Descending(
    VOID    **state
    );
#endif
