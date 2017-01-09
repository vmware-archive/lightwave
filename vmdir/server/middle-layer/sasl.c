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
 * Module Name: Directory middle layer
 *
 * Filename: sasl.c
 *
 * Abstract:
 *
 * sasl bind support - GSSAPI/krb5
 *
 */

#include "includes.h"

static PSTR g_pszSASL2Path = NULL;

#define SASL_GSSAPI_MECH "GSSAPI"
#define SASL_SRP_MECH    "SRP"

// Only support GSSAPI currently.
// Note, we hard code this in DSE root entry query as well.
#define SUPPORTED_SASL_BIND_MECHANISM   \
        { SASL_GSSAPI_MECH, SASL_SRP_MECH, NULL }

// Default SASL security layer properties
static sasl_security_properties_t gSASLSecurityProps =
    {
        VMDIR_SF_INIT(.min_ssf,         VMDIR_SASL_MIN_SSF),
        VMDIR_SF_INIT(.max_ssf,         VMDIR_SASL_MAX_SSF),
        VMDIR_SF_INIT(.maxbufsize,      VMDIR_SASL_MAX_BUFFER_LEN),
        VMDIR_SF_INIT(.security_flags,  SASL_SEC_NOPLAINTEXT|SASL_SEC_NOANONYMOUS),
        VMDIR_SF_INIT(.property_names,  NULL),
        VMDIR_SF_INIT(.property_values, NULL)
    };

static
DWORD
_VmDirSASLGetCtxProps(
    PVDIR_SASL_BIND_INFO    pSaslBindInfo
    );

static
DWORD
_VmDirSASLToLDAPError(
    int     saslError
    );

static
int
_VmDirSASLLog(
    PVOID       pVoid,
    int         iPriority,
    PCSTR       pszMessage
    );

static
int
_VmDirSASL2PATH(
    void *          pContext,
    const char **   ppszPath
    );

static
int
VmDirSASL2PATH(
    PSTR *ppszPath
    );

static
PVOID
_VmDirSASLMutexNew(
    VOID
    );

static
VOID
_VmDirSASLMutexDispose(
    PVOID   pSASLMutex
    );

static
int
_VmDirSASLMutexLock(
    PVOID   pSASLMutex
    );

static
int
_VmDirSASLMutexUnlock(
    PVOID   pSASLMutex
    );

