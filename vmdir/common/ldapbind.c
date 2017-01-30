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
int
_VmDirSASLGSSAPIInteraction(
    LDAP *      pLd,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    );

static
int
_VmDirSASLSRPInteraction(
    LDAP *      pLd,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    );

DWORD
VmDirSASLGSSAPIBind(
     LDAP**     ppLd,
     PCSTR      pszURI
     )
{
    DWORD       dwError = 0;
    int         retVal = 0;
    LDAP*       pLd = NULL;
    const int   ldapVer = LDAP_VERSION3;

    if ( ppLd == NULL || pszURI == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    retVal = ldap_initialize( &pLd, pszURI);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = ldap_sasl_interactive_bind_s( pLd,
                                            NULL,
                                            "GSSAPI",
                                            NULL,
                                            NULL,
                                            LDAP_SASL_QUIET,
                                            _VmDirSASLGSSAPIInteraction,
                                            NULL);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    *ppLd = pLd;

cleanup:

    return dwError;

ldaperror:

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirSASLGSSBind failed. (%d)(%s)",
                                           retVal, ldap_err2string(retVal) );
    dwError = VmDirMapLdapError(retVal);

error:
    if (retVal == 0)
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirSASLGSSBind failed. (%u)", dwError);
    }

    if ( pLd )
    {
        ldap_unbind_ext_s( pLd, NULL, NULL);
    }
    goto cleanup;
}

DWORD
VmDirSASLSRPBind(
     LDAP**     ppLd,
     PCSTR      pszURI,
     PCSTR      pszUPN,
     PCSTR      pszPass
     )
{
    DWORD       dwError = 0;
    int         retVal = 0;
    PSTR        pszLowerCaseUPN = NULL;
    LDAP*       pLd = NULL;
    const int   ldapVer = LDAP_VERSION3;
    const int   iSaslNoCanon = 1;
    VMDIR_SASL_INTERACTIVE_DEFAULT srpDefault = {0};
    int         iCnt = 0;

    if ( ppLd == NULL || pszURI == NULL || pszUPN == NULL || pszPass == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocASCIIUpperToLower( pszUPN, &pszLowerCaseUPN );
    BAIL_ON_VMDIR_ERROR(dwError);

    srpDefault.pszAuthName = pszLowerCaseUPN;
    srpDefault.pszPass     = pszPass;

    for (iCnt=0; iCnt<2; iCnt++)
    {
        retVal = ldap_initialize( &pLd, pszURI);
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

        retVal = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

        // turn off SASL hostname canonicalization for SRP mech
        retVal = ldap_set_option(pLd, LDAP_OPT_X_SASL_NOCANON, &iSaslNoCanon);
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

        retVal = ldap_sasl_interactive_bind_s( pLd,
                                                NULL,
                                                "SRP",
                                                NULL,
                                                NULL,
                                                LDAP_SASL_QUIET,
                                                _VmDirSASLSRPInteraction,
                                                &srpDefault);
        if (retVal == LDAP_SERVER_DOWN)
        {
            VmDirSleep(50); // pause 50 ms
            if ( pLd )
            {
                ldap_unbind_ext_s(pLd, NULL, NULL);
                pLd = NULL;
            }
            continue;   // if transient network error, retry once.
        }
        else
        {
            break;
        }
    }
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);  // bail ldap_sasl_interactive_bind_s failure.

    *ppLd = pLd;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseUPN);

    return dwError;

ldaperror:

    VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirSASLSRPBind failed. (%d)(%s)",
                                           retVal, ldap_err2string(retVal) );
    dwError = VmDirMapLdapError(retVal);

error:
    if (retVal == 0)
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirSASLSRPBind failed. (%u)", dwError);
    }

    if ( pLd )
    {
        ldap_unbind_ext_s( pLd, NULL, NULL);
    }
    goto cleanup;
}

/*
 * Bind to a LDAP server via SSL port.
 * Require server certificate verification.
 *
 * In 5.5. mix mode, replication goes through ldaps port.
 */
