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

static
USN
VmDirBackendGetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx
    );

static
VOID
VmDirBackendSetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN                 maxOriginatingUSN
    );


/*
 * Called during server start up to configure backends(s)
 * Currently, we only support ONE backend.
 */
DWORD
VmDirBackendConfig(
    VOID)
{
    DWORD                   dwError = 0;

    gVdirBEGlobals.pszBERootDN = "";
    gVdirBEGlobals.pBE = NULL;

    gVdirBEGlobals.pBE = VmDirMDBBEInterface();

    gVdirBEGlobals.pBE->pfnBEGetMaxOriginatingUSN = VmDirBackendGetMaxOriginatingUSN;

    gVdirBEGlobals.pBE->pfnBESetMaxOriginatingUSN = VmDirBackendSetMaxOriginatingUSN;

    return dwError;
}

/*
 * Select backend based on entry DN
 */
PVDIR_BACKEND_INTERFACE
VmDirBackendSelect(
    PCSTR   pszDN)
{
    // only have one backend for now
    return gVdirBEGlobals.pBE;
}

/*
 * Free PVDIR_BACKEND_CTX and it's content.
 */
VOID
VmDirBackendCtxFree(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    if ( pBECtx )
    {
        VmDirBackendCtxContentFree( pBECtx );
        VMDIR_SAFE_FREE_MEMORY( pBECtx );
    }

    return;
}

/*
 * Free backend specific resources
 */
VOID
VmDirBackendCtxContentFree(
    PVDIR_BACKEND_CTX   pBECtx)
{
    if ( pBECtx )
    {
        if (pBECtx->pBE && pBECtx->pBEPrivate)
        {
            pBECtx->pBE->pfnBETxnAbort(pBECtx);
        }

        if (pBECtx->pBE && pBECtx->wTxnUSN > 0)
        {
            pBECtx->wTxnUSN = 0;
        }

        VMDIR_SAFE_FREE_MEMORY(pBECtx->pszBEErrorMsg);
    }

    return;
}

DWORD
VmDirBackendUniqKeyGetValue(
    PCSTR       pKey,
    PSTR*       ppValue
    )
{
    DWORD               dwError = 0;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PSTR                pValue = NULL;

    if (!pKey || !ppValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    beCtx.pBE = VmDirBackendSelect(NULL);
    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_READ);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = beCtx.pBE->pfnBEUniqKeyGetValue(
                            &beCtx, pKey, &pValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppValue = pValue;
    pValue = NULL;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnCommit(&beCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pValue);
    VmDirBackendCtxContentFree(&beCtx);

    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
                    "%s error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

DWORD
VmDirBackendUniqKeySetValue(
    PCSTR       pKey,
    PCSTR       pValue,
    BOOLEAN     bForce
    )
{
    DWORD               dwError = 0;
    VDIR_BACKEND_CTX    beCtx = {0};
    BOOLEAN             bHasTxn = FALSE;
    PSTR                pLocalValue = NULL;

    if (!pKey || !pValue)
    {
        dwError = VMDIR_ERROR_GENERIC;
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    beCtx.pBE = VmDirBackendSelect(NULL);
    dwError = beCtx.pBE->pfnBETxnBegin(&beCtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    if (!bForce)
    {
        // Maybe MDB has option to force set already?
        // for now, query to see if key exists.
        dwError = beCtx.pBE->pfnBEUniqKeyGetValue(
                                &beCtx, pKey, &pLocalValue);
        if (dwError == 0)
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_TYPE_OR_VALUE_EXISTS);
        }

        if (dwError == VMDIR_ERROR_NOT_FOUND)
        {
            dwError = 0;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = beCtx.pBE->pfnBEUniqKeySetValue(
                            &beCtx, pKey, pValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    beCtx.pBE->pfnBETxnCommit(&beCtx);
    bHasTxn = FALSE;

cleanup:
    if (bHasTxn)
    {
        beCtx.pBE->pfnBETxnAbort(&beCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pLocalValue);
    VmDirBackendCtxContentFree(&beCtx);

    return dwError;

error:
    VMDIR_LOG_INFO( LDAP_DEBUG_BACKEND,
                    "%s error (%d)", __FUNCTION__, dwError );
    goto cleanup;
}

static
VOID
VmDirBackendSetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx,
    USN                 maxOriginatingUSN
    )
{
    LwInterlockedExchange64(&gVdirBEGlobals.usnMaxOriginating, maxOriginatingUSN);

    return;
}

static
USN
VmDirBackendGetMaxOriginatingUSN(
    PVDIR_BACKEND_CTX   pBECtx
    )
{
    USN maxOriginatingUSN = 0;

    // should use atomic read LW function when available.
    maxOriginatingUSN = gVdirBEGlobals.usnMaxOriginating;

    return maxOriginatingUSN;
}

