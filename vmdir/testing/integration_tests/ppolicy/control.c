/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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
DWORD
_DirTestSRPCtrlBind(
    LDAP*   pLd,
    PVMDIR_PP_CTRL_BIND pCtrlBind,
    LDAPControl** psctrls,
    LDAPControl** pcctrls,
    LDAPMessage**   ppResult
    )
{
    DWORD   dwError = 0;
    VMDIR_SASL_INTERACTIVE_DEFAULT srpDefault = {0};
    PCSTR   pszRtnMech = NULL;
    int     iMsgId = 0;
    LDAPMessage*    pResult = NULL;
    const int   iSaslNoCanon = 1;
    PSTR        pszLowerCaseUPN = NULL;
    int         iRtn=0;

    dwError = VmDirAllocASCIIUpperToLower(pCtrlBind->pszBindUPN, &pszLowerCaseUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    srpDefault.pszAuthName = pszLowerCaseUPN;
    srpDefault.pszPass     = pCtrlBind->pszPassword;

    dwError = ldap_set_option(pLd, LDAP_OPT_X_SASL_NOCANON, &iSaslNoCanon);
    BAIL_ON_VMDIR_ERROR(dwError);

    do {
        dwError = ldap_sasl_interactive_bind(
            pLd,
            NULL,
            "SRP",
            psctrls,
            pcctrls,
            LDAP_SASL_QUIET,
            VmDirSASLSRPInteraction,
            &srpDefault,
            pResult,
            &pszRtnMech,
            &iMsgId);

        if (dwError != LDAP_SASL_BIND_IN_PROGRESS)
        {
            break;
        }

        ldap_msgfree(pResult);
        iRtn = ldap_result(pLd, iMsgId, LDAP_MSG_ALL, NULL, &pResult);
        if (iRtn != LDAP_RES_BIND || !pResult )
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
        }

    } while ( dwError == LDAP_SASL_BIND_IN_PROGRESS );
    pCtrlBind->dwBindResult = dwError;
    dwError = 0;

    *ppResult = pResult;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseUPN);
    return dwError;

error:
    ldap_msgfree(pResult);
    goto cleanup;
}

