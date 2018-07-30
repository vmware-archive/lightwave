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
VmDirWriteMetaDataAttribute(
   PVDIR_OPERATION              pOperation,
   PVDIR_ATTRIBUTE              pAttr,
   int                          numAttrMetaData,
   PATTRIBUTE_META_DATA_NODE    pAttrMetaData,
   BerElement *                 ber,
   PBOOLEAN                     pbNonTrivialAttrsInReplScope,
   PSTR*                        ppszErrorMsg
   )
{
    DWORD                dwError = 0;
    VDIR_BERVALUE        attrMetaData =
                         {{ATTR_ATTR_META_DATA_LEN, ATTR_ATTR_META_DATA}, 0, 0, NULL};
    int                  i = 0;
    VDIR_BERVALUE        berVal = VDIR_BERVALUE_INIT;
    char                 pszAttrMetaDataVal[256 + VMDIR_MAX_ATTR_META_DATA_LEN] = {'\0'};
    PSTR                 pszLocalErrorMsg = NULL;
    PSZ_METADATA_BUF     pszMetaData = {'\0'};

    if (!pOperation ||
        !pAttrMetaData ||
        !ber ||
        !pbNonTrivialAttrsInReplScope)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    *pbNonTrivialAttrsInReplScope = FALSE;

    if (ber_printf(ber, "{O[", &(attrMetaData)) == -1)
    {
        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "%s: ber_printf (to print attribute name ...) failed",
                __FUNCTION__);
        dwError = LDAP_OTHER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                dwError, (pszLocalErrorMsg), "Encoding attribute type failed.");
    }

    for ( ; pAttr != NULL; pAttr = pAttr->next)
    {
        /* By this time, we have already filtered out attributes that should be
         * send back to replication consumer in prior WriteAttributes -> IsAttrInReplScope call.
         * They contain proper pAttr value.
         */
        if (!IS_VMDIR_ATTRIBUTE_NOT_IN_SCOPE(pAttr->pMetaData))
        {
            memset(&pszMetaData[0], '\0', VMDIR_MAX_ATTR_META_DATA_LEN);
            dwError = VmDirMetaDataSerialize(pAttr->pMetaData, pszMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);

            VmDirStringPrintFA(
                    pszAttrMetaDataVal,
                    sizeof(pszAttrMetaDataVal),
                    "%.256s:%s",
                    pAttr->type.lberbv.bv_val,
                    pszMetaData);

            berVal.lberbv.bv_val = pszAttrMetaDataVal;
            berVal.lberbv.bv_len = VmDirStringLenA(pszAttrMetaDataVal);
            if (VmDirStringCompareA(pAttr->type.lberbv_val, ATTR_MODIFYTIMESTAMP, FALSE) != 0 &&
                VmDirStringCompareA(pAttr->type.lberbv_val, ATTR_MODIFIERS_NAME, FALSE) != 0  &&
                VmDirStringCompareA(pAttr->type.lberbv_val, ATTR_USN_CHANGED, FALSE) != 0     &&
                VmDirStringCompareA(pAttr->type.lberbv_val, ATTR_OBJECT_GUID, FALSE) != 0)
            {
                /* To prevent endless replication ping pong, supplier should send result only if
                 * there are changes to attribute other than ATTR_USN_CHANGED, ATTR_MODIFYTIMESTAMP,
                 * ATTR_MODIFIERS_NAME and ATTR_OBJECT_GUID.
                 */
                *pbNonTrivialAttrsInReplScope = TRUE;
            }
            if (ber_printf( ber, "O", &berVal ) == -1)
            {
                VMDIR_LOG_VERBOSE(
                        VMDIR_LOG_MASK_ALL,
                        "%s: ber_printf (to print an attribute value ...) failed.",
                        __FUNCTION__);
                dwError = LDAP_OTHER;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        dwError, (pszLocalErrorMsg), "Encoding an attribute value failed.");
            }
        }
    }

    // include attrMetaData for the deleted attributes
    for (i = 0; i<numAttrMetaData; i++)
    {
        if (pAttrMetaData[i].pMetaData)
        {
            BOOLEAN                 bSendAttrMetaData = TRUE;
            PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;

            if (pOperation->syncReqCtrl != NULL) //Replication
            {
                dwError = VmDirIsAttrInScope(
                        pOperation,
                        NULL,
                        pAttrMetaData[i].pMetaData,
                        0,
                        &bSendAttrMetaData);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (bSendAttrMetaData)
            {
                pATDesc = VmDirSchemaAttrIdToDesc(pOperation->pSchemaCtx, pAttrMetaData[i].attrID);
                if (pATDesc == NULL)
                {
                    VMDIR_LOG_VERBOSE(
                            VMDIR_LOG_MASK_ALL,
                            "%s: VmDirSchemaAttrIdToDesc failed for attribute id: %d.",
                            __FUNCTION__,
                            pAttrMetaData[i].attrID);
                    dwError = LDAP_OTHER;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(
                            dwError,
                            (pszLocalErrorMsg),
                            "%s: VmDirSchemaAttrIdToDesc failed.",
                            __FUNCTION__);
                }

                memset(&pszMetaData[0], '\0', VMDIR_MAX_ATTR_META_DATA_LEN);
                dwError = VmDirMetaDataSerialize(pAttrMetaData[i].pMetaData, pszMetaData);
                BAIL_ON_VMDIR_ERROR(dwError);

                VmDirStringPrintFA(
                        pszAttrMetaDataVal,
                        sizeof(pszAttrMetaDataVal),
                        "%.256s:%s",
                        pATDesc->pszName,
                        pszMetaData);

                berVal.lberbv.bv_val = pszAttrMetaDataVal;
                berVal.lberbv.bv_len = VmDirStringLenA(pszAttrMetaDataVal);

                *pbNonTrivialAttrsInReplScope = TRUE;

                if (ber_printf(ber, "O", &berVal) == -1)
                {
                    VMDIR_LOG_VERBOSE(
                            VMDIR_LOG_MASK_ALL,
                            "%s: ber_printf (to print an attribute value ...) failed.",
                            __FUNCTION__);
                    dwError = LDAP_OTHER;
                    BAIL_ON_VMDIR_ERROR_WITH_MSG(
                            dwError, (pszLocalErrorMsg),"Encoding an attribute value failed.");
                }
            }
        }
    }

    if (ber_printf( ber, "]N}") == -1)
    {
        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "%s: ber_printf (to terminate an attribute's values ...) failed",
                __FUNCTION__);
        dwError = LDAP_OTHER;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(
                dwError, (pszLocalErrorMsg), "Encoding terminating an attribute type failed.");
    }