BOOLEAN
VmDirIsSupportedSASLMechanism(
    PCSTR   pszMech
    )
{
    static PCSTR    supportedSASLMech[] = SUPPORTED_SASL_BIND_MECHANISM;
    int             iCnt = 0;

    for (iCnt = 0; supportedSASLMech[iCnt] != NULL; iCnt++)
    {
        if (VmDirStringCompareA(pszMech, supportedSASLMech[iCnt], FALSE) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

DWORD
VmDirSASLInit(
    VOID
    )
{
    DWORD       dwError = 0;

    static sasl_callback_t saslServerCB[] =
    {
        { SASL_CB_LOG, (int (*)(void))_VmDirSASLLog, NULL },
        { SASL_CB_GETPATH, (int (*)(void)) _VmDirSASL2PATH, &g_pszSASL2Path },
        { SASL_CB_LIST_END, NULL, NULL }
    };

    dwError = VmDirSASL2PATH(&g_pszSASL2Path);
    BAIL_ON_VMDIR_ERROR(dwError);

    // provide SASL mutex functions
    sasl_set_mutex( _VmDirSASLMutexNew,
                    _VmDirSASLMutexLock,
                    _VmDirSASLMutexUnlock,
                    _VmDirSASLMutexDispose);

    dwError = sasl_server_init( saslServerCB, "vmdird");
    BAIL_ON_SASL_ERROR(dwError);

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(g_pszSASL2Path);
sasl_error:

    goto cleanup;
}

/*
 * per SASL session initialization.
 * We keep session data in Connection layer.
 */
DWORD
VmDirSASLSessionInit(
    PVDIR_SASL_BIND_INFO    pSaslBindInfo
    )
{
    DWORD               dwError = 0;
    sasl_conn_t*        pLocalSaslCtx = NULL;

    // start new sasl context
    dwError = sasl_server_new(  "ldap", // service protocol
                                NULL,   // need realm here?
                                NULL,
                                NULL,
                                NULL,
                                NULL,   // per session CB
                                0,      // default security flag
                                &pLocalSaslCtx);
    BAIL_ON_SASL_ERROR(dwError)


    // default security properties
    dwError = sasl_setprop( pLocalSaslCtx,
                            SASL_SEC_PROPS,
                            &gSASLSecurityProps );
    BAIL_ON_SASL_ERROR(dwError);

    pSaslBindInfo->pSaslCtx   = pLocalSaslCtx;

cleanup:

    return dwError;

sasl_error:

    if (pLocalSaslCtx)
    {
        sasl_dispose(&pLocalSaslCtx);
    }

    memset(pSaslBindInfo, 0, sizeof(*pSaslBindInfo));

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "SASLSessionInit: sasl error (%d)", dwError );

    dwError = _VmDirSASLToLDAPError(dwError);

    goto cleanup;
}

/*
 * Start a SASL authentication.
 *
 */
DWORD
VmDirSASLSessionStart(
    PVDIR_SASL_BIND_INFO    pSaslBindInfo,
    PVDIR_BIND_REQ          pBindReq,
    PVDIR_BERVALUE          pBervSaslReply,
    PVDIR_LDAP_RESULT       pResult
    )
{
    DWORD               dwError = 0;
    PVOID               pOutBlob = NULL;
    unsigned            iRespLen = 0;
    PCSTR               pszRespBlob = NULL;

    dwError = sasl_server_start(    pSaslBindInfo->pSaslCtx,
                                    pSaslBindInfo->bvMechnism.lberbv.bv_val,
                                    pBindReq->cred.lberbv.bv_val,
                                    (unsigned)pBindReq->cred.lberbv.bv_len,
                                    &pszRespBlob,
                                    &iRespLen);
    if (dwError == SASL_OK)
    {
        dwError = _VmDirSASLGetCtxProps(pSaslBindInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LDAP_SUCCESS;
        pSaslBindInfo->saslStatus = SASL_STATUS_DONE;
    }
    else if (dwError == SASL_CONTINUE)
    {
        pSaslBindInfo->saslStatus = SASL_STATUS_IN_PROGRESS;

        if (iRespLen > 0)
        {
            dwError = VmDirAllocateAndCopyMemory((PVOID)pszRespBlob, iRespLen, &pOutBlob);
            BAIL_ON_VMDIR_ERROR(dwError);

            pBervSaslReply->lberbv.bv_val = pOutBlob;
            pBervSaslReply->lberbv.bv_len = iRespLen;
            pBervSaslReply->bOwnBvVal = TRUE;
        }
        else
        {
            pBervSaslReply->lberbv.bv_len = 0;
            pBervSaslReply->lberbv.bv_val = "";
            pBervSaslReply->bOwnBvVal = FALSE;
        }

        dwError = LDAP_SUCCESS;
        pResult->errCode = LDAP_SASL_BIND_IN_PROGRESS;
        pResult->replyInfo.type = REP_SASL;
    }
    else
    {
        BAIL_ON_SASL_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pOutBlob);

    pBervSaslReply->lberbv.bv_val = NULL;
    pBervSaslReply->lberbv.bv_len = 0;
    pBervSaslReply->bOwnBvVal = FALSE;

    goto cleanup;

sasl_error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "SASLSessionStart: sasl error (%d)(%s)",
                     dwError,
                     VDIR_SAFE_STRING(sasl_errdetail(pSaslBindInfo->pSaslCtx)) );

    dwError = _VmDirSASLToLDAPError(dwError);

    goto error;
}

/*
 * Continue next SASL negotiation with client.
 */
DWORD
VmDirSASLSessionStep(
    PVDIR_SASL_BIND_INFO    pSaslBindInfo,
    PVDIR_BIND_REQ          pBindReq,
    PVDIR_BERVALUE          pBervSaslReply,
    PVDIR_LDAP_RESULT       pResult
    )
{
    DWORD               dwError = 0;
    PVOID               pOutBlob = NULL;
    unsigned            iRespLen = 0;
    PCSTR               pszRespBlob = NULL;

    dwError = sasl_server_step(     pSaslBindInfo->pSaslCtx,
                                    pBindReq->cred.lberbv.bv_val,
                                    (unsigned) pBindReq->cred.lberbv.bv_len,
                                    &pszRespBlob,
                                    &iRespLen);
    if (dwError == SASL_OK)
    {
        dwError = _VmDirSASLGetCtxProps(pSaslBindInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LDAP_SUCCESS;
        pSaslBindInfo->saslStatus = SASL_STATUS_DONE;
    }
    else if (dwError == SASL_CONTINUE)
    {
        pSaslBindInfo->saslStatus = SASL_STATUS_IN_PROGRESS;

        if (iRespLen > 0)
        {
            dwError = VmDirAllocateAndCopyMemory((PVOID)pszRespBlob, iRespLen, &pOutBlob);
            BAIL_ON_VMDIR_ERROR(dwError);

            pBervSaslReply->lberbv.bv_val = pOutBlob;
            pBervSaslReply->lberbv.bv_len = iRespLen;
            pBervSaslReply->bOwnBvVal = TRUE;
        }
        else
        {
            pBervSaslReply->lberbv.bv_len = 0;
            pBervSaslReply->lberbv.bv_val = "";
            pBervSaslReply->bOwnBvVal = FALSE;
        }

        dwError = LDAP_SUCCESS;
        pResult->errCode = LDAP_SASL_BIND_IN_PROGRESS;
        pResult->replyInfo.type = REP_SASL;
    }
    else
    {
        BAIL_ON_SASL_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pOutBlob);

    pBervSaslReply->lberbv.bv_val = NULL;
    pBervSaslReply->lberbv.bv_len = 0;
    pBervSaslReply->bOwnBvVal = FALSE;

    goto cleanup;

sasl_error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "SASLSessionStep: sasl error (%d)(%s)",
                     dwError,
                     VDIR_SAFE_STRING(sasl_errdetail(pSaslBindInfo->pSaslCtx)) );

    dwError = _VmDirSASLToLDAPError(dwError);
    if (dwError == LDAP_INVALID_CREDENTIALS )
    {
        _VmDirSASLGetCtxProps(pSaslBindInfo);
        // ignore error
    }

    goto error;
}

/*
 * Free SASL related resources.
 */
VOID
VmDirSASLSessionClose(
    PVDIR_SASL_BIND_INFO    pSaslBindInfo
    )
{
    if (pSaslBindInfo)
    {
        if (pSaslBindInfo->pSaslCtx)
        {
            if (pSaslBindInfo->saslSSF > 0)
            {
                VmDirSASLSockbufRemove(pSaslBindInfo->pSockbuf);
            }

            sasl_dispose(&pSaslBindInfo->pSaslCtx);
            pSaslBindInfo->pSaslCtx = NULL;
        }

        VMDIR_SAFE_FREE_MEMORY(pSaslBindInfo->pSessionCB);
        VMDIR_SAFE_FREE_MEMORY(pSaslBindInfo->pszBindUserName);
        VmDirFreeBervalContent(&pSaslBindInfo->bvMechnism);
    }

    return;
}

/*
 * Free SASL library resources -- do not call until all library uses are done
 */
VOID
VmDirSASLShutdown(
    VOID
    )
{
    sasl_done();
    VMDIR_SAFE_FREE_MEMORY(g_pszSASL2Path);
}

static
DWORD
_VmDirSASLGetCtxProps(
    PVDIR_SASL_BIND_INFO    pSaslBindInfo
    )
{
    DWORD   dwError = 0;
    PCSTR   pszBindUPN = NULL;
    PCSTR   pszBindRealm = NULL;
    sasl_ssf_t* pLocalSaslSSF = NULL;

    // pSaslCtx owns pszBindUPN
    dwError = sasl_getprop(pSaslBindInfo->pSaslCtx, SASL_USERNAME, (const void**)&pszBindUPN);
    BAIL_ON_SASL_ERROR(dwError);

    // pSaslCtx owns pszBindRealm
    dwError = sasl_getprop(pSaslBindInfo->pSaslCtx, SASL_DEFUSERREALM, (const void**)&pszBindRealm);
    BAIL_ON_SASL_ERROR(dwError);

    //////////////////////////////////////////////////////////////////////////////////////////////
    // BUGBUG, Not clear how to get the realm part in GSSAPI bind.
    // BUGBUG, In this case SASL_USERNAME has NO realm portion and SASL_DEFUSERREALM is NULL.
    // BUGBUG, Thus, use gVmdirKrbGlobals.pszRealm for now.
    // BUGBUG, This will not work if we have to support multiple realms scenario.
    //////////////////////////////////////////////////////////////////////////////////////////////
    // In SRP case, SASL_USERNAME has full UPN and SASL_DEFUSERREALM is NULL.
    //////////////////////////////////////////////////////////////////////////////////////////////
    if ( pszBindRealm == NULL
         &&
         VmDirStringCompareA( pSaslBindInfo->bvMechnism.lberbv.bv_val, SASL_GSSAPI_MECH, FALSE ) == 0
       )
    {
        pszBindRealm = gVmdirKrbGlobals.pszRealm;
    }

    VMDIR_SAFE_FREE_MEMORY( pSaslBindInfo->pszBindUserName );
    dwError = VmDirAllocateStringPrintf(    &pSaslBindInfo->pszBindUserName,
                                                "%s%s%s",
                                                VDIR_SAFE_STRING(pszBindUPN),
                                                pszBindRealm ? "@" : "",
                                                VDIR_SAFE_STRING(pszBindRealm));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = sasl_getprop( pSaslBindInfo->pSaslCtx, SASL_SSF, (const void**)&pLocalSaslSSF );
    BAIL_ON_SASL_ERROR(dwError);
    pSaslBindInfo->saslSSF = *pLocalSaslSSF;

cleanup:

    return dwError;

error:

    goto cleanup;

sasl_error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "_VmDirSASLGetCtxProps: sasl error (%d)(%s)",
                     dwError,
                     VDIR_SAFE_STRING(sasl_errdetail(pSaslBindInfo->pSaslCtx)) );

    dwError = _VmDirSASLToLDAPError(dwError);

    goto error;

}

static
int
_VmDirSASLLog(
    PVOID       pVoid,
    int         iPriority,
    PCSTR       pszMessage
    )
{
    // Lotus does not use any SASL plugins, but SASL code return this message in success scenario as well.
    static  PCSTR   pszIgnoreThisMsg = "could not find auxprop plugin, was searching for '[all]'";

    if ( pszMessage != NULL )
    {   // ignore iPriority for now, log every thing.
        if ( VmDirStringCompareA( pszMessage, pszIgnoreThisMsg, FALSE ) != 0 )
        {
            VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL , "SASL log (%s)", pszMessage);
        }
    }

    return SASL_OK;
}

