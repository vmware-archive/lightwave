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

static
BOOLEAN
LwCARestAuthIsOpenAPI(
    PCSTR           pcszURI
    );


DWORD
LwCARestAuth(
    PLWCA_REST_OPERATION    pRestOp,
    PLWCA_REQ_CONTEXT       *ppReqCtx,
    PBOOLEAN                pbAuthenticated
    )
{
    DWORD                   dwError = 0;
    PLWCA_REQ_CONTEXT       pReqCtx = NULL;
    BOOLEAN                 bAuthenticated = FALSE;

    if (!pRestOp || !ppReqCtx || !pbAuthenticated)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    if (LwCARestAuthIsOpenAPI(pRestOp->pszURI))
    {
        bAuthenticated = TRUE;
        goto ret;
    }

    dwError = LwCAOIDCTokenAuthenticate(
                        pRestOp->pszAuth,
                        pRestOp->pszMethod,
                        pRestOp->pszContentType,
                        pRestOp->pszDate,
                        pRestOp->pszBody,
                        pRestOp->pszURI,
                        &bAuthenticated,
                        &pReqCtx);
    BAIL_ON_LWCA_ERROR(dwError);

    if (!bAuthenticated)
    {
        LWCA_LOG_ERROR("[%s:%d] Failed to authenticate HTTP request!", __FUNCTION__, __LINE__);
    }
    else
    {
        LWCA_LOG_INFO(
                "[%s:%d] Authenticated HTTP request. UPN (%s)",
                __FUNCTION__,
                __LINE__,
                pReqCtx->pszBindUPN);
    }


ret:

    *pbAuthenticated = bAuthenticated;
    *ppReqCtx = pReqCtx;

cleanup:

    return dwError;

error:

    LwCARequestContextFree(pReqCtx);
    if (ppReqCtx)
    {
        *ppReqCtx = NULL;
    }
    if (pbAuthenticated)
    {
        *pbAuthenticated = FALSE;
    }

    goto cleanup;
}


static
BOOLEAN
LwCARestAuthIsOpenAPI(
    PCSTR           pcszURI
    )
{
    return (!LwCAStringCompareA(LWCA_REST_OPENAPI_VERSION, pcszURI, FALSE));
}
