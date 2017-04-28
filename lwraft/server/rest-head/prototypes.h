/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

// accesstoken.c
DWORD
VmDirRESTAccessTokenInit(
    PVDIR_REST_ACCESS_TOKEN*    ppAccessToken
    );

DWORD
VmDirRESTAccessTokenParse(
    PVDIR_REST_ACCESS_TOKEN pAccessToken,
    PSTR                    pszAuthData
    );

VOID
VmDirFreeRESTAccessToken(
    PVDIR_REST_ACCESS_TOKEN pAccessToken
    );

// auth.c
DWORD
VmDirRESTAuth(
    PVDIR_REST_OPERATION    pRestOp
    );

DWORD
VmDirRESTAuthBasic(
    PVDIR_REST_OPERATION    pRestOp,
    PVDIR_OPERATION         pBindOp
    );

DWORD
VmDirRESTAuthToken(
    PVDIR_REST_OPERATION    pRestOp,
    PVDIR_OPERATION         pBindOp
    );

// decode.c
DWORD
VmDirRESTDecodeAttributeNoAlloc(
    json_t*         pjInput,
    PVDIR_ATTRIBUTE pAttr
    );

DWORD
VmDirRESTDecodeAttribute(
    json_t*             pjInput,
    PVDIR_ATTRIBUTE*    ppAttr
    );

DWORD
VmDirRESTDecodeEntry(
    json_t*         pjInput,
    PVDIR_ENTRY*    ppEntry
    );

DWORD
VmDirRESTDecodeEntryMods(
    json_t*             pjInput,
    PVDIR_MODIFICATION* ppMods,
    DWORD*              pdwNumMods
    );

DWORD
VmDirRESTDecodeObjectPathToDN(
    PCSTR   pszObjPath,
    PCSTR   pszTenant,
    PSTR*   ppszDN
    );

DWORD
VmDirRESTDecodeObjectFilter(
    PVDIR_FILTER    pFilter,
    PCSTR           pszTenant
    );

DWORD
VmDirRESTDecodeObject(
    json_t*         pjInput,
    PCSTR           pszObjPath,
    PCSTR           pszTenant,
    PVDIR_ENTRY*    ppObj
    );

DWORD
VmDirRESTDecodeObjectMods(
    json_t*             pjInput,
    PCSTR               pszTenant,
    PVDIR_MODIFICATION* ppMods,
    DWORD*              pdwNumMods
    );

// encode.c
DWORD
VmDirRESTEncodeAttribute(
    PVDIR_ATTRIBUTE pAttr,
    json_t**        ppjOutput
    );

DWORD
VmDirRESTEncodeEntry(
    PVDIR_ENTRY     pEntry,
    PVDIR_BERVALUE  pbvAttrs,
    json_t**        ppjOutput
    );

DWORD
VmDirRESTEncodeEntryArray(
    PVDIR_ENTRY_ARRAY   pEntryArray,
    PVDIR_BERVALUE      pbvAttrs,
    json_t**            ppjOutput
    );

DWORD
VmDirRESTEncodeDNToObjectPath(
    PCSTR   pszDN,
    PCSTR   pszTenant,
    PSTR*   ppszObjPath
    );

DWORD
VmDirRESTEncodeObjectAttribute(
    PVDIR_ATTRIBUTE pObjAttr,
    PCSTR           pszTenant,
    json_t**        ppjOutput
    );

DWORD
VmDirRESTEncodeObject(
    PVDIR_ENTRY     pObj,
    PVDIR_BERVALUE  pbvAttrs,
    PCSTR           pszTenant,
    json_t**        ppjOutput
    );

DWORD
VmDirRESTEncodeObjectArray(
    PVDIR_ENTRY_ARRAY   pObjArray,
    PVDIR_BERVALUE      pbvAttrs,
    PCSTR               pszTenant,
    json_t**            ppjOutput,
    size_t*             pSkipped
    );

// httperror.c
PVDIR_HTTP_ERROR
VmDirRESTGetHttpError(
    int httpStatus
    );

// ldapapi.c
DWORD
VmDirRESTGetLdapModule(
    PREST_MODULE*   ppRestModule
    );

DWORD
VmDirRESTLdapAdd(
    void*   pIn,
    void**  ppOut
    );

DWORD
VmDirRESTLdapSearch(
    void*   pIn,
    void**  ppOut
    );

DWORD
VmDirRESTLdapModify(
    void*   pIn,
    void**  ppOut
    );

DWORD
VmDirRESTLdapDelete(
    void*   pIn,
    void**  ppOut
    );

DWORD
VmDirRESTLdapSetResult(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_LDAP_RESULT   pLdapRslt,
    DWORD               dwErr,
    PSTR                pszErrMsg
    );

// ldapcontro.c
DWORD
VmDirAddCondWriteCtrl(
    PVDIR_OPERATION pOp,
    PCSTR           pszCondWriteFilter
    );