DWORD
VmDirSSLBind(
     LDAP**     ppLd,
     PCSTR      pszURI,
     PCSTR      pszDN,
     PCSTR      pszPassword
     )
{
    DWORD     dwError = 0;
    int       retVal = 0;
    LDAP*     pLd = NULL;
    BerValue  ldapBindPwd = {0};
    const int ldapVer = LDAP_VERSION3;
    int       iTLSDEMAND = LDAP_OPT_X_TLS_DEMAND;
    int       iTLSMin =  LDAP_OPT_X_TLS_PROTOCOL_TLS1_0;
    PSTR      pszTrustCertFile = NULL;
    SSL_CTX*  pSslCtx = NULL;

    if ( ppLd == NULL || pszURI == NULL )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // only allow ldaps traffic over SSL port
    if ( VmDirStringNCompareA( pszURI, VMDIR_LDAPS_PROTOCOL, 5, FALSE) != 0 )
    {
        dwError = VMDIR_ERROR_ACCESS_DENIED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirPrepareOpensslClientCtx( &pSslCtx, &pszTrustCertFile, pszURI );
    BAIL_ON_VMDIR_ERROR(dwError);

    retVal = ldap_set_option( NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &iTLSDEMAND);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = ldap_set_option(NULL, LDAP_OPT_X_TLS_PROTOCOL_MIN, &iTLSMin);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = ldap_initialize(&pLd, pszURI);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = ldap_set_option( pLd, LDAP_OPT_X_TLS_CTX, pSslCtx);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    ldapBindPwd.bv_val = 0;
    ldapBindPwd.bv_len = 0;
    if (pszPassword)
    {
        ldapBindPwd.bv_val = (PSTR) pszPassword;
        ldapBindPwd.bv_len = (ULONG) VmDirStringLenA(pszPassword);
    }
    retVal = ldap_sasl_bind_s( pLd,
                                pszDN,
                                LDAP_SASL_SIMPLE,
                                &ldapBindPwd,      // ldaps with credentials
                                NULL,
                                NULL,
                                NULL);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    *ppLd = pLd;

cleanup:

    if (pSslCtx)
    {
        SSL_CTX_free(pSslCtx);
    }
    VMDIR_SAFE_FREE_MEMORY(pszTrustCertFile);

    return dwError;

ldaperror:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSSLBind failed for %s %s. (%d)(%s)",
                                         pszURI, pszDN,
                                         retVal, ldap_err2string(retVal));
    dwError = VmDirMapLdapError(retVal);

error:
    if (retVal == 0)
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "_VmDirSSLBind failed. (%u)", dwError);
    }

    if ( pLd )
    {
        ldap_unbind_ext_s( pLd, NULL, NULL);
    }
    goto cleanup;
}

/*
 * Bind to partner via "SRP" mechanism.
 */
