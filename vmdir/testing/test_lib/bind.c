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
_VmDirTestSRPBind(
    LDAP*   pLd,
    PCSTR      pszUPN,
    PCSTR      pszPass,
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

    dwError = VmDirAllocASCIIUpperToLower(pszUPN, &pszLowerCaseUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    srpDefault.pszAuthName = pszLowerCaseUPN;
    srpDefault.pszPass     = pszPass;

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
    BAIL_ON_VMDIR_ERROR(dwError);

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
_VmDirTestSimpleBind(
    LDAP*   pLd,
    PCSTR      pszDN,
    PCSTR      pszPass,
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

    passwdBV.bv_len = VmDirStringLenA(pszPass);
    passwdBV.bv_val = (PSTR)pszPass;

    dwError = ldap_sasl_bind(
        pLd,
        pszDN,
        LDAP_SASL_SIMPLE,
        &passwdBV,
        psctrls,
        pcctrls,
        &iMsgId);
    BAIL_ON_VMDIR_ERROR(dwError);

    iRtn = ldap_result(pLd, iMsgId, LDAP_MSG_ALL, NULL, &pResult);
    if (iRtn != LDAP_RES_BIND || !pResult )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_STATE);
    }

    *ppResult = pResult;

cleanup:
    return dwError;

error:
    ldap_msgfree(pResult);
    goto cleanup;
}

static
DWORD
VmDirTestDirBind(
     PCSTR      pszAuthMethod,
     PCSTR      pszHost,
     PCSTR      pszDN,
     PCSTR      pszUPN,
     PCSTR      pszPass,
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

    if (!ppOutLd || !pszHost || !ppOutResult || !pszPass  || (!pszDN && !pszUPN))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszURI, "ldap://%s:%s", pszHost, DEFAULT_LDAP_PORT_STR);
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

    if (VmDirStringCompareA(pszAuthMethod, "simple", FALSE) == 0)
    {
        dwError = _VmDirTestSimpleBind(
            pLd,
            pszDN,
            pszPass,
            psctrls,
            pcctrls,
            ppOutResult);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (VmDirStringCompareA(pszAuthMethod, "srp", FALSE) == 0)
    {
        dwError = _VmDirTestSRPBind(
            pLd,
            pszUPN,
            pszPass,
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
VmDirTestSRPBind(
     PCSTR      pszHost,
     PCSTR      pszUPN,
     PCSTR      pszPass,
     int        iTimeout,
     LDAPControl** psctrls,
     LDAPControl** pcctrls,
     LDAP**     ppOutLd,
     LDAPMessage** ppOutResult
     )
{
    return VmDirTestDirBind(
                "srp",
                pszHost,
                NULL,
                pszUPN,
                pszPass,
                iTimeout,
                psctrls,
                pcctrls,
                ppOutLd,
                ppOutResult);
}

DWORD
VmDirTestSimpleBind(
     PCSTR      pszHost,
     PCSTR      pszDN,
     PCSTR      pszPass,
     int        iTimeout,
     LDAPControl** psctrls,
     LDAPControl** pcctrls,
     LDAP**     ppOutLd,
     LDAPMessage** ppOutResult
     )
{
    return VmDirTestDirBind(
                "simple",
                pszHost,
                pszDN,
                NULL,
                pszPass,
                iTimeout,
                psctrls,
                pcctrls,
                ppOutLd,
                ppOutResult);
}
