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
 * Module Name: Directory ldap-head
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Function prototypes
 *
 */

#ifndef _PROTOTYPES_H_
#define _PROTOTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

// opstatistic.c
VOID
VmDirOPStatisticUpdate(
    ber_tag_t opTag,
    uint64_t iThisTimeInMilliSecs
    );

//replscope.c
DWORD
VmDirIsAttrInScope(
    PVDIR_OPERATION              pOperation,
    PCSTR                        pszAttrType,
    PVMDIR_ATTRIBUTE_METADATA    pAttrMetadata,
    USN                          priorSentUSNCreated,
    PBOOLEAN                     pbInScope
    );

DWORD
VmDirIsAttrValueInScope(
    PVDIR_OPERATION    pOperation,
    PDEQUE             pAllValueMetaDataQueue,
    PDEQUE             pValueMetaDataToSendQueue
    );

DWORD
VmDirIsUsnInScope(
    PVDIR_OPERATION     pOperation,
    PCSTR               pszAttrName,
    PCSTR               pszOrigInvocationId,
    USN                 origUsn,
    USN                 localUSN,
    USN                 priorSentUSNCreated,
    PBOOLEAN            pbIsUsnInScope
    );

//writeattributes.c
DWORD
VmDirWriteMetaDataAttribute(
   PVDIR_OPERATION              pOperation,
   PVDIR_ATTRIBUTE              pAttr,
   int                          numAttrMetaData,
   PATTRIBUTE_META_DATA_NODE    pAttrMetaData,
   BerElement *                 ber,
   PBOOLEAN                     pbNonTrivialAttrsInReplScope,
   PSTR*                        ppszErrorMsg
   );

DWORD
VmDirWriteValueMetaDataAttribute(
   PVDIR_OPERATION              pOperation,
   PDEQUE                       pAllValueMetaDataQueue,
   BerElement *                 pBer,
   PSTR*                        ppszErrorMsg,
   PBOOLEAN                     pNonTrivialAttrs
   );

#ifdef __cplusplus
}
#endif

#endif // _PROTOTYPES_H_