DWORD
VmDirSafeLDAPBind(
    LDAP**      ppLd,
    PCSTR       pszHost,
    PCSTR       pszUPN,
    PCSTR       pszPassword
    )
{
    DWORD       dwError = 0;

    LDAP*       pLd = NULL;
    char        ldapURI[VMDIR_MAX_LDAP_URI_LEN + 1] = {0};

    if (ppLd == NULL || pszHost == NULL || pszUPN == NULL || pszPassword == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirIsIPV6AddrFormat( pszHost ) )
    {
        dwError = VmDirStringPrintFA( ldapURI, sizeof(ldapURI)-1,  "%s://[%s]:%d",
                                      VMDIR_LDAP_PROTOCOL, pszHost, DEFAULT_LDAP_PORT_NUM);
    }
    else
    {
        dwError = VmDirStringPrintFA( ldapURI, sizeof(ldapURI)-1,  "%s://%s:%d",
                                      VMDIR_LDAP_PROTOCOL, pszHost, DEFAULT_LDAP_PORT_NUM);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSASLSRPBind( &pLd, &(ldapURI[0]), pszUPN, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLd;

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSafeLDAPBind to (%s) failed. SRP(%d)",
                     ldapURI, dwError );

    if ( pLd )
    {
        ldap_unbind_ext_s( pLd, NULL, NULL);
    }

    goto cleanup;
}

/*
 * There is NO password supplied in anonymous bind.
 * This is the only place we allow LDAP_SASL_SIMPLE bind over port 389.
 *
 * Thus, NO credentials would ever go over clear text channel.
 */
DWORD
VmDirAnonymousLDAPBind(
    LDAP**      ppLd,
    PCSTR       pszLdapURI
    )
{
    DWORD       dwError = 0;
    int         retVal = 0;
    const int   ldapVer = LDAP_VERSION3;
    BerValue    ldapBindPwd = {0};
    LDAP*       pLocalLd = NULL;


    if (ppLd == NULL || pszLdapURI == NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    retVal = ldap_initialize( &pLocalLd, pszLdapURI);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    retVal = ldap_set_option( pLocalLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    ldapBindPwd.bv_val = NULL;
    ldapBindPwd.bv_len = 0;

    retVal = ldap_sasl_bind_s(
                               pLocalLd,
                               "",
                               LDAP_SASL_SIMPLE,
                               &ldapBindPwd,  // no credentials
                               NULL,
                               NULL,
                               NULL);
    BAIL_ON_SIMPLE_LDAP_ERROR(retVal);

    *ppLd = pLocalLd;

cleanup:

    return dwError;

ldaperror:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirAnonymousLDAPBind to (%s) failed. (%d)(%s)",
                     VDIR_SAFE_STRING(pszLdapURI), retVal, ldap_err2string(retVal) );
    dwError = VmDirMapLdapError(retVal);

error:
    if (retVal == 0)
    {
        VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "VmDirAnonymousLDAPBind to (%s) failed. (%u)", VDIR_SAFE_STRING(pszLdapURI), dwError);
    }

    if (pLocalLd)
    {
        ldap_unbind_ext_s( pLocalLd, NULL, NULL);
    }

    goto cleanup;
}

static
int
_VmDirSASLSRPInteraction(
    LDAP *      pLd,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    )
{
    sasl_interact_t*                pInteract = pIn;
    PVMDIR_SASL_INTERACTIVE_DEFAULT pDef = pDefaults;

    while( (pDef != NULL) && (pInteract->id != SASL_CB_LIST_END) )
    {

        switch( pInteract->id )
        {
        case SASL_CB_GETREALM:
                pInteract->defresult = pDef->pszRealm;
                break;
        case SASL_CB_AUTHNAME:
                pInteract->defresult = pDef->pszAuthName;
                break;
        case SASL_CB_PASS:
                pInteract->defresult = pDef->pszPass;
                break;
        case SASL_CB_USER:
                pInteract->defresult = pDef->pszUser;
                break;
        default:
                break;
        }

        pInteract->result = (pInteract->defresult) ? pInteract->defresult : "";
        pInteract->len    = (unsigned int) VmDirStringLenA( pInteract->result );

        pInteract++;
    }

    return LDAP_SUCCESS;
}

static
int
_VmDirSASLGSSAPIInteraction(
    LDAP *      pLd,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    )
{
    // dummy function to satisfy ldap_sasl_interactive_bind call
    return LDAP_SUCCESS;
}

int
VmDirCreateSyncRequestControl(
    PCSTR pszInvocationId,
    USN lastLocalUsnProcessed,
    PCSTR pszUtdVector,
    LDAPControl *           syncReqCtrl
    )
{
    int                 retVal = LDAP_SUCCESS;
    BerElement *        ber = NULL;
    PSTR                pszLastLocalUsnProcessed = NULL;

    if (syncReqCtrl == NULL)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    if ((ber = ber_alloc()) == NULL)
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    if (VmDirAllocateStringPrintf(&pszLastLocalUsnProcessed, "%lld", lastLocalUsnProcessed))
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

    if ( ber_printf( ber, "{i{sss}}", LDAP_SYNC_REFRESH_ONLY,
                    pszInvocationId,
                    pszLastLocalUsnProcessed,
                    pszUtdVector ) == -1)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirCreateSyncRequestControl: ber_printf failed." );
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR( retVal );
    }

    memset( syncReqCtrl, 0, sizeof( LDAPControl ));
    syncReqCtrl->ldctl_oid = LDAP_CONTROL_SYNC;
    syncReqCtrl->ldctl_iscritical = '1';
    if (ber_flatten2(ber, &syncReqCtrl->ldctl_value, 1))
    {
        retVal = LDAP_OPERATIONS_ERROR;
        BAIL_ON_SIMPLE_LDAP_ERROR(retVal);
    }

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszLastLocalUsnProcessed);
    if (ber)
    {
        ber_free(ber, 1);
    }
    return retVal;

ldaperror:
    ber_bvfree(&syncReqCtrl->ldctl_value);
    goto cleanup;
}