static
DWORD
_DirTestSimpleCtrlBind(
    LDAP*   pLd,
    PVMDIR_PP_CTRL_BIND pCtrlBind,
    LDAPControl** psctrls,
    LDAPControl** pcctrls,
    LDAPMessage**   ppResult
    )
{
    DWORD   dwError = 0;
    int     iMsgId = 0;
    BerValue    passwdBV = {0};
    LDAPMessage*    pResult = NULL;
    int         iRtn = 0;
    int         iTLSNEVER = LDAP_OPT_X_TLS_NEVER;
    SSL_CTX*    pSslCtx  = NULL;

    pSslCtx = SSL_CTX_new(TLSv1_2_client_method());
    if (!pSslCtx)
    {
        dwError = VMDIR_ERROR_NO_SSL_CTX;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    // no server ssl cert verify, just channel encryption
    SSL_CTX_set_verify(pSslCtx, SSL_VERIFY_NONE, NULL);

    dwError = ldap_set_option(pLd, LDAP_OPT_X_TLS_CTX, pSslCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_set_option(pLd, LDAP_OPT_X_TLS_REQUIRE_CERT, &iTLSNEVER);
    BAIL_ON_VMDIR_ERROR(dwError);

    passwdBV.bv_len = VmDirStringLenA(pCtrlBind->pszPassword);
    passwdBV.bv_val = (PSTR)pCtrlBind->pszPassword;

    pCtrlBind->dwBindResult = ldap_sasl_bind(
        pLd,
        pCtrlBind->pszBindDN,
        LDAP_SASL_SIMPLE,
        &passwdBV,
        psctrls,
        pcctrls,
        &iMsgId);

    iRtn = ldap_result(pLd, iMsgId, LDAP_MSG_ALL, NULL, &pResult);
    if (iRtn != LDAP_RES_BIND || !pResult )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    *ppResult = pResult;

cleanup:
    if (pSslCtx)
    {
        SSL_CTX_free(pSslCtx);
    }
    return dwError;

error:
    ldap_msgfree(pResult);
    goto cleanup;
}

DWORD
TestCtrlBind(
     PVMDIR_PP_CTRL_BIND pCtrlBind,
     int        iTimeout,
     LDAPControl** psctrls,
     LDAPControl** pcctrls,
     LDAP**     ppOutLd,
     LDAPMessage** ppOutResult
     )
{
    DWORD       dwError = 0;
    PSTR        pszURI = NULL;
    LDAP*       pLd = NULL;
    const int   ldapVer = LDAP_VERSION3;

    if (VmDirStringCompareA(pCtrlBind->pszMech, TEST_SASL_SIMPLE, FALSE) == 0)
    {
        dwError = VmDirAllocateStringPrintf(&pszURI, "ldaps://%s:%s", pCtrlBind->pszHost, DEFAULT_LDAPS_PORT_STR);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(&pszURI, "ldap://%s:%s", pCtrlBind->pszHost, DEFAULT_LDAP_PORT_STR);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_initialize(&pLd, pszURI);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_set_option(pLd, LDAP_OPT_PROTOCOL_VERSION, &ldapVer);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (iTimeout > 0)
    {
        struct timeval  optTimeout={0};
        optTimeout.tv_sec = iTimeout;

        dwError = ldap_set_option(pLd, LDAP_OPT_NETWORK_TIMEOUT, (void *)&optTimeout);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirStringCompareA(pCtrlBind->pszMech, TEST_SASL_SIMPLE, FALSE) == 0)
    {
        dwError = _DirTestSimpleCtrlBind(
            pLd,
            pCtrlBind,
            psctrls,
            pcctrls,
            ppOutResult);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (VmDirStringCompareA(pCtrlBind->pszMech, TEST_SASL_SRP, FALSE) == 0)
    {
        dwError = _DirTestSRPCtrlBind(
            pLd,
            pCtrlBind,
            psctrls,
            pcctrls,
            ppOutResult);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppOutLd = pLd;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszURI);
    return dwError;

error:
    VDIR_SAFE_LDAP_UNBIND_EXT_S(pLd);
    goto cleanup;
}

DWORD
TestPPCtrlBind(
    PVMDIR_PP_CTRL_BIND pCtrlBind
    )
{
    DWORD   dwError = 0;

    LDAP*      pLd = NULL;
    LDAPMessage*    pResult = NULL;
    LDAPControl* psctrls = NULL;
    LDAPControl** ppcctrls = NULL;
    LDAPControl* srvCtrls[2]  = {NULL, NULL};
    LDAPControl* pPPReplyCtrl = NULL;
    int         errCode = 0;

    dwError = ldap_control_create(
        LDAP_CONTROL_PASSWORDPOLICYREQUEST,0, NULL, 0, &psctrls);
    BAIL_ON_VMDIR_ERROR(dwError);;

    srvCtrls[0] = psctrls;
    srvCtrls[1] = NULL;

    dwError = TestCtrlBind(
        pCtrlBind,
        3,
        srvCtrls,
        NULL,
        &pLd,
        &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pResult)
    {
        dwError = ldap_parse_result(
            pLd,
            pResult,
            &errCode,
            NULL,
            NULL,
            NULL,
            &ppcctrls,
            1); // free pResult
        BAIL_ON_VMDIR_ERROR(dwError);

        pPPReplyCtrl = ldap_control_find(LDAP_CONTROL_PASSWORDPOLICYREQUEST, ppcctrls, NULL);
        if (pPPReplyCtrl)
        {
            pCtrlBind->bHasPPCtrlResponse = TRUE;
            dwError = ldap_parse_passwordpolicy_control(
                pLd,
                pPPReplyCtrl,
                &pCtrlBind->PPolicyState.iWarnPwdExpiring,
                &pCtrlBind->PPolicyState.iWarnGraceAuthN,
                &pCtrlBind->PPolicyState.PPolicyError);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    ldap_controls_free(ppcctrls);
    VDIR_SAFE_LDAP_UNBIND_EXT_S(pLd);
    return dwError;

error:
    goto cleanup;
}