static
DWORD
_VmDirSASLToLDAPError(
    int     saslError
    )
{
    DWORD   dwError;

    switch (saslError)
    {
        case SASL_OK:
            dwError = LDAP_SUCCESS;
            break;

        case SASL_CONTINUE:
            dwError = LDAP_SASL_BIND_IN_PROGRESS;
            break;

        case SASL_NOMECH:
            dwError = LDAP_AUTH_METHOD_NOT_SUPPORTED;
            break;

        case SASL_BADAUTH:
        case SASL_NOUSER:
        case SASL_TRANS:
        case SASL_EXPIRED:
            dwError = LDAP_INVALID_CREDENTIALS;
            break;

        case SASL_NOAUTHZ:
            dwError = LDAP_INSUFFICIENT_ACCESS;
            break;

        case SASL_TOOWEAK:
        case SASL_ENCRYPT:
            dwError = LDAP_INAPPROPRIATE_AUTH;
            break;

        case SASL_UNAVAIL:
        case SASL_TRYAGAIN:
            dwError = LDAP_UNAVAILABLE;
            break;

        case SASL_DISABLED:
            dwError = LDAP_UNWILLING_TO_PERFORM;
            break;

        case SASL_FAIL:
        case SASL_NOMEM:
        default:
            dwError = LDAP_OTHER;
            break;
    }

    return dwError;
}