VOID
VmDirDeleteSyncRequestControl(
    LDAPControl *           syncReqCtrl
    )
{
    if (syncReqCtrl)
    {
        if (syncReqCtrl->ldctl_value.bv_val)
        {
            ber_memfree(syncReqCtrl->ldctl_value.bv_val);
        }
        memset(syncReqCtrl, 0, sizeof(LDAPControl));
    }
}

DWORD
VmDirMapLdapError(
    int ldapErrorCode
    )
{
    switch(ldapErrorCode)
    {
        case LDAP_SUCCESS:
            return VMDIR_SUCCESS;
        case LDAP_SERVER_DOWN:
            return VMDIR_ERROR_SERVER_DOWN;
        case LDAP_UNWILLING_TO_PERFORM:
            return VMDIR_ERROR_UNWILLING_TO_PERFORM;
        case LDAP_SASL_BIND_IN_PROGRESS:
            return VMDIR_ERROR_SASL_BIND_IN_PROGRESS;
        case LDAP_ALREADY_EXISTS:
            return VMDIR_ERROR_ENTRY_ALREADY_EXIST;
        case LDAP_INSUFFICIENT_ACCESS:
            return VMDIR_ERROR_INSUFFICIENT_ACCESS;
        case LDAP_NO_SUCH_ATTRIBUTE:
            return VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        case LDAP_INVALID_SYNTAX:
            return VMDIR_ERROR_INVALID_SYNTAX;
        case LDAP_SIZELIMIT_EXCEEDED:
            return VMDIR_ERROR_SIZELIMIT_EXCEEDED;
        case LDAP_TYPE_OR_VALUE_EXISTS:
            return VMDIR_ERROR_TYPE_OR_VALUE_EXISTS;
        case LDAP_AUTH_METHOD_NOT_SUPPORTED:
            return VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED;
        case LDAP_NO_SUCH_OBJECT:
            return VMDIR_ERROR_ENTRY_NOT_FOUND;
        case LDAP_UNDEFINED_TYPE:
            return VMDIR_ERROR_UNDEFINED_TYPE;
        case LDAP_TIMELIMIT_EXCEEDED:
            return VMDIR_ERROR_TIMELIMIT_EXCEEDED;
        case LDAP_INVALID_DN_SYNTAX:
            return VMDIR_ERROR_INVALID_DN;
        case LDAP_NOT_ALLOWED_ON_NONLEAF:
            return VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF;
        case LDAP_OBJECT_CLASS_VIOLATION:
            return VMDIR_ERROR_OBJECTCLASS_VIOLATION;
        case LDAP_INVALID_CREDENTIALS:
            return VMDIR_ERROR_USER_INVALID_CREDENTIAL;
        case LDAP_UNAVAILABLE:
            return VMDIR_ERROR_UNAVAILABLE;
        case LDAP_CONSTRAINT_VIOLATION:
            return VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        case LDAP_BUSY:
            return VMDIR_ERROR_BUSY;
        default:
            return VMDIR_ERROR_GENERIC;
    }
}

/*
 *  Bind to a host with the handle to be used later
 */
DWORD
VmDirConnectLDAPServerWithMachineAccount(
    PCSTR  pszHostName,
    PCSTR  pszDomain,
    LDAP** ppLd
    )
{
    DWORD dwError = 0;
    PSTR pszDCAccount = NULL;
    PSTR pszDCAccountPassword = NULL;
    PSTR pszServerName = NULL;
    char bufUPN[VMDIR_MAX_UPN_LEN] = {0};
    LDAP* pLd = NULL;

    dwError = VmDirRegReadDCAccount( &pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirReadDCAccountPassword( &pszDCAccountPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA( bufUPN, sizeof(bufUPN)-1,  "%s@%s", pszDCAccount, pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServerName( pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind( &pLd, pszServerName, bufUPN, pszDCAccountPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLd;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    VMDIR_SECURE_FREE_STRINGA(pszDCAccountPassword);
    VMDIR_SAFE_FREE_STRINGA(pszServerName);
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirLdapUnbind(
    LDAP** ppLd
    )
{
    if (ppLd && *ppLd)
    {
        ldap_unbind_ext_s(*ppLd, NULL, NULL);
        *ppLd = NULL;
    }
}