DWORD
VmDirRESTLdapGetHttpError(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR*               ppszHttpStatus,
    PSTR*               ppszHttpReason
    );

// libmain.c
DWORD
VmDirRESTRequestHandler(
    PVMREST_HANDLE  pRESTHandle,
    PREST_REQUEST   pRequest,
    PREST_RESPONSE* ppResponse,
    uint32_t        paramsCount
    );

// objectapi.c
DWORD
VmDirRESTGetObjectModule(
    PREST_MODULE*   ppRestModule
    );

DWORD
VmDirRESTObjectPut(
    void*   pIn,
    void**  ppOut
    );

DWORD
VmDirRESTObjectGet(
    void*   pIn,
    void**  ppOut
    );

DWORD
VmDirRESTObjectPatch(
    void*   pIn,
    void**  ppOut
    );

DWORD
VmDirRESTObjectDelete(
    void*   pIn,
    void**  ppOut
    );

// operation.c
DWORD
VmDirRESTOperationCreate(
    PVDIR_REST_OPERATION*   ppRestOp
    );

DWORD
VmDirRESTOperationReadRequest(
    PVDIR_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_REQUEST           pRestReq,
    DWORD                   dwParamCount
    );

DWORD
VmDirRESTOperationWriteResponse(
    PVDIR_REST_OPERATION    pRestOp,
    PVMREST_HANDLE          pRESTHandle,
    PREST_RESPONSE*         ppResponse
    );

VOID
VmDirFreeRESTOperation(
    PVDIR_REST_OPERATION    pRestOp
    );

// param.c
DWORD
VmDirRESTGetStrParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    PSTR*                   ppszVal,
    BOOLEAN                 bRequired
    );

DWORD
VmDirRESTGetIntParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    int*                    piVal,
    BOOLEAN                 bRequired
    );

DWORD
VmDirRESTGetBoolParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    BOOLEAN*                pbVal,
    BOOLEAN                 bRequired
    );

DWORD
VmDirRESTGetStrListParam(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszKey,
    PVMDIR_STRING_LIST*     ppValList,
    BOOLEAN                 bRequired
    );

DWORD
VmDirRESTGetLdapSearchParams(
    PVDIR_REST_OPERATION    pRestOp,
    int*                    piScope,
    PVDIR_FILTER*           ppFilter,
    PVDIR_BERVALUE*         ppbvAttrs,
    PVDIR_LDAP_CONTROL*     ppPagedResultsCtrl
    );

DWORD
VmDirRESTGetObjectTenantParam(
    PVDIR_REST_OPERATION    pRestOp,
    PSTR*                   ppszTenant
    );

DWORD
VmDirRESTGetObjectGetParams(
    PVDIR_REST_OPERATION    pRestOp,
    PSTR*                   ppszTenant,
    int*                    piSearchScope,
    PVDIR_FILTER*           ppFilter,
    PVDIR_BERVALUE*         ppbvAttrs,
    PVDIR_LDAP_CONTROL*     ppPagedResultsCtrl
    );

DWORD
VmDirRESTRenameParamKey(
    PVDIR_REST_OPERATION    pRestOp,
    PCSTR                   pszOldKey,
    PCSTR                   pszNewKey
    );

DWORD
VmDirRESTFilterObjectToDN(
    PCSTR           pszTenant,
    PVDIR_FILTER    pObjectFilter,
    PVDIR_FILTER*   ppDNFilter
    );

// resource.c
PVDIR_REST_RESOURCE
VmDirRESTGetResource(
    PSTR    pszPath
    );

DWORD
VmDirRESTUnknownSetResult(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_LDAP_RESULT   pLdapRslt,
    DWORD               dwErr,
    PSTR                pszErrMsg
    );

DWORD
VmDirRESTUnknownGetHttpError(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR*               ppszHttpStatus,
    PSTR*               ppszHttpReason
    );

// result.c
DWORD
VmDirRESTResultCreate(
    PVDIR_REST_RESULT*  ppRestRslt
    );

DWORD
VmDirRESTResultSetError(
    PVDIR_REST_RESULT   pRestRslt,
    int                 errCode,
    PSTR                pszErrMsg
    );

DWORD
VmDirRESTResultSetStrData(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    PSTR                pszVal
    );

DWORD
VmDirRESTResultSetIntData(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    int                 iVal
    );

DWORD
VmDirRESTResultSetObjData(
    PVDIR_REST_RESULT   pRestRslt,
    PSTR                pszKey,
    json_t*             pjVal
    );

DWORD
VmDirRESTResultToResponseBody(
    PVDIR_REST_RESULT   pRestRslt,
    PVDIR_REST_RESOURCE pResource,
    PSTR*               ppszBody
    );

VOID
VmDirFreeRESTResult(
    PVDIR_REST_RESULT   pRestRslt
    );