static
PVOID
_VmDirSASLMutexNew(
    VOID
    )
{
    DWORD           dwError = 0;
    PVMDIR_MUTEX    pLocalMutex = NULL;

    dwError = VmDirAllocateMutex(&pLocalMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return pLocalMutex;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirSASLMutexNew failed (%d)", dwError);

    VMDIR_SAFE_FREE_MUTEX(pLocalMutex);

    goto cleanup;
}

static
VOID
_VmDirSASLMutexDispose(
    PVOID   pSASLMutex
    )
{
    PVMDIR_MUTEX    pLocalMutex = (PVMDIR_MUTEX) pSASLMutex;

    VMDIR_SAFE_FREE_MUTEX(pLocalMutex);

    return;
}

static
int
_VmDirSASLMutexLock(
    PVOID   pSASLMutex
    )
{
    DWORD           dwError = SASL_OK;
    PVMDIR_MUTEX    pLocalMutex = (PVMDIR_MUTEX) pSASLMutex;

    dwError = VmDirLockMutex(pLocalMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return (int)dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirSASLMutexLock failed (%d)", dwError);
    dwError = SASL_FAIL;

    goto cleanup;
}

static
int
_VmDirSASLMutexUnlock(
    PVOID   pSASLMutex
    )
{
    DWORD           dwError = SASL_OK;
    PVMDIR_MUTEX    pLocalMutex = (PVMDIR_MUTEX) pSASLMutex;

    dwError = VmDirUnLockMutex(pLocalMutex);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return (int)dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirSASLMutexUnlock failed (%d)", dwError);
    dwError = SASL_FAIL;

    goto cleanup;
}

/*
 * For vmdir, sasl2 path should be "default cyrus sasl2;vmdir sasl2"
 * e.g.
 * Windows "c:\CMU\bin\sasl2;c:\program files\vmware\cis\vmdir\sasl2"
 * Linux   "/opt/likewise/lib64/sasl2:/usr/lib/vmware-vmdir/lib64/sasl2"
 */
static
int
_VmDirSASL2PATH(
    void *          pContext,
    const char **   ppszPath
    )
{
    DWORD   dwError = 0;

    *ppszPath = *((const char**) pContext);

    return dwError;
}

static
int
VmDirSASL2PATH(
    PSTR *ppszPath
    )
{
    DWORD   dwError = 0;
    PSTR    pszLocalPath = NULL;

#ifdef _WIN32
    char    sasl2SearchPathBuf[MAX_PATH] = {0};
    char    vmdirInstallPathBuf[MAX_PATH] = {0};

    // base sasl lib path
    dwError = VmDirGetRegKeyValue( VDMIR_CONFIG_SASL2_KEY_PATH,
                                   "SearchPath",
                                   sasl2SearchPathBuf,
                                   MAX_PATH );
    BAIL_ON_VMDIR_ERROR(dwError);

    // vmdir specific sasl lib path
    dwError = VmDirGetRegKeyValue(  VMDIR_CONFIG_SOFTWARE_KEY_PATH,
                                    "InstallPath",
                                    vmdirInstallPathBuf,
                                    MAX_PATH );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszLocalPath, "%s;%s\\sasl2",
                                             sasl2SearchPathBuf,
                                             vmdirInstallPathBuf);
#else
    dwError = VmDirAllocateStringPrintf( &pszLocalPath, "%s:%s/sasl2",
                                             VMDIR_CONFIG_SASL2_LIB_PATH,
                                             VMDIR_LIB_DIR);
#endif
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszPath = pszLocalPath;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "SASL2PATH=%s", pszLocalPath);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSASL2PATH failed (%d)", dwError);
    VMDIR_SAFE_FREE_MEMORY( pszLocalPath );

    goto cleanup;
}
