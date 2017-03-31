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
DWORD
_VmDirCheckChangePNIDPreConditions(
    PCSTR   pszUsername,
    PCSTR   pszPassword
    );

static
DWORD
_VmDirDeleteDefaultAccount(
    LDAP*   pLd,
    PCSTR   pszDomain,
    PCSTR   pszDCAccount
    );

static
DWORD
_VmDirDeleteServerObj(
    LDAP*   pLd,
    PSTR*   ppszServerId,
    PSTR*   ppszInvocationId,
    PSTR*   ppszReplInterval,
    PSTR*   ppszReplPageSize
    );

static
DWORD
_VmDirCreateServerObj(
    LDAP*   pLd,
    PCSTR   pszServerDN,
    PSTR    pszCN,
    PSTR    pszServerId,
    PSTR    pszInvocationId,
    PSTR    pszReplInterval,
    PSTR    pszReplPageSize
    );

static
DWORD
_VmDirCreateReplAgrsContainer(
    LDAP*   pLd,
    PCSTR   pszServerDN
    );

static
DWORD
_VmDirPatchDSERoot(
    LDAP*   pLd,
    PCSTR   pszServerDN,
    PCSTR   pszDCAccountDN,
    PCSTR   pszDCAccountUPN
    );

DWORD
VmDirChangePNID(
    PSTR    pszUsername,
    PSTR    pszPassword,
    PSTR    pszNewPNID
    )
{
    DWORD   dwError = 0;
    LDAP*   pLd = NULL;
    PSTR    pszDCAccount = NULL;
    PSTR    pszDomain = NULL;
    PSTR    pszSiteName = NULL;
    PSTR    pszLowerCasePNID = NULL;
    PSTR    pszUpperCaseDomain = NULL;
    PSTR    pszDomainDN = NULL;
    PSTR    pszServerDN = NULL;
    PSTR    pszDCAccountDN = NULL;
    PSTR    pszDCAccountUPN = NULL;

    PSTR    pszServerId = NULL;
    PSTR    pszInvocationId = NULL;
    PSTR    pszReplInterval = NULL;
    PSTR    pszReplPageSize = NULL;

    if (IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszNewPNID))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // check precondition
    dwError = _VmDirCheckChangePNIDPreConditions(pszUsername, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    // open local ldap connection
    dwError = VmDirRegReadDCAccount(&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName(pszDCAccount, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
            &pLd, "localhost", pszDomain, pszUsername, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    // build string parameters
    dwError = VmDirGetSiteNameInternal(pLd, &pszSiteName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower(pszNewPNID, &pszLowerCasePNID);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIILowerToUpper(pszDomain, &pszUpperCaseDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszServerDN, "%s=%s,%s=%s,%s=%s,%s=%s,%s=%s,%s",
            ATTR_CN, pszLowerCasePNID,
            ATTR_CN, VMDIR_SERVERS_CONTAINER_NAME,
            ATTR_CN, pszSiteName,
            ATTR_CN, VMDIR_SITES_RDN_VAL,
            ATTR_CN, VMDIR_CONFIGURATION_CONTAINER_NAME,
            pszDomainDN
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszDCAccountDN, "%s=%s,%s=%s,%s",
            ATTR_CN, pszLowerCasePNID,
            ATTR_OU, VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszDCAccountUPN, "%s@%s",
            pszLowerCasePNID,
            pszUpperCaseDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    // re-create or patch entries
    dwError = _VmDirDeleteDefaultAccount(pLd, pszDomain, pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetupDefaultAccount(
            pszDomain, pszNewPNID, pszNewPNID, pszUsername, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirDeleteServerObj(
            pLd,
            &pszServerId,
            &pszInvocationId,
            &pszReplInterval,
            &pszReplPageSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCreateServerObj(
            pLd,
            pszServerDN,
            pszLowerCasePNID,
            pszServerId,
            pszInvocationId,
            pszReplInterval,
            pszReplPageSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirPatchDSERoot(
            pLd, pszServerDN, pszDCAccountDN, pszDCAccountUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // update kerberos keytab file
    dwError = VmDirUpdateKeytabFile(
            pszNewPNID, pszDomain, pszNewPNID, pszUsername, pszPassword, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    VmDirLdapUnbind(&pLd);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccount);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszSiteName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCasePNID);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomain);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszServerDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccountDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszServerId);
    VMDIR_SAFE_FREE_MEMORY(pszInvocationId);
    VMDIR_SAFE_FREE_MEMORY(pszReplInterval);
    VMDIR_SAFE_FREE_MEMORY(pszReplPageSize);
    return dwError;
}

static
DWORD
_VmDirCheckChangePNIDPreConditions(
    PCSTR   pszUsername,
    PCSTR   pszPassword
    )
{
    DWORD   dwError = 0;
    DWORD   dwServerCnt = 0;

    // should have only one server object
    dwError = VmDirGetServers(
            "localhost", pszUsername, pszPassword, NULL, &dwServerCnt);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwServerCnt > 1)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    }

error:
    return dwError;
}

static
DWORD
_VmDirDeleteDefaultAccount(
    LDAP*   pLd,
    PCSTR   pszDomain,
    PCSTR   pszDCAccount
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PCSTR   pszServiceTable[] = VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER;

    if (!pLd ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszDCAccount))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; i < sizeof(pszServiceTable)/sizeof(pszServiceTable[0]); i++)
    {
        dwError = VmDirLdapDeleteServiceAccount(
                pLd, pszDomain, pszServiceTable[i], pszDCAccount, TRUE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapDeleteDCAccount(pLd, pszDomain, pszDCAccount, TRUE, TRUE);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_VmDirDeleteServerObj(
    LDAP*   pLd,
    PSTR*   ppszServerId,
    PSTR*   ppszInvocationId,
    PSTR*   ppszReplInterval,
    PSTR*   ppszReplPageSize
    )
{
    DWORD   dwError = 0;
    PSTR    pszServerDN = NULL;
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    struct berval** ppServerId = NULL;
    struct berval** ppInvocationid = NULL;
    struct berval** ppReplInterval = NULL;
    struct berval** ppReplPageSize = NULL;

    PCSTR   ppszAttrs[] =
    {
            ATTR_SERVER_ID,
            ATTR_INVOCATION_ID,
            ATTR_REPL_INTERVAL,
            ATTR_REPL_PAGE_SIZE,
            NULL
    };

    if (!pLd ||
        !ppszServerId ||
        !ppszInvocationId ||
        !ppszReplInterval ||
        !ppszReplPageSize)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerDN("localhost", &pszServerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                pLd,
                pszServerDN,
                LDAP_SCOPE_BASE,
                NULL,
                (PSTR*)ppszAttrs,
                FALSE, /* get values      */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit */
                &pResult);
    dwError = VmDirMapLdapError(dwError);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pResult);
    ppServerId = ldap_get_values_len(pLd, pEntry, ATTR_SERVER_ID);
    ppInvocationid = ldap_get_values_len(pLd, pEntry, ATTR_INVOCATION_ID);
    ppReplInterval = ldap_get_values_len(pLd, pEntry, ATTR_REPL_INTERVAL);
    ppReplPageSize = ldap_get_values_len(pLd, pEntry, ATTR_REPL_PAGE_SIZE);

    dwError = VmDirAllocateStringA(ppServerId[0]->bv_val, ppszServerId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ppInvocationid[0]->bv_val, ppszInvocationId);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ppReplInterval[0]->bv_val, ppszReplInterval);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(ppReplPageSize[0]->bv_val, ppszReplPageSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDeleteDITSubtree(pLd, pszServerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    if (ppServerId)
    {
        ldap_value_free_len(ppServerId);
    }
    if (ppInvocationid)
    {
        ldap_value_free_len(ppInvocationid);
    }
    if (ppReplInterval)
    {
        ldap_value_free_len(ppReplInterval);
    }
    if (ppReplPageSize)
    {
        ldap_value_free_len(ppReplPageSize);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    VMDIR_SAFE_FREE_MEMORY(pszServerDN);
    return dwError;
}

static
DWORD
_VmDirCreateServerObj(
    LDAP*   pLd,
    PCSTR   pszServerDN,
    PSTR    pszCN,
    PSTR    pszServerId,
    PSTR    pszInvocationId,
    PSTR    pszReplInterval,
    PSTR    pszReplPageSize
    )
{
    DWORD   dwError = 0;
    PCSTR   valsClass[] = {OC_DIR_SERVER, OC_TOP, NULL};
    PCSTR   valsCn[] = {pszCN, NULL};
    PCSTR   valsServerId[] = {pszServerId, NULL};
    PCSTR   valsInvocationId[] = {pszInvocationId, NULL};
    PCSTR   valsReplInterval[] = {pszReplInterval, NULL};
    PCSTR   valsReplPageSize[] = {pszReplPageSize, NULL};

    LDAPMod     mod[] =
    {
            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
            {LDAP_MOD_ADD, ATTR_SERVER_ID, {(PSTR*)valsServerId}},
            {LDAP_MOD_ADD, ATTR_INVOCATION_ID, {(PSTR*)valsInvocationId}},
            {LDAP_MOD_ADD, ATTR_REPL_INTERVAL, {(PSTR*)valsReplInterval}},
            {LDAP_MOD_ADD, ATTR_REPL_PAGE_SIZE, {(PSTR*)valsReplPageSize}}
    };

    LDAPMod*    attrs[] =
    {
            &mod[0], &mod[1], &mod[2], &mod[3], &mod[4], &mod[5], NULL
    };

    if (!pLd ||
        IsNullOrEmptyString(pszServerDN) ||
        IsNullOrEmptyString(pszCN) ||
        IsNullOrEmptyString(pszServerId) ||
        IsNullOrEmptyString(pszInvocationId) ||
        IsNullOrEmptyString(pszReplInterval) ||
        IsNullOrEmptyString(pszReplPageSize))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_add_ext_s(pLd, pszServerDN, attrs, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCreateReplAgrsContainer(pLd, pszServerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_VmDirCreateReplAgrsContainer(
    LDAP*   pLd,
    PCSTR   pszServerDN
    )
{
    DWORD   dwError = 0;
    PSTR    pszReplAgrsContainerDN = NULL;
    PCSTR   valsClass[] = {OC_CONTAINER, OC_TOP, NULL};
    PCSTR   valsCn[] = {VMDIR_REPL_AGRS_CONTAINER_NAME, NULL};

    LDAPMod     mod[] =
    {
            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}}
    };

    LDAPMod*    attrs[] =
    {
            &mod[0], &mod[1], NULL
    };

    if (!pLd || IsNullOrEmptyString(pszServerDN))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
            &pszReplAgrsContainerDN, "%s=%s,%s",
            ATTR_CN, VMDIR_REPL_AGRS_CONTAINER_NAME,
            pszServerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_add_ext_s(pLd, pszReplAgrsContainerDN, attrs, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    VMDIR_SAFE_FREE_MEMORY(pszReplAgrsContainerDN);
    return dwError;
}

static
DWORD
_VmDirPatchDSERoot(
    LDAP*   pLd,
    PCSTR   pszServerDN,
    PCSTR   pszDCAccountDN,
    PCSTR   pszDCAccountUPN
    )
{
    DWORD   dwError = 0;
    PCSTR   pszModAttrArr[] =
    {
            ATTR_SERVER_NAME,       pszServerDN,
            ATTR_DC_ACCOUNT_DN,     pszDCAccountDN,
            ATTR_DC_ACCOUNT_UPN,    pszDCAccountUPN,
            NULL
    };

    if (!pLd ||
        IsNullOrEmptyString(pszServerDN) ||
        IsNullOrEmptyString(pszDCAccountDN) ||
        IsNullOrEmptyString(pszDCAccountUPN))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapModReplAttributesValue(
            pLd, PERSISTED_DSE_ROOT_DN, pszModAttrArr);
    BAIL_ON_VMDIR_ERROR(dwError);

error:
    return dwError;
}