cleanup:

    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }

    return(dwError);

error:

    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s: failed (%u)(%s)",
            __FUNCTION__,
            dwError,
            VDIR_SAFE_STRING(pszLocalErrorMsg));
    goto cleanup;
}

DWORD
VmDirWriteValueMetaDataAttribute(
   PVDIR_OPERATION     pOperation,
   PDEQUE              pAllValueMetaDataQueue,
   BerElement *        pBer,
   PSTR*               ppszErrorMsg,
   PBOOLEAN            pNonTrivialAttrs
   )
{
    DWORD                              dwError = 0;
    VDIR_BERVALUE                      attrValueMetaData =
                                       {{ATTR_ATTR_VALUE_META_DATA_LEN, ATTR_ATTR_VALUE_META_DATA},
                                         0, 0, NULL};
    VDIR_BERVALUE                      bervValueMetaData = VDIR_BERVALUE_INIT;
    PSTR                               pszLocalErrorMsg = NULL;
    DEQUE                              valueMetaDataToSendQueue = {0};
    PDEQUE                             pValueMetaDataQueue = NULL;
    PVMDIR_VALUE_ATTRIBUTE_METADATA    pValueMetaData = NULL;

    if (!pOperation ||
        !pAllValueMetaDataQueue ||
        !pBer ||
        !pNonTrivialAttrs)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pOperation->syncReqCtrl)
    {
        dwError = VmDirIsAttrValueInScope(
                pOperation, pAllValueMetaDataQueue, &valueMetaDataToSendQueue);
        BAIL_ON_VMDIR_ERROR(dwError);

        pValueMetaDataQueue = &valueMetaDataToSendQueue;
    }
    else
    {
        pValueMetaDataQueue = pAllValueMetaDataQueue;
    }

    if (!dequeIsEmpty(pValueMetaDataQueue))
    {
        *pNonTrivialAttrs = TRUE;

        if (ber_printf(pBer, "{O[", &(attrValueMetaData)) == -1)
        {
            VMDIR_LOG_VERBOSE(
                    VMDIR_LOG_MASK_ALL,
                    "%s: ber_printf (to print attribute name ...) failed",
                    __FUNCTION__);

            dwError = LDAP_OTHER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(
                    dwError, (pszLocalErrorMsg), "Encoding attribute type failed.");
        }

        while (!dequeIsEmpty(pValueMetaDataQueue))
        {
            dequePopLeft(pValueMetaDataQueue, (PVOID*)&pValueMetaData);

            dwError = VmDirValueMetaDataSerialize(pValueMetaData, &bervValueMetaData);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (ber_printf(pBer, "O", &(bervValueMetaData)) == -1)
            {
                VMDIR_LOG_VERBOSE(
                        VMDIR_LOG_MASK_ALL,
                        "%s: ber_printf (to print attribute name ...) failed",
                        __FUNCTION__);

                dwError = LDAP_OTHER;
                BAIL_ON_VMDIR_ERROR_WITH_MSG(
                        dwError, (pszLocalErrorMsg), "Encoding an attribute value failed.");
            }

            VmDirFreeBervalContent(&bervValueMetaData);
            VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
        }

        if (ber_printf(pBer, "]N}") == -1)
        {
            VMDIR_LOG_VERBOSE(
                    VMDIR_LOG_MASK_ALL,
                    "%s: ber_printf (to print attribute name ...) failed",
                    __FUNCTION__);

            dwError = LDAP_OTHER;
            BAIL_ON_VMDIR_ERROR_WITH_MSG(
                    dwError, (pszLocalErrorMsg), "Encoding terminating an attribute type failed.");
        }
    }

cleanup:
    if (ppszErrorMsg)
    {
        *ppszErrorMsg = pszLocalErrorMsg;
    }
    else
    {
        VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "failed (%u)(%s)",
            dwError,
            VDIR_SAFE_STRING(pszLocalErrorMsg));

    VMDIR_SAFE_FREE_VALUE_METADATA(pValueMetaData);
    goto cleanup;
}
