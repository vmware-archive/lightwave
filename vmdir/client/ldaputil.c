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

extern PCSTR gGroupWhiteList[];

static
DWORD
_VmDirLdapSetupAccountMembership(
    LDAP*   pLd,
    PCSTR   pszDomainDN,
    PCSTR   pszBuiltinGroupName,
    PCSTR   pszAccountDN
    );

/*
 * Create dn for remote repl hostname using ldap search lookup
 * Sample of remote repl host DN:
 * cn=blisles-B,cn=Servers,cn=siteB,cn=Sites,cn=Configuration,dc=vmware,dc=com
 *
 */
DWORD
VmDirLdapCreateReplHostNameDN(
    PSTR* ppszReplHostNameDN,
    LDAP* pLd,
    PCSTR pszReplHostName
    )
{
    DWORD dwError = 0;
    PSTR pszReplHostNameDN = NULL;
    PSTR pszSearchFilter = NULL;
    LDAPMessage* searchRes = NULL;

    LDAPMessage* entry = NULL;
    PSTR pszDn = NULL;

    dwError = VmDirAllocateStringPrintf(
                  &pszSearchFilter,
                  "(&(cn=%s)(objectclass=vmwDirServer))",
                  pszReplHostName
                  );
    dwError = ldap_search_ext_s( pLd,
                                 "",
                                 LDAP_SCOPE_SUBTREE,
                                 pszSearchFilter,
                                 NULL, TRUE,
                                 NULL,                       // server ctrls
                                 NULL,                       // client ctrls
                                 NULL,                       // timeout
                                 0,                         // size limit,
                                 &searchRes
                               );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (1 != ldap_count_entries(pLd, searchRes))
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    entry = ldap_first_entry(pLd, searchRes);
    if (!entry)
    {
        dwError = ERROR_INVALID_ENTRY;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszDn = ldap_get_dn(pLd, entry);
    if (IsNullOrEmptyString(pszDn))
    {
        dwError = ERROR_INVALID_DN;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszDn, &pszReplHostNameDN);
    BAIL_ON_VMDIR_ERROR(dwError);
    *ppszReplHostNameDN = pszReplHostNameDN;

cleanup:
    ldap_memfree(pszDn);
    ldap_msgfree(searchRes);
    VMDIR_SAFE_FREE_MEMORY(pszSearchFilter);
    return dwError;

error:
    *ppszReplHostNameDN = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszReplHostNameDN);
    goto cleanup;
}

/*
 * Used in VmDirCreateCMSubtree
 */
static
DWORD
VmDirAddCMSiteNode(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszSiteGUID)
{
    DWORD       dwError = 0;
    PSTR        pszDN = NULL;

    PCSTR       valsCn[] = {pszSiteGUID, NULL};
    PCSTR       valsClass[] = {CM_OBJECTCLASS_SITE, NULL};
    PCSTR       valsDisname[] = {CM_DISPLAYNAME_SITE, NULL};

    LDAPMod     mod[3]={
                            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
                            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
                            {LDAP_MOD_ADD, ATTR_DISPLAY_NAME, {(PSTR*)valsDisname}}
                       };
    LDAPMod*    attrs[] = {&mod[0], &mod[1], &mod[2], NULL};
    PSTR        pszDomainDN = NULL;

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszDN,
                                            "cn=%s,cn=%s,cn=%s,%s",
                                            pszSiteGUID,
                                            CM_SITE,
                                            CM_COMPONENTMANAGER,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Adding %s.", pszDN);

    dwError = ldap_add_ext_s(
                             pLd,
                             pszDN,
                             attrs,
                             NULL,
                             NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirAddCMSiteNode failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * Used in VmDirCreateCMSubtree
 */
static
DWORD
VmDirAddLduNode(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszSiteGUID,
    PCSTR pszLduGUID)
{
    DWORD       dwError = 0;
    PSTR        pszDN = NULL;
    PCSTR       valsCn[] = {pszLduGUID, NULL};
    PCSTR       valsClass[] = {CM_OBJECTCLASS_LDU, NULL};
    PCSTR       valsDisname[] = {NULL, NULL};
    PCSTR       valsSite[] = {pszSiteGUID, NULL};
    PSTR        pszDisName = NULL;

    LDAPMod     mod[]={
                            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
                            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
                            {LDAP_MOD_ADD, ATTR_SITE_ID, {(PSTR*)valsSite}},
                            {LDAP_MOD_ADD, ATTR_DISPLAY_NAME, {(PSTR*)valsDisname}}
                       };
    LDAPMod*    attrs[] = {&mod[0], &mod[1], &mod[2], &mod[3], NULL};
    PSTR        pszDomainDN = NULL;

    dwError = VmDirAllocateStringPrintf(&pszDisName,
                                            "Deployment %s",
                                            pszLduGUID);
    valsDisname[0] = pszDisName;

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszDN,
                                            "cn=%s,cn=%s,cn=%s,%s",
                                            pszLduGUID,
                                            CM_LDUS,
                                            CM_COMPONENTMANAGER,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Adding %s.", pszDN);

    dwError = ldap_add_ext_s(
                             pLd,
                             pszDN,
                             attrs,
                             NULL,
                             NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszDisName);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirAddLduNode failed. Error(%u)", dwError);
    goto cleanup;
}

static
BOOL
VmDirIsSolutionUser(
    PCSTR   pszUserDN
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainName = NULL;
    PSTR    pszDomainDN = NULL;
    CHAR    pszLduGuid[VMDIR_GUID_STR_LEN] = {0};
    PSTR    pszLduDN = NULL;

    dwError = VmDirGetDomainName(
                            "localhost",
                            &pszDomainName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetLocalLduGuid(pszLduGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszLduDN,
                                            "cn=%s,cn=%s,cn=%s,%s",
                                            pszLduGuid,
                                            CM_LDUS,
                                            CM_COMPONENTMANAGER,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (VmDirCaselessStrStrA(pszUserDN, pszLduDN))
    {
        dwError = 0;
    }
    else
    {
        dwError = 1;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszLduDN);
    return dwError == 0;

error:
    goto cleanup;
}

static
DWORD
VmDirLdapGetAttributeValues(
    LDAP* pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    LDAPControl** ppSrvCtrls,
    BerValue*** pppBerValues
    )
{
    DWORD           dwError = 0;
    PCSTR           ppszAttrs[] = {pszAttribute, NULL};
    LDAPMessage*    pSearchRes = NULL;
    LDAPMessage*    pEntry = NULL;
    BerValue**      ppBerValues = NULL;

    dwError = ldap_search_ext_s(
                            pLd,
                            pszDN,
                            LDAP_SCOPE_BASE,
                            NULL,       /* filter */
                            (PSTR*)ppszAttrs,
                            FALSE,      /* attrsonly */
                            ppSrvCtrls, /* serverctrls */
                            NULL,       /* clientctrls */
                            NULL,       /* timeout */
                            0,         /* sizelimit */
                            &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    pEntry = ldap_first_entry(pLd, pSearchRes);
    if (!pEntry)
    {
        dwError = LDAP_NO_SUCH_ATTRIBUTE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerValues = ldap_get_values_len(pLd, pEntry, pszAttribute);
    if (!ppBerValues)
    {
        dwError = LDAP_NO_SUCH_ATTRIBUTE;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    *pppBerValues = ppBerValues;
cleanup:
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    return dwError;
error:
    *pppBerValues = NULL;
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLdapGetAttributeValues failed. Error(%u)", dwError);
    goto cleanup;
}

static
DWORD
VmDirLdapWriteAttributeValues(
    LDAP* pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR pszValue
    )
{
    DWORD       dwError = 0;
    LDAPMod     mod = {0};
    LDAPMod*    mods[2] = {&mod, NULL};
    PSTR        vals[2] = {(PSTR)pszValue, NULL};

    mod.mod_op = LDAP_MOD_ADD;
    mod.mod_type = (PSTR)pszAttribute;
    mod.mod_vals.modv_strvals = vals;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Add %s - %s:%s", pszDN, pszAttribute, pszValue);

    dwError = ldap_modify_ext_s(
                            pLd,
                            pszDN,
                            mods,
                            NULL,
                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapWriteAttributeValues failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * Helper function
 * Get first attribute from pEntry to pszAttrTarget which is CHAR array on stack
 * Used in VmDirMessagetoReplicationInfo
 */
static
DWORD
VmDirGetReplicationAttr(
    LDAP* pLd,
    LDAPMessage* pEntry,
    PCSTR pszAttribute,
    PSTR pszAttrTarget,
    DWORD dwAttrLen)
{
    DWORD       dwError = 0;
    BerValue**  ppBerValues = NULL;
    PSTR        pszDN = NULL;

    pszDN = ldap_get_dn(pLd, pEntry);
    assert(pszDN);

    dwError = VmDirLdapGetAttributeValues(
                                        pLd,
                                        pszDN,
                                        pszAttribute,
                                        NULL,
                                        &ppBerValues);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNCpyA(pszAttrTarget, dwAttrLen, ppBerValues[0]->bv_val, ppBerValues[0]->bv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pszDN)
    {
        ldap_memfree(pszDN);
    }

    if (ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetReplicationAttr failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * Helper function
 * Get replication information from pMessage to pInfo
 * Used in VmDirGetReplicationInfo
 */
static
DWORD
VmDirMessagetoReplicationInfo(
    LDAP* pLd,
    LDAPMessage* pEntry,
    PREPLICATION_INFO pInfo)
{
    DWORD       dwError = 0;
    BerElement* ber = NULL;
    PSTR        attr = NULL;

    for (attr = ldap_first_attribute(pLd, pEntry, &ber);
         attr;
         attr = ldap_next_attribute(pLd, pEntry, ber))
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Attribute:%s", attr);

        if (VmDirStringCompareA(attr, ATTR_LABELED_URI, FALSE) == 0)
        {
            dwError = VmDirGetReplicationAttr(pLd, pEntry, attr, pInfo->pszURI, VMDIR_MAX_LDAP_URI_LEN);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        ldap_memfree(attr);
        attr = NULL;
    }

cleanup:
    if (ber)
    {
        ber_free(ber,0);
    }
    return dwError;

error:
    if (attr)
    {
        ldap_memfree(attr);
    }

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirMessagetoReplicationInfo failed. Error(%u)", dwError);
    goto cleanup;
}

static
DWORD
VmDirMergeGroup(
    LDAP*   pSourceLd,
    LDAP*   pTargetLd,
    PCSTR   pszSourceDomainDN,
    PCSTR   pszTargetDomainDN,
    PCSTR   pszSourceGroupDN
    )
{
    DWORD           dwError = 0;
    PSTR            pszTargetGroupDN = NULL;
    BerValue**      ppBerValues = NULL;
    PSTR            pszGroupCN = NULL;
    int             i = 0;

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Merging group: %s", VDIR_SAFE_STRING(pszSourceGroupDN));

    dwError = VmDirGetTargetDN(pszSourceDomainDN, pszTargetDomainDN, pszSourceGroupDN, &pszTargetGroupDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VmDirIfDNExist(pTargetLd, pszTargetGroupDN))
    {
        dwError = VmDirDnLastRDNToCn(pszTargetGroupDN, &pszGroupCN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAddVmIdentityGroup(
                                        pTargetLd,
                                        pszGroupCN,
                                        pszTargetGroupDN
                                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapGetAttributeValues(
                                    pSourceLd,
                                    pszSourceGroupDN,
                                    ATTR_MEMBER,
                                    NULL,
                                    &ppBerValues);
    //if no members, we just return success
    if (dwError == LDAP_NO_SUCH_ATTRIBUTE)
    {
        dwError = ERROR_SUCCESS;
        goto cleanup;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; ppBerValues[i]; i++)
    {
        PCSTR pszUserDN = ppBerValues[i]->bv_val;
        //we only migrate solution users
        if (VmDirIsSolutionUser(pszUserDN))
        {
            VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Merging member: %s", pszUserDN);
            dwError = VmDirLdapWriteAttributeValues(
                                            pTargetLd,
                                            pszTargetGroupDN,
                                            ATTR_MEMBER,
                                            pszUserDN);
            //ignore error for conflict
            if (dwError == LDAP_TYPE_OR_VALUE_EXISTS)
            {
                dwError = ERROR_SUCCESS;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszGroupCN);
    VMDIR_SAFE_FREE_MEMORY(pszTargetGroupDN);
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    return dwError;
error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirMergeGroup failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * ####################### Shared Functions in libvmdirclient ########################
 */
/*
 * Used in VmDirCopyLDAPSubTree
 */
DWORD
VmDirGetTargetDN(
    PCSTR pszSourceBase,
    PCSTR pszTargetBase,
    PCSTR pszSourceDN,
    PSTR* ppszTargetDN
    )
{
    DWORD   dwError = 0;
    PSTR    pszTargetDN = NULL;
    SIZE_T  sizeSource = 0;
    SIZE_T  sizeTarget = 0;
    SIZE_T  sizeSourceDN = 0;
    SIZE_T  sizeTargetDN = 0;

    sizeSource = VmDirStringLenA(pszSourceBase);
    sizeTarget = VmDirStringLenA(pszTargetBase);
    sizeSourceDN = VmDirStringLenA(pszSourceDN);
    sizeTargetDN = sizeSourceDN - sizeSource + sizeTarget +1;

    dwError = VmDirAllocateMemory(sizeTargetDN, (PVOID*)&pszTargetDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNCpyA(pszTargetDN, sizeTargetDN, pszSourceDN, sizeSourceDN - sizeSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringCatA(pszTargetDN, sizeTargetDN, pszTargetBase);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszTargetDN = pszTargetDN;
cleanup:
    return dwError;

error:
    *ppszTargetDN = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszTargetDN);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetTargetDN failed. Error(%u)", dwError);
    goto cleanup;
}

BOOLEAN
VmDirIfDNExist(
    LDAP* pLd,
    PCSTR pszDN)
{
    DWORD           dwError = 0;
    LDAPMessage*    pSearchRes = NULL;

    dwError = ldap_search_ext_s(
                            pLd,
                            pszDN,         /* base */
                            LDAP_SCOPE_BASE,
                            NULL,       /* filter */
                            NULL,       /* attrs */
                            FALSE,      /* attrsonly */
                            NULL,       /* serverctrls */
                            NULL,       /* clientctrls */
                            NULL,       /* timeout */
                            0,          /* sizelimit */
                            &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    return dwError == 0;
error:
    goto cleanup;
}

DWORD
VmDirConnectLDAPServerByURI(
    LDAP**      ppLd,
    PCSTR       pszLdapURI,
    PCSTR       pszDomain,
    PCSTR       pszUserName,
    PCSTR       pszPassword
    )
{
    DWORD       dwError = 0;
    PSTR        pszHost = NULL;
    LDAP*       pLocalLd = NULL;

    if (ppLd == NULL || pszLdapURI == NULL || IsNullOrEmptyString(pszDomain) ||
            IsNullOrEmptyString(pszUserName) || pszPassword==NULL) //allows empty password
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapURI2Host( pszLdapURI, &pszHost );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(   &pLocalLd,
                                        pszHost,
                                        pszDomain,
                                        pszUserName,
                                        pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLocalLd;

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pszHost );

    return dwError;
error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirConnectLDAPServerByURI failed. Error(%u)(%s)",
                                        dwError, VDIR_SAFE_STRING( pszLdapURI));

    if ( pLocalLd )
    {
        ldap_unbind_ext_s(pLocalLd, NULL, NULL);
    }

    goto cleanup;
}

/*
 * connect to LDAP via srp or krb mechanism
 */
DWORD
VmDirConnectLDAPServer(
    LDAP**      ppLd,
    PCSTR       pszHostName,
    PCSTR       pszDomain,
    PCSTR       pszUserName,
    PCSTR       pszPassword
    )
{
    DWORD   dwError = 0;
    PSTR    pszUPN = NULL;
    LDAP*   pLocalLd = NULL;

    if (    ppLd == NULL                        ||
            pszHostName == NULL                 ||
            IsNullOrEmptyString(pszDomain)      ||
            IsNullOrEmptyString(pszUserName)    ||
            pszPassword == NULL   // allows empty password for krb scenario, i.e. "" is ok.
       )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszUPN, "%s@%s", pszUserName, pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind( &pLocalLd,
                                 pszHostName,
                                 pszUPN,
                                 pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLocalLd;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszUPN);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirConnectLDAPServer failed. (%u)(%s)", dwError, VDIR_SAFE_STRING(pszUPN));
    if ( pLocalLd )
    {
        ldap_unbind_ext_s(pLocalLd, NULL, NULL);
    }

    goto cleanup;
}

DWORD
VmDirGetSiteGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PSTR* ppszGUID
    )
{
    DWORD dwError = 0;
    PSTR  pszFilter = "(objectclass=*)";
    PSTR  pszSiteName = NULL;
    PSTR  pszSiteDN = NULL;
    PSTR  pszAttrObjectGUID = "objectGUID";
    PSTR  ppszAttrs[] = { pszAttrObjectGUID, NULL };
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR  pszGUID = NULL;

    dwError = VmDirGetSiteNameInternal(pLd, &pszSiteName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSiteDN(pszDomain, pszSiteName, &pszSiteDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszSiteDN,
                    LDAP_SCOPE_BASE,
                    pszFilter,
                    ppszAttrs,
                    FALSE, /* get values      */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) != 1) /* expect only one site object */
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLd, pEntry, pszAttrObjectGUID);

    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
                        &pszGUID,
                        "%s",
                        ppValues[0]->bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGUID = pszGUID;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMDIR_SAFE_FREE_MEMORY(pszSiteName);
    VMDIR_SAFE_FREE_MEMORY(pszSiteDN);

    return dwError;

error:

    if (ppszGUID)
    {
        *ppszGUID = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszGUID);

    goto cleanup;
}

DWORD
VmDirGetSiteNameInternal(
    LDAP* pLd,
    PSTR* ppszSiteName
    )
{
    DWORD dwError = 0;
    PSTR  pszSiteName = NULL;
    LDAPMessage *pResult = NULL;
    PSTR  pszAttrSiteName = "msDS-SiteName";
    PSTR  ppszAttrs[] = { pszAttrSiteName, NULL };
    PSTR  pszFilter = "(objectclass=*)";
    struct berval** ppValues = NULL;

    if (!pLd || !ppszSiteName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(
                pLd,
                NULL,
                LDAP_SCOPE_BASE,
                pszFilter,
                &ppszAttrs[0],
                FALSE, /* get values also */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) > 0)
    {
        LDAPMessage *pEntry = ldap_first_entry(pLd, pResult);

        if (pEntry)
        {
            ppValues = ldap_get_values_len(pLd, pEntry, pszAttrSiteName);

            if (ppValues && ldap_count_values_len(ppValues) > 0)
            {
                dwError = VmDirAllocateStringPrintf(
                                &pszSiteName,
                                "%s",
                                ppValues[0]->bv_val);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    if (IsNullOrEmptyString(pszSiteName))
    {
        dwError = ERROR_NO_SITENAME;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszSiteName = pszSiteName;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:

    if (ppszSiteName)
    {
        *ppszSiteName = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszSiteName);

    goto cleanup;
}

DWORD
VmDirGetSiteDN(
    PCSTR pszDomain,
    PCSTR pszSiteName,
    PSTR* ppszSiteDN
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PSTR  pszSiteDN = NULL;

    if (IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszSiteName) ||
        !ppszSiteDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszSiteDN,
                    "CN=%s,CN=Sites,CN=Configuration,%s",
                    pszSiteName,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszSiteDN = pszSiteDN;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    if (ppszSiteDN)
    {
        *ppszSiteDN = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszSiteDN);

    goto cleanup;
}

/*
 * Used in VmDirCreateCMSubtree
 */
DWORD
VmDirAddVmIdentityContainer(
    LDAP* pLd,
    PCSTR pszCN,
    PCSTR pszDN
    )
{
    DWORD       dwError = 0;
    PCSTR       valsCn[] = {pszCN, NULL};
    PCSTR       valsClass[] = {OC_TOP, OC_CONTAINER, NULL};
    LDAPMod     mod[2]={
                            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
                            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}}
                       };
    LDAPMod*    attrs[] = {&mod[0], &mod[1], NULL};

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Adding %s.", pszDN);

    dwError = ldap_add_ext_s(
                             pLd,
                             pszDN,
                             attrs,
                             NULL,
                             NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirAddVmIdentityContainer failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * Get all replication info of a server (pszHost) from pLd
 * 1. server entry DN: cn=SERVER_NAME,cn=server,cn=SITE_NAME,cn=sites,cn=configuration,DOMAIN_DN
 * 2. find all replication agreement entries under cn=SITE_NAME,cn=sites,cn=configuration,DOMAIN_DN
 * 3. filter out SERVER_NAME == pszHost
 */
DWORD
VmDirGetReplicationInfo(
    LDAP* pLd,
    PCSTR pszHost,
    PCSTR pszDomain,
    PREPLICATION_INFO* ppReplicationInfo,
    DWORD* pdwInfoCount
    )
{
    DWORD               dwError = 0;
    PSTR                pszSearchBaseDN = NULL;
    PCSTR               ppszAttrs[] = {ATTR_LABELED_URI, NULL};
    LDAPMessage*        pSearchRes = NULL;
    LDAPMessage*        pEntry = NULL;
    PREPLICATION_INFO   pReplicationInfo = NULL;
    int                 i = 0;
    DWORD               dwInfoCount = 0;
    PSTR                pszDomainDN = NULL;
    PSTR                pszEntryDN = NULL;
    PSTR                pszHostMatch = NULL;

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszSearchBaseDN,
                                            "cn=Sites,cn=Configuration,%s",
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                             pLd,
                             pszSearchBaseDN,
                             LDAP_SCOPE_SUBTREE,
                             "objectclass=vmwReplicationAgreement", /* filter */
                             (PSTR*)ppszAttrs,  /* attrs[]*/
                             FALSE,
                             NULL,              /* serverctrls */
                             NULL,              /* clientctrls */
                             NULL,              /* timeout */
                             0,
                             &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwInfoCount = ldap_count_entries(pLd, pSearchRes);
    if (dwInfoCount > 0)
    {
        dwError = VmDirAllocateMemory(dwInfoCount*sizeof(REPLICATION_INFO), (PVOID*)&pReplicationInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(
                        &pszHostMatch,
                        "cn=Replication Agreements,cn=%s,cn=Servers,",
                        pszHost);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i=0, pEntry = ldap_first_entry(pLd, pSearchRes);
             pEntry;
             pEntry = ldap_next_entry(pLd, pEntry))
        {
            pszEntryDN = ldap_get_dn(pLd, pEntry);
            if (VmDirStringCaseStrA( pszEntryDN, pszHostMatch ))
            {
                dwError = VmDirMessagetoReplicationInfo(pLd, pEntry, &pReplicationInfo[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
                i++;
            }

            ldap_memfree(pszEntryDN);
            pszEntryDN=NULL;
        }
    }

    *ppReplicationInfo = pReplicationInfo;
    *pdwInfoCount = i;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszSearchBaseDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszHostMatch);

    ldap_memfree(pszEntryDN);
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }
    return dwError;

error:
    *ppReplicationInfo = NULL;
    *pdwInfoCount = 0;
    VmDirFreeMemory(pReplicationInfo);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetReplicationInfo failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * Find all RA DN that has pszHost as it replication parnter
 * examine labeledURI and find pszHost exists.
 * NOTE, this works only if labeledURI is created using consistent naming scheme.
 * i.e. pszHost = DCAccount (in registry)
 *
 */
DWORD
VmDirGetAllRAToHost(
    LDAP*   pLD,
    PCSTR   pszHost,
    PSTR**  pppRADNArray,
    DWORD*  pdwSize
    )
{
    DWORD               dwError = 0;
    PSTR                pszSiteDN = NULL;
    LDAPMessage*        pSearchRes = NULL;
    LDAPMessage*        pEntry = NULL;
    DWORD               dwCnt = 0;
    DWORD               dwSearchSize = 0;
    PSTR                pszDomain=NULL;
    PSTR*               ppLocalArray=NULL;
    PSTR                pszDomainDN = NULL;
    PSTR                pszEntryDN = NULL;

    if ( !pLD || !pszHost || !pppRADNArray || !pdwSize )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName( "localhost", &pszDomain );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszSiteDN,
                                            "cn=Sites,cn=Configuration,%s",
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                             pLD,
                             pszSiteDN,
                             LDAP_SCOPE_SUBTREE,
                             "objectclass=vmwReplicationAgreement", /* filter */
                             NULL,              /* attrs[]*/
                             FALSE,
                             NULL,              /* serverctrls */
                             NULL,              /* clientctrls */
                             NULL,              /* timeout */
                             0,
                             &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwSearchSize = ldap_count_entries(pLD, pSearchRes);
    if (dwSearchSize > 0)
    {
        dwError = VmDirAllocateMemory(dwSearchSize*sizeof(PSTR), (PVOID*)&ppLocalArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (dwCnt=0, pEntry = ldap_first_entry(pLD, pSearchRes);
             pEntry;
             pEntry = ldap_next_entry(pLD, pEntry))
        {
            PSTR    pszHostMatch = NULL;
            PSTR    pszRAMatch = NULL;

            pszEntryDN = ldap_get_dn(pLD, pEntry);
            // DN: labeledURI=ldap(s)://PARTNER_HOST:port,cn=Replication Agreements,cn=LOCAL_DCACCOUNT,.....
            pszHostMatch = VmDirStringCaseStrA( pszEntryDN, pszHost );
            if ( pszHostMatch )
            {
                pszRAMatch = VmDirStringCaseStrA( pszEntryDN, ",cn=Replication Agreements,cn=" );
                if ( pszRAMatch && (pszHostMatch < pszRAMatch) ) // pszHost match DN PARTNER_HOST section
                {
                    dwError = VmDirAllocateStringA( pszEntryDN, &(ppLocalArray[dwCnt]) );
                    BAIL_ON_VMDIR_ERROR(dwError);
                    dwCnt++;
                }
            }

            ldap_memfree(pszEntryDN);
            pszEntryDN=NULL;
        }
    }

    *pppRADNArray = ppLocalArray;
    *pdwSize      = dwCnt;
    ppLocalArray  = NULL;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszSiteDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);

    ldap_memfree(pszEntryDN);
    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }

    return dwError;

error:
    for (dwCnt=0; dwCnt < dwSearchSize; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY( ppLocalArray[dwCnt] );
    }
    VMDIR_SAFE_FREE_MEMORY(ppLocalArray);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetAllRAToHost failed. Error(%u)", dwError);

    goto cleanup;
}

static
DWORD
VmDirStoreLduGuidtoDC(
    LDAP* pLd,
    PCSTR pszDomainDn,
    PCSTR pszLduGuid)
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    char    pszHostName[VMDIR_MAX_HOSTNAME_LEN];

    // vdcpromo sets this key.
    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMDIR_REG_KEY_DC_ACCOUNT,
                                   pszHostName,
                                   sizeof(pszHostName)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszDN,
                                            "cn=%s,ou=%s,%s",
                                            pszHostName,
                                            VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
                                            pszDomainDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    //ATTR_LDU_GUID
    dwError = VmDirLdapModReplaceAttribute(
                                        pLd,
                                        pszDN,
                                        ATTR_LDU_GUID,
                                        pszLduGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirStoreLduGuidtoDC failed. Error(%u)", dwError);
    goto cleanup;
}
/*
 * Create Component Manager Subtree in directory, including ComponentManager, CMSites, Ldus, Site-Guid and Ldu-Guid
 * Store Ldu-GUID into DC object
 * Used in setupldu.c and split.c
 */
DWORD
VmDirCreateCMSubtree(
    void* pvLd,
    PCSTR pszDomain,
    PCSTR pszSiteGuid,
    PCSTR pszLduGuid)
{
    DWORD   dwError = 0;
    PSTR    pszDN = NULL;
    PSTR    pszDomainDN = NULL;
    LDAP* pLd = (LDAP *) pvLd;

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);

    //create "ComponentManager"
    dwError = VmDirAllocateStringPrintf(&pszDN,
                                            "cn=%s,%s",
                                            CM_COMPONENTMANAGER,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAddVmIdentityContainer(pLd, CM_COMPONENTMANAGER, pszDN);
    //if CM node already exists, ignore error. It happens for join scenario. Replication happens before running vdcsetupldu
    if (dwError == LDAP_ALREADY_EXISTS)
    {
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_SAFE_FREE_MEMORY(pszDN);

    //create "Ldus"
    dwError = VmDirAllocateStringPrintf(&pszDN,
                                            "cn=%s,cn=%s,%s",
                                            CM_LDUS,
                                            CM_COMPONENTMANAGER,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAddVmIdentityContainer(pLd, CM_LDUS, pszDN);
    //if Ldus node already exists, ignore
    if (dwError == LDAP_ALREADY_EXISTS)
    {
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_SAFE_FREE_MEMORY(pszDN);

    //create "CMSites"
    dwError = VmDirAllocateStringPrintf(&pszDN,
                                            "cn=%s,cn=%s,%s",
                                            CM_SITE,
                                            CM_COMPONENTMANAGER,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAddVmIdentityContainer(pLd, CM_SITE, pszDN);
    //if CMSites container node already exists, ignore
    if (dwError == LDAP_ALREADY_EXISTS)
    {
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_SAFE_FREE_MEMORY(pszDN);

    //add Site
    dwError = VmDirAddCMSiteNode(
                            pLd,
                            pszDomain,
                            pszSiteGuid);
    //if CMSites node already exists (created by first infrastructure vm), ignore
    if (dwError == LDAP_ALREADY_EXISTS)
    {
        dwError = ERROR_SUCCESS;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    //Add Ldu
    dwError = VmDirAddLduNode(
                            pLd,
                            pszDomain,
                            pszSiteGuid,
                            pszLduGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStoreLduGuidtoDC(
                                pLd,
                                pszDomainDN,
                                pszLduGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirCreateCMSubtree failed. Error(%u)", dwError);
    goto cleanup;
}


DWORD
VmDirGetServerName(
    PCSTR pszHostName,
    PSTR* ppszServerName)
{
    DWORD       dwError = 0;
    PSTR        pszHostURI = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszServerName = NULL;
    BerValue**  ppBerValues = NULL;

    if (IsNullOrEmptyString(pszHostName) || ppszServerName == NULL)
    {
        dwError =  LDAP_INVALID_SYNTAX;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirIsIPV6AddrFormat( pszHostName ) )
    {
        dwError = VmDirAllocateStringPrintf( &pszHostURI, "%s://[%s]:%d",
                                                 VMDIR_LDAP_PROTOCOL,
                                                 pszHostName,
                                                 DEFAULT_LDAP_PORT_NUM);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf( &pszHostURI, "%s://%s:%d",
                                                 VMDIR_LDAP_PROTOCOL,
                                                 pszHostName,
                                                 DEFAULT_LDAP_PORT_NUM);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAnonymousLDAPBind( &pLd, pszHostURI );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapGetAttributeValues(
                                pLd,
                                "",
                                ATTR_SERVER_NAME,
                                NULL,
                                &ppBerValues);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDnLastRDNToCn(ppBerValues[0]->bv_val, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszServerName = pszServerName;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszHostURI);

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    return dwError;
error:
    *ppszServerName = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszServerName);

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetServerName failed with error (%u)", dwError);
    goto cleanup;
}

/*
 * Setup one way replication agreement
 */
DWORD
VmDirLdapSetupRemoteHostRA(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszReplHostName,
    DWORD dwHighWatermark
    )
{
    DWORD       dwError = 0;
    PSTR        pszReplURI = NULL;
    PSTR        pszReplHostNameDN = NULL;
    PSTR        pszReplAgrDN = NULL;
    PSTR        pszLastLocalUsn = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszDomainDN = NULL;

    if ( VmDirIsIPV6AddrFormat( pszReplHostName ) )
    {
        dwError = VmDirAllocateStringPrintf( &pszReplURI, "%s://[%s]", VMDIR_LDAP_PROTOCOL, pszReplHostName);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf( &pszReplURI, "%s://%s", VMDIR_LDAP_PROTOCOL, pszReplHostName);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
                            &pLd,
                            pszHostName,
                            pszDomainName,
                            pszUsername,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapCreateReplHostNameDN(&pszReplHostNameDN, pLd, pszHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                                    &pszReplAgrDN,
                                    "labeledURI=%s,cn=%s,%s",
                                    pszReplURI,      // uri points back to local server
                                    VMDIR_REPL_AGRS_CONTAINER_NAME,
                                    pszReplHostNameDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszReplAgrDN > 0)
    {

        char* modv_agr[] = {OC_REPLICATION_AGREEMENT, OC_TOP, NULL};
        char* modv_uri[] = {pszReplURI, NULL};
        char* modv_usn[] = {VMDIR_DEFAULT_REPL_LAST_USN_PROCESSED, NULL};
        LDAPMod replAgreement = {0};
        LDAPMod replURI = {0};
        LDAPMod replUSN = {0};
        USN     lastLocalUsn = 0;

        LDAPMod* pReplAgrObjAttrs[] =
        {
                &replAgreement,
                &replURI,
                &replUSN,
                NULL
        };

        replAgreement.mod_op = LDAP_MOD_ADD;
        replAgreement.mod_type = ATTR_OBJECT_CLASS;
        replAgreement.mod_values = modv_agr;

        replURI.mod_op = LDAP_MOD_ADD;
        replURI.mod_type = ATTR_LABELED_URI;
        replURI.mod_values = modv_uri;

        if (dwHighWatermark == 0)
        {
            dwError = VmDirLdapGetHighWatermark(
                                        pLd,
                                        pszHostName,
                                        pszReplHostName,
                                        pszDomainName,
                                        pszUsername,
                                        pszPassword,
                                        &lastLocalUsn);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            lastLocalUsn = dwHighWatermark;
        }

        dwError = VmDirAllocateStringPrintf(
                                        &pszLastLocalUsn,
                                        "%lu",
                                        lastLocalUsn);
        BAIL_ON_VMDIR_ERROR(dwError);

        modv_usn[0] = pszLastLocalUsn;

        replUSN.mod_op = LDAP_MOD_ADD;
        replUSN.mod_type = ATTR_LAST_LOCAL_USN_PROCESSED;
        replUSN.mod_values = modv_usn;

        // and the ldap_add_ext_s is a synchronous call
        dwError = ldap_add_ext_s(pLd, pszReplAgrDN, &pReplAgrObjAttrs[0], NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszReplURI);
    VMDIR_SAFE_FREE_MEMORY(pszReplHostNameDN);
    VMDIR_SAFE_FREE_MEMORY(pszReplAgrDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszLastLocalUsn);

    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapSetupRemoteHostRA failed with error (%u)", dwError);
    goto cleanup;
}

/*
 * Remove one way replication agreement
 */
DWORD
VmDirLdapRemoveRemoteHostRA(
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszReplHostName
    )
{
    DWORD       dwError = 0;
    PSTR        pszReplURI = NULL;
    PSTR        pszReplHostNameDN = NULL;
    PSTR        pszReplAgrDN = NULL;

    LDAP*       pLd = NULL;
    PSTR        pszDomainDN = NULL;

    if ( VmDirIsIPV6AddrFormat( pszReplHostName ) )
    {
        dwError = VmDirAllocateStringPrintf( &pszReplURI, "%s://[%s]", VMDIR_LDAP_PROTOCOL, pszReplHostName);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf( &pszReplURI, "%s://%s", VMDIR_LDAP_PROTOCOL, pszReplHostName);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
                            &pLd,
                            pszHostName,
                            pszDomainName,
                            pszUsername,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapCreateReplHostNameDN(&pszReplHostNameDN, pLd, pszHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                                    &pszReplAgrDN,
                                    "labeledURI=%s,cn=%s,%s",
                                    pszReplURI,
                                    VMDIR_REPL_AGRS_CONTAINER_NAME,
                                    pszReplHostNameDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // the ldap_delete_ext_s is a synchronous call
    dwError = ldap_delete_ext_s(
                    pLd,
                    pszReplAgrDN,
                    NULL,
                    NULL
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszReplURI);
    VMDIR_SAFE_FREE_MEMORY(pszReplHostNameDN);
    VMDIR_SAFE_FREE_MEMORY(pszReplAgrDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapRemoveRemoteHostRA failed with error (%u)", dwError);
    goto cleanup;
}

#ifdef _WIN32

static  _TCHAR  RSA_SERVER_CERT[MAX_PATH];
static  _TCHAR  RSA_SERVER_KEY[MAX_PATH];

#endif

/*
 * Setup this Domain Controller account on the partner host
 */
DWORD
VmDirLdapSetupDCAccountOnPartner(
    PCSTR pszDomainName,
    PCSTR pszHostName,              // Partner host name
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszDCHostName             // Self host name
    )
{
    DWORD       dwError = 0;
    PSTR        pszDCDN = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszDomainDN = NULL;
    PBYTE       pByteDCAccountPasswd = NULL;
    DWORD       dwDCAccountPasswdSize = 0;
    PSTR        pszUPN = NULL;
    PSTR        pszUpperCaseDomainName = NULL;
    PSTR        pszLowerCaseDCHostName = NULL;
    BOOLEAN     bIsLocalLdapServer = TRUE;
    PCSTR       pszServerName = NULL;
    DWORD       dwRetries = 0;
    BOOLEAN     bAcctExists=FALSE;
    PSTR        pszSRPUPN = NULL;
    PSTR        pszMachineGUID = NULL;

    char* modv_oc[] = {OC_PERSON, OC_ORGANIZATIONAL_PERSON, OC_USER, OC_COMPUTER, OC_TOP, NULL};
    char* modv_cn[] = {(PSTR)pszDCHostName, NULL};
    char* modv_sam[] = {(PSTR)pszDCHostName, NULL};
    char* modv_upn[] = {(PSTR)NULL, NULL};
    char* modv_machine[] = {(PSTR)NULL, NULL};
    char* modv_PSCVersion[] = {(PSTR)NULL, NULL};

    BerValue    bvPasswd = {0};
    BerValue*   pbvPasswd[2] = { NULL, NULL};

    LDAPMod modObjectClass = {0};
    LDAPMod modCn = {0};
    LDAPMod modPwd = {0};
    LDAPMod modSamAccountName = {0};
    LDAPMod modUserPrincipalName = {0};
    LDAPMod modMachineGUID = {0};
    LDAPMod modPSCVersion = {0};

    LDAPMod* pDCMods[] =
    {
            &modObjectClass,
            &modCn,
            &modPwd,
            &modSamAccountName,
            &modUserPrincipalName,
            &modMachineGUID,
            &modPSCVersion,
            NULL
    };

    if (IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszDCHostName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    modObjectClass.mod_op = LDAP_MOD_ADD;
    modObjectClass.mod_type = ATTR_OBJECT_CLASS;
    modObjectClass.mod_values = modv_oc;

    modCn.mod_op = LDAP_MOD_ADD;
    modCn.mod_type = ATTR_CN;
    modCn.mod_values = modv_cn;

    modPwd.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    modPwd.mod_type = ATTR_USER_PASSWORD;
    modPwd.mod_bvalues = pbvPasswd;
    pbvPasswd[0] = &bvPasswd;

    modSamAccountName.mod_op = LDAP_MOD_ADD;
    modSamAccountName.mod_type = ATTR_SAM_ACCOUNT_NAME;
    modSamAccountName.mod_values = modv_sam;

    modMachineGUID.mod_op = LDAP_MOD_ADD;
    modMachineGUID.mod_type = ATTR_MACHINE_GUID;
    modMachineGUID.mod_values = modv_machine;

    modPSCVersion.mod_op = LDAP_MOD_ADD;
    modPSCVersion.mod_type = ATTR_PSC_VERSION;
    modPSCVersion.mod_values = modv_PSCVersion;
    modv_PSCVersion[0]= VDIR_PSC_VERSION;

    dwError = VmDirGenerateGUID(&pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_machine[0] = pszMachineGUID;

    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower( pszDCHostName, &pszLowerCaseDCHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszUPN, "%s@%s", pszLowerCaseDCHostName, pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszSRPUPN, "%s@%s", pszUsername, pszDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_upn[0] = pszUPN;

    modUserPrincipalName.mod_op = LDAP_MOD_ADD;
    modUserPrincipalName.mod_type = ATTR_KRB_UPN;
    modUserPrincipalName.mod_values = modv_upn;

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszDCDN, "%s=%s,%s=%s,%s", ATTR_CN, pszLowerCaseDCHostName,
                                             ATTR_OU, VMDIR_DOMAIN_CONTROLLERS_RDN_VAL, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( VmDirStringCompareA( pszHostName, pszDCHostName, FALSE ) != 0 )
    {   // BUGBUG, does not consider simple vs fqdn name scenario.
        // It handles case from VmDirSetupHostInstance and VmDirJoin
        bIsLocalLdapServer = FALSE;
    }

    // use localhost if ldap server is local, so NIMBUS will work as is.
    pszServerName = (bIsLocalLdapServer ? "localhost" : pszHostName);

    dwError = VmDirConnectLDAPServer( &pLd,
                                      pszServerName,
                                      pszDomainName,
                                      pszUsername,
                                      pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwRetries=0; dwRetries < VMDIR_MAX_PASSWORD_RETRIES; dwRetries++)
    {
        dwError = VmDirGeneratePassword( pszHostName, // partner host in DC join scenario
                                         pszSRPUPN,
                                         pszPassword,
                                         &pByteDCAccountPasswd,
                                         &dwDCAccountPasswdSize );
        BAIL_ON_VMDIR_ERROR(dwError);

        bvPasswd.bv_val = pByteDCAccountPasswd;
        bvPasswd.bv_len = dwDCAccountPasswdSize;

        if (!bAcctExists)
        {
            // add ComputerAccount
            dwError = ldap_add_ext_s(pLd, pszDCDN, &pDCMods[0], NULL, NULL);
            if (dwError == LDAP_ALREADY_EXISTS)
            {
                bAcctExists = TRUE;
            }
        }

        if (bAcctExists)
        {
            PCSTR pszModAttrAry[] = {ATTR_USER_PASSWORD,
                                     pByteDCAccountPasswd,
                                     ATTR_PSC_VERSION,
                                     VDIR_PSC_VERSION,
                                     NULL
                                    };
            // reset ComputerAccount password. NOTE pByteDCAccountPasswd is null terminated.
            dwError = VmDirLdapModReplAttributesValue(pLd, pszDCDN, &pszModAttrAry[0]);
        }

        if (dwError == LDAP_SUCCESS || dwError != LDAP_CONSTRAINT_VIOLATION)
        {
            break; // done or other unexpected error
        }

        // password LDAP_CONSTRAINT_VIOLATION retry again.
        VMDIR_SAFE_FREE_MEMORY(pByteDCAccountPasswd);
        dwDCAccountPasswdSize = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "DC account (%s) created (recycle %s)", pszDCDN, bAcctExists ? "T":"F");

    // add DCAccount into DCAdmins group
    dwError = _VmDirLdapSetupAccountMembership( pLd, pszDomainDN, VMDIR_DC_GROUP_NAME, pszDCDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Store DC account password in registry.
    dwError = VmDirConfigSetDCAccountInfo(
                    pszDCHostName,
                    pszDCDN,
                    pByteDCAccountPasswd,
                    dwDCAccountPasswdSize,
                    pszMachineGUID );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDCDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pByteDCAccountPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseDCHostName);
    VMDIR_SAFE_FREE_MEMORY(pszSRPUPN);
    VMDIR_SAFE_FREE_MEMORY(pszMachineGUID);

    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapSetupDCAccountOnPartner (%s) failed with error (%u)",
                    VDIR_SAFE_STRING(pszDCDN), dwError);
    goto cleanup;
}

/*
 * Setup this Computer account on the remote host
 */
DWORD
VmDirLdapSetupComputerAccount(
    PCSTR pszDomainName,
    PCSTR pszHostName,              // Remote host name
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszComputerHostName       // Self host name
    )
{
    DWORD       dwError = 0;
    PSTR        pszComputerDN = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszDomainDN = NULL;
    PBYTE       pByteAccountPasswd = NULL;
    DWORD       dwAccountPasswdSize = 0;
    PSTR        pszUPN = NULL;
    PSTR        pszUpperCaseDomainName = NULL;
    PSTR        pszLowerCaseComputerHostName = NULL;
    BOOLEAN     bIsLocalLdapServer = TRUE;
    PCSTR       pszServerName = NULL;
    PSTR        pszSiteGUID = NULL;
    PSTR        pszMachineGUID = NULL;
    DWORD       dwRetries = 0;
    BOOLEAN     bAcctExists = FALSE;
    PSTR        pszSRPUPN = NULL;

    char* modv_oc[] = {OC_PERSON, OC_ORGANIZATIONAL_PERSON, OC_USER, OC_COMPUTER, OC_TOP, NULL};
    char* modv_cn[] = {(PSTR)pszComputerHostName, NULL};
    char* modv_sam[] = {(PSTR)pszComputerHostName, NULL};
    char* modv_site[] = {(PSTR)NULL, NULL};
    char* modv_machine[] = {(PSTR)NULL, NULL};
    char* modv_upn[] = {(PSTR)NULL, NULL};

    BerValue    bvPasswd = {0};
    BerValue*   pbvPasswd[2] = { NULL, NULL};

    LDAPMod modObjectClass = {0};
    LDAPMod modCn = {0};
    LDAPMod modPwd = {0};
    LDAPMod modSamAccountName = {0};
    LDAPMod modUserPrincipalName = {0};
    LDAPMod modSiteGUID = {0};
    LDAPMod modMachineGUID = {0};

    LDAPMod* pDCMods[] =
    {
            &modObjectClass,
            &modCn,
            &modPwd,
            &modSamAccountName,
            &modUserPrincipalName,
            &modSiteGUID,
            &modMachineGUID,
            NULL
    };

    if (IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszComputerHostName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    modObjectClass.mod_op = LDAP_MOD_ADD;
    modObjectClass.mod_type = ATTR_OBJECT_CLASS;
    modObjectClass.mod_values = modv_oc;

    modCn.mod_op = LDAP_MOD_ADD;
    modCn.mod_type = ATTR_CN;
    modCn.mod_values = modv_cn;

    modPwd.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    modPwd.mod_type = ATTR_USER_PASSWORD;
    modPwd.mod_bvalues = pbvPasswd;
    pbvPasswd[0] = &bvPasswd;

    modSamAccountName.mod_op = LDAP_MOD_ADD;
    modSamAccountName.mod_type = ATTR_SAM_ACCOUNT_NAME;
    modSamAccountName.mod_values = modv_sam;

    modSiteGUID.mod_op = LDAP_MOD_ADD;
    modSiteGUID.mod_type = ATTR_SITE_GUID;
    modSiteGUID.mod_values = modv_site;

    modMachineGUID.mod_op = LDAP_MOD_ADD;
    modMachineGUID.mod_type = ATTR_MACHINE_GUID;
    modMachineGUID.mod_values = modv_machine;

    dwError = VmDirAllocateStringPrintf( &pszSRPUPN, "%s@%s", pszUsername, pszDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower(
                    pszComputerHostName,
                    &pszLowerCaseComputerHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszUPN,
                    "%s@%s",
                    pszLowerCaseComputerHostName,
                    pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_upn[0] = pszUPN;

    modUserPrincipalName.mod_op = LDAP_MOD_ADD;
    modUserPrincipalName.mod_type = ATTR_KRB_UPN;
    modUserPrincipalName.mod_values = modv_upn;

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszComputerDN,
                    "%s=%s,%s=%s,%s",
                    ATTR_CN,
                    pszLowerCaseComputerHostName,
                    ATTR_OU,
                    VMDIR_COMPUTERS_RDN_VAL,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( VmDirStringCompareA( pszHostName, pszComputerHostName, FALSE ) != 0 )
    {   // BUGBUG, does not consider simple vs fqdn name scenario.
        // It handles case from VmDirSetupHostInstance and VmDirJoin
        bIsLocalLdapServer = FALSE;
    }

    // use localhost if ldap server is local, so NIMBUS will work as is.
    pszServerName = (bIsLocalLdapServer ? "localhost" : pszHostName);

    dwError = VmDirConnectLDAPServer( &pLd,
                                      pszServerName,
                                      pszDomainName,
                                      pszUsername,
                                      pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSiteGuidInternal(pLd, pszDomainName, &pszSiteGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_site[0] = pszSiteGUID;

    dwError = VmDirGenerateGUID(&pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_machine[0] = pszMachineGUID;

    for (dwRetries=0; dwRetries < VMDIR_MAX_PASSWORD_RETRIES; dwRetries++)
    {
        dwError = VmDirGeneratePassword( pszServerName,
                                         pszSRPUPN,
                                         pszPassword,
                                         &pByteAccountPasswd,
                                         &dwAccountPasswdSize );
        BAIL_ON_VMDIR_ERROR(dwError);

        bvPasswd.bv_val = pByteAccountPasswd;
        bvPasswd.bv_len = dwAccountPasswdSize;

        if (!bAcctExists)
        {
            // add ComputerAccount
            dwError = ldap_add_ext_s(pLd, pszComputerDN, &pDCMods[0], NULL, NULL);
            if (dwError == LDAP_ALREADY_EXISTS)
            {
                bAcctExists = TRUE;
            }
        }

        if (bAcctExists)
        {
            // reset ComputerAccount password. NOTE pByteDCAccountPasswd is null terminated.
            dwError = VmDirLdapModReplaceAttribute( pLd, pszComputerDN, ATTR_USER_PASSWORD, pByteAccountPasswd );
        }

        if (dwError == LDAP_SUCCESS || dwError != LDAP_CONSTRAINT_VIOLATION)
        {
            break; // done or other unexpected error
        }

        // pasword LDAP_CONSTRAINT_VIOLATION retry again.
        VMDIR_SAFE_FREE_MEMORY(pByteAccountPasswd);
        dwAccountPasswdSize = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Computer account (%s) created (recycle %s)", pszComputerDN, bAcctExists ? "T":"F");

    // add Computer Account into DCClients group
    dwError = _VmDirLdapSetupAccountMembership( pLd, pszDomainDN, VMDIR_DCCLIENT_GROUP_NAME, pszComputerDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Store Computer account info in registry.
    dwError = VmDirConfigSetDCAccountInfo(
                    pszComputerHostName,
                    pszComputerDN,
                    pByteAccountPasswd,
                    dwAccountPasswdSize,
                    pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszComputerDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pByteAccountPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseComputerHostName);
    VMDIR_SAFE_FREE_MEMORY(pszSiteGUID);
    VMDIR_SAFE_FREE_MEMORY(pszSRPUPN);
    VMDIR_SAFE_FREE_MEMORY(pszMachineGUID);

    VmDirLdapUnbind(&pLd);

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapSetupComputerAccount (%s) failed with error (%u)",
                    VDIR_SAFE_STRING(pszComputerDN), dwError);
    goto cleanup;
}

/*
 * Remove this Computer account on the remote host
 */
DWORD
VmDirLdapRemoveComputerAccount(
    PCSTR pszDomainName,
    PCSTR pszHostName,              // Remote host name
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszComputerHostName       // Self host name
    )
{
    DWORD       dwError = 0;
    PSTR        pszComputerDN = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszDomainDN = NULL;
    PSTR        pszUPN = NULL;
    PSTR        pszUpperCaseDomainName = NULL;
    PSTR        pszLowerCaseComputerHostName = NULL;
    BOOLEAN     bIsLocalLdapServer = TRUE;
    PCSTR       pszServerName = NULL;

    if (IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszComputerHostName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }


    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower(
                    pszComputerHostName,
                    &pszLowerCaseComputerHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszUPN,
                    "%s@%s",
                    pszLowerCaseComputerHostName,
                    pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszComputerDN,
                    "%s=%s,%s=%s,%s",
                    ATTR_CN,
                    pszLowerCaseComputerHostName,
                    ATTR_OU,
                    VMDIR_COMPUTERS_RDN_VAL,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( VmDirStringCompareA( pszHostName, pszComputerHostName, FALSE ) != 0 )
    {   // BUGBUG, does not consider simple vs fqdn name scenario.
        // It handles case from VmDirSetupHostInstance and VmDirJoin
        bIsLocalLdapServer = FALSE;
    }

    // use localhost if ldap server is local, so NIMBUS will work as is.
    pszServerName = (bIsLocalLdapServer ? "localhost" : pszHostName);

    dwError = VmDirConnectLDAPServer( &pLd,
                                      pszServerName,
                                      pszDomainName,
                                      pszUsername,
                                      pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_delete_ext_s(pLd, pszComputerDN, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);


cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszComputerDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseComputerHostName);

    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapRemoveComputerAccount (%s) failed with error (%u)",
                    VDIR_SAFE_STRING(pszComputerDN), dwError);
    goto cleanup;
}


/*
 * Setup this Domain Controller's ldap service account on the partner host
 */
DWORD
VmDirLdapSetupServiceAccount(
    PCSTR pszDomainName,
    PCSTR pszHostName,          // Partner host name
    PCSTR pszUsername,
    PCSTR pszPassword,
    PCSTR pszServiceName,
    PCSTR pszDCHostName         // Self host name
    )
{
    DWORD       dwError = 0;
    PSTR        pszMSADN = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszDomainDN = NULL;
    PBYTE       pByteMSAPasswd = NULL;
    DWORD       dwMSAPasswdSize = 0;
    PSTR        pszUPN = NULL;
    PSTR        pszName = NULL;
    PSTR        pszUpperCaseDomainName = NULL;
    PSTR        pszLowerCaseDCHostName = NULL;
    BOOLEAN     bIsLocalLdapServer = TRUE;
    PCSTR       pszServerName = NULL;
    BOOLEAN     bAcctExists = FALSE;
    PSTR        pszSRPUPN = NULL;

    char* modv_oc[] = {OC_MANAGED_SERVICE_ACCOUNT, OC_ORGANIZATIONAL_PERSON, OC_USER, OC_COMPUTER,
                       OC_TOP, NULL};
    char* modv_cn[] = {(PSTR)NULL, NULL};
    char* modv_sam[] = {(PSTR)NULL, NULL};
    char* modv_upn[] = {(PSTR)NULL, NULL};
    BerValue    bvPasswd = {0};
    BerValue*   pbvPasswd[2] = { NULL, NULL};

    LDAPMod modObjectClass = {0};
    LDAPMod modCn = {0};
    LDAPMod modPwd = {0};
    LDAPMod modSamAccountName = {0};
    LDAPMod modUserPrincipalName = {0};

    LDAPMod* pDCMods[] =
    {
            &modObjectClass,
            &modCn,
            &modPwd,
            &modSamAccountName,
            &modUserPrincipalName,
            NULL
    };

    modObjectClass.mod_op = LDAP_MOD_ADD;
    modObjectClass.mod_type = ATTR_OBJECT_CLASS;
    modObjectClass.mod_values = modv_oc;

    dwError = VmDirAllocateStringPrintf( &pszName, "%s/%s", pszServiceName, pszDCHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_cn[0] = modv_sam[0] = pszName;

    modCn.mod_op = LDAP_MOD_ADD;
    modCn.mod_type = ATTR_CN;
    modCn.mod_values = modv_cn;

    modPwd.mod_op = LDAP_MOD_ADD | LDAP_MOD_BVALUES;
    modPwd.mod_type = ATTR_USER_PASSWORD;
    modPwd.mod_bvalues = &(pbvPasswd[0]);
    pbvPasswd[0] = &bvPasswd;

    modSamAccountName.mod_op = LDAP_MOD_ADD;
    modSamAccountName.mod_type = ATTR_SAM_ACCOUNT_NAME;
    modSamAccountName.mod_values = modv_sam;

    dwError = VmDirAllocateStringPrintf( &pszSRPUPN, "%s@%s", pszUsername, pszDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower( pszDCHostName, &pszLowerCaseDCHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszUPN, "%s/%s@%s", pszServiceName, pszLowerCaseDCHostName, pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    modv_upn[0] = pszUPN;

    modUserPrincipalName.mod_op = LDAP_MOD_ADD;
    modUserPrincipalName.mod_type = ATTR_KRB_UPN;
    modUserPrincipalName.mod_values = modv_upn;

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszMSADN, "%s=%s,%s=%s,%s", ATTR_CN, pszUPN,
                                             ATTR_CN, VMDIR_MSAS_RDN_VAL, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( VmDirStringCompareA( pszHostName, pszDCHostName, FALSE ) != 0 )
    {   // BUGBUG, does not consider simple vs fqdn name scenario.
        // It handles case from VmDirSetupHostInstance and VmDirJoin
        bIsLocalLdapServer = FALSE;
    }

    pszServerName = (bIsLocalLdapServer ? "localhost" : pszHostName);

    dwError = VmDirConnectLDAPServer( &pLd,
                                      pszServerName,
                                      pszDomainName,
                                      pszUsername,
                                      pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    while( TRUE )
    {
        dwError = VmDirGeneratePassword( pszHostName, // partner host in DC join scenario
                                         pszSRPUPN,
                                         pszPassword,
                                         (PBYTE*)(&pByteMSAPasswd),
                                         &dwMSAPasswdSize );
        BAIL_ON_VMDIR_ERROR(dwError);

        bvPasswd.bv_val = pByteMSAPasswd;
        bvPasswd.bv_len = dwMSAPasswdSize;

        // and the ldap_add_ext_s is a synchronous call
        dwError = ldap_add_ext_s(pLd, pszMSADN, &pDCMods[0], NULL, NULL);
        if ( dwError == LDAP_SUCCESS )
        {
            break;
        }
        else if ( dwError == LDAP_ALREADY_EXISTS )
        {
            bAcctExists = TRUE;

            // reset ServiceAccount password. NOTE pByteDCAccountPasswd is null terminted.
            dwError = VmDirLdapModReplaceAttribute( pLd, pszMSADN, ATTR_USER_PASSWORD, pByteMSAPasswd );
            if (dwError == LDAP_CONSTRAINT_VIOLATION)
            {
                VMDIR_SAFE_FREE_MEMORY(pByteMSAPasswd);
                dwMSAPasswdSize = 0;
                continue;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
        else if (dwError == LDAP_CONSTRAINT_VIOLATION)
        {
            VMDIR_SAFE_FREE_MEMORY(pByteMSAPasswd);
            dwMSAPasswdSize = 0;
            continue;
        }
        else
        {
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Service account (%s) created (recycle %s)", pszMSADN, bAcctExists ? "T":"F");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszMSADN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pByteMSAPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszName);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseDCHostName);
    VMDIR_SAFE_FREE_MEMORY(pszSRPUPN);

    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapSetupServiceAccountOnPartner (%s) failed with error (%u)",
                    VDIR_SAFE_STRING(pszMSADN), dwError);
    goto cleanup;
}

/*
 * Delete this Domain Controller account on the partner host
 */
DWORD
VmDirLdapDeleteDCAccountOnPartner(
    PCSTR   pszDomainName,
    PCSTR   pszHostName,              // Partner host name
    PCSTR   pszUsername,
    PCSTR   pszPassword,
    PCSTR   pszDCHostName,             // Self host name
    BOOLEAN bActuallyDelete
    )
{
    DWORD       dwError = 0;
    LDAP*       pLd = NULL;

    dwError = VmDirConnectLDAPServer( &pLd, pszHostName, pszDomainName, pszUsername, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapDeleteDCAccount( pLd, pszDomainName, pszDCHostName, bActuallyDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

    /* TBD: Optionally: Remove DC account password from registry.
    if (bActuallyDelete)
    {
        dwError = VmDirConfigSetDCAccountInfo( pszDCDN, pByteDCAccountPasswd, dwDCAccountPasswdSize );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    */

cleanup:

    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLdapDeleteDCAccountOnPartner failed with error (%u)",
                     dwError);
    goto cleanup;
}

/*
 * Delete a service account, that belongs to this Domain Controller, on the partner host
 */
DWORD
VmDirLdapDeleteServiceAccountOnPartner(
    PCSTR   pszDomainName,
    PCSTR   pszHostName,          // Partner host name
    PCSTR   pszUsername,
    PCSTR   pszPassword,
    PCSTR   pszServiceName,
    PCSTR   pszDCHostName,         // Self host name
    BOOLEAN bActuallyDelete
    )
{
    DWORD       dwError = 0;
    LDAP*       pLd = NULL;

    dwError = VmDirConnectLDAPServer( &pLd, pszHostName, pszDomainName, pszUsername, pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapDeleteServiceAccount(pLd, pszDomainName, pszServiceName, pszDCHostName, bActuallyDelete);
    BAIL_ON_VMDIR_ERROR(dwError);


cleanup:

    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapDeleteServiceAccountOnPartner failed with error (%u)",
                    dwError);
    goto cleanup;
}

DWORD
VmDirGetServerObjectDN(
    PCSTR pszServerName,
    PCSTR pszDomainName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszLotusServerObjectName,
    PSTR* ppszCurrentServerObjectDN
    )
{
    DWORD           dwError = 0;
    LDAP*           pLd = NULL;
    LDAPMessage*    pEntry = NULL;
    LDAPMessage*    pResult = NULL;
    PSTR            pszFilter = NULL;
    PSTR            pszEntryDN = NULL;
    PSTR            pszServerObjectDN = NULL;

    if (IsNullOrEmptyString(pszServerName)          ||
        IsNullOrEmptyString(pszDomainName)          ||
        IsNullOrEmptyString(pszUserName)            ||
        IsNullOrEmptyString(pszPassword)            ||
        IsNullOrEmptyString(pszLotusServerObjectName) ||
        ppszCurrentServerObjectDN == NULL
       )
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf( &pszFilter, "(&(cn=%s)(objectclass=%s))",
                                             pszLotusServerObjectName, OC_DIR_SERVER );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer( &pLd,
                                      pszServerName,
                                      pszDomainName,
                                      pszUserName,
                                      pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    "",
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    NULL,
                    FALSE, /* get values      */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    // should have either 0 or 1 result
    if (ldap_count_entries(pLd, pResult) > 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( (pEntry = ldap_first_entry(pLd, pResult)) != NULL )
    {
        pszEntryDN = ldap_get_dn(pLd, pEntry);

        dwError = VmDirAllocateStringA( pszEntryDN, &pszServerObjectDN );
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppszCurrentServerObjectDN = pszServerObjectDN;
        pszServerObjectDN = NULL;
    }


cleanup:

    ldap_memfree( pszEntryDN );
    ldap_msgfree( pResult );
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    VMDIR_SAFE_FREE_MEMORY(pszServerObjectDN);
    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed with error (%u)", __FUNCTION__, dwError);
    goto cleanup;
}

DWORD
VmDirMergeGroups(
    LDAP*   pSourceLd,
    LDAP*   pTargetLd,
    PCSTR   pszSourceDomainDN,
    PCSTR   pszTargetDomainDN
    )
{
    DWORD           dwError = 0;
    PSTR            pszGroupDN = NULL;
    int i = 0;

    while (gGroupWhiteList[i])
    {
        dwError = VmDirAllocateStringPrintf(&pszGroupDN, "%s,%s", gGroupWhiteList[i], pszSourceDomainDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (VmDirIfDNExist(pSourceLd, pszGroupDN))
        {
            dwError = VmDirMergeGroup(
                                pSourceLd,
                                pTargetLd,
                                pszSourceDomainDN,
                                pszTargetDomainDN,
                                pszGroupDN
                );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        VMDIR_SAFE_FREE_MEMORY(pszGroupDN);
        i++;
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszGroupDN);
    return dwError;
error:
    printf("VmDirMergeGroups failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirAddVmIdentityGroup(
    LDAP* pLd,
    PCSTR pszCN,
    PCSTR pszDN
    )
{
    DWORD       dwError = 0;
    PCSTR       valsCn[] = {pszCN, NULL};
    PCSTR       valsClass[] = {OC_GROUP, NULL};
    PCSTR       valsAccount[] = {pszCN, NULL};

    LDAPMod     mod[3]={
                            {LDAP_MOD_ADD, ATTR_CN, {(PSTR*)valsCn}},
                            {LDAP_MOD_ADD, ATTR_OBJECT_CLASS, {(PSTR*)valsClass}},
                            {LDAP_MOD_ADD, ATTR_SAM_ACCOUNT_NAME, {(PSTR*)valsAccount}}
                       };
    LDAPMod*    attrs[] = {&mod[0], &mod[1], &mod[2], NULL};

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "Adding %s.", pszDN);

    dwError = ldap_add_ext_s(
                             pLd,
                             pszDN,
                             attrs,
                             NULL,
                             NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirAddVmIdentityGroup failed. Error(%u)", dwError);
    goto cleanup;
}

static
DWORD
_VmDirGetDSERootAttribute(
    PCSTR pszHostName,
    PCSTR pszAttrName,
    PSTR* ppszAttrValue)
{
    DWORD       dwError = 0;
    PSTR        pszLocalHostURI = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszLocalAttrValue = NULL;
    BerValue**  ppBerValues = NULL;

    if (IsNullOrEmptyString(pszHostName) || IsNullOrEmptyString(pszAttrName) || ppszAttrValue == NULL)
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirIsIPV6AddrFormat( pszHostName ) )
    {
        dwError = VmDirAllocateStringPrintf( &pszLocalHostURI, "%s://[%s]:%d",
                                                 VMDIR_LDAP_PROTOCOL,
                                                 pszHostName,
                                                 DEFAULT_LDAP_PORT_NUM);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf( &pszLocalHostURI, "%s://%s:%d",
                                                 VMDIR_LDAP_PROTOCOL,
                                                 pszHostName,
                                                 DEFAULT_LDAP_PORT_NUM);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAnonymousLDAPBind( &pLd, pszLocalHostURI );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapGetAttributeValues(
                                pLd,
                                "",
                                pszAttrName,
                                NULL,
                                &ppBerValues);
    BAIL_ON_VMDIR_ERROR(dwError);

    // BUGBUG BUGBUG, what if binary data?
    dwError = VmDirAllocateStringA(ppBerValues[0]->bv_val, &pszLocalAttrValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAttrValue = pszLocalAttrValue;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalHostURI);

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    return dwError;
error:
    *ppszAttrValue = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszLocalAttrValue);

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirGetDSERootAttribute failed with error (%u)", dwError);
    goto cleanup;
}

/*
 * Public APIs
 */

DWORD
VmDirGetDomainDN(
    PCSTR pszHostName,
    PSTR* ppszDomainDN)
{
    return _VmDirGetDSERootAttribute(
                            pszHostName,
                            ATTR_ROOT_DOMAIN_NAMING_CONTEXT,
                            ppszDomainDN);
}

DWORD
VmDirGetServerDN(
    PCSTR pszHostName,
    PSTR* ppszServerDN
    )
{
    return _VmDirGetDSERootAttribute(
                            pszHostName,
                            ATTR_SERVER_NAME,
                            ppszServerDN);
}

DWORD
VmDirGetDomainName(
    PCSTR pszHostName,
    PSTR* ppszDomainName)
{
    DWORD dwError = 0;
    PSTR pszDomainDN = NULL;
    PSTR pszDomainName = NULL;

    dwError = VmDirGetDomainDN(pszHostName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDomainDNToName(pszDomainDN, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDomainName = pszDomainName;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    return dwError;
error:
    *ppszDomainName = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetDomainName failed with error (%u)", dwError);
    goto cleanup;

}

DWORD
VmDirGetPartnerSiteName(
    PCSTR pszHostName,
    PSTR* ppszSiteName
    )
{
    return _VmDirGetDSERootAttribute(
                            pszHostName,
                            ATTR_SITE_NAME,
                            ppszSiteName);
}

DWORD
VmDirGetAdminDN(
    PCSTR pszHostName,
    PSTR* ppszAdminDN)
{
    return _VmDirGetDSERootAttribute(
                            pszHostName,
                            ATTR_DEFAULT_ADMIN_DN,
                            ppszAdminDN);
}

// SUNG, Bad naming if this is public api.  VmDirAdminNameFromHostName?
DWORD
VmDirGetAdminName(
    PCSTR pszHostName,
    PSTR* ppszAdminName)
{
    DWORD dwError=0;
    PSTR pszAdminDN = NULL;
    PSTR pszAdminName = NULL;

    dwError = VmDirGetAdminDN(pszHostName, &pszAdminDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirDnLastRDNToCn(pszAdminDN, &pszAdminName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAdminName = pszAdminName;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszAdminDN);
    return dwError;
error:
    *ppszAdminName = NULL;
    VMDIR_SAFE_FREE_MEMORY(pszAdminName);

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirGetAdminName failed with error (%u)", dwError);
    goto cleanup;
}

/*
 * Delete the whole subtree in DIT
 * BUGBUG, need to handle big subtree scenario.
 */
DWORD
VmDirDeleteDITSubtree(
   LDAP*    pLD,
   PCSTR    pszDN
   )
{
    DWORD           dwError = 0;
    LDAPMessage*    pSearchRes = NULL;
    LDAPMessage*    pEntry = NULL;
    PSTR            pszEntryDN = NULL;
    int             iSize = 0;
    int             iCnt = 0;
    int             iDeleted = 0;
    PSTR*           ppDNArray = NULL;

    if ( !pLD || !pszDN || pszDN[0]=='\0' )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(
                                 pLD,
                                 pszDN,
                                 LDAP_SCOPE_SUBTREE,
                                 NULL,              /* filter */
                                 NULL,              /* attrs[]*/
                                 FALSE,
                                 NULL,              /* serverctrls */
                                 NULL,              /* clientctrls */
                                 NULL,              /* timeout */
                                 0,
                                 &pSearchRes);
    if (dwError == LDAP_NO_SUCH_OBJECT)
    {   // no such node exists, nothing to delete.
        dwError = 0;
        goto cleanup;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    iSize = ldap_count_entries( pLD, pSearchRes );
    dwError = VmDirAllocateMemory( sizeof(PSTR)*iSize, (PVOID*)&ppDNArray );
    BAIL_ON_VMDIR_ERROR(dwError);

    for ( pEntry = ldap_first_entry(pLD, pSearchRes), iCnt = 0;
          pEntry;
          pEntry = ldap_next_entry(pLD, pEntry), iCnt++)
    {
        pszEntryDN = ldap_get_dn(pLD, pEntry);
        dwError = VmDirAllocateStringA( pszEntryDN, &(ppDNArray[iCnt]) );
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_memfree( pszEntryDN );
        pszEntryDN = NULL;
    }

    // sort DN array by DN length
    qsort ( ppDNArray, iSize, sizeof( PSTR ), VmDirCompareStrByLen );

    // delete DN (order by length)
    for ( iCnt = iSize - 1, iDeleted = 0; iCnt >= 0; iCnt-- )
    {
        dwError = ldap_delete_ext_s( pLD, ppDNArray[iCnt], NULL, NULL );
        switch (dwError)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "Tried deleting (%s), No Such Object.", ppDNArray[iCnt]);
                dwError = LDAP_SUCCESS;
                break;

            case LDAP_SUCCESS:
                iDeleted++;
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "(%s) deleted successfully.", ppDNArray[iCnt]);
                break;

            default:
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "(%s) deletion failed, error (%d).", ppDNArray[iCnt], dwError);
                break;
        }
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirDeleteSubtree: subtree (%s) (%u) deleted", pszDN, iDeleted);

cleanup:

    for (iCnt = 0; iCnt < iSize; iCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY( ppDNArray[iCnt] );
    }
    VMDIR_SAFE_FREE_MEMORY(ppDNArray);

    ldap_memfree( pszEntryDN );
    ldap_msgfree( pSearchRes );

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirDeleteSubtree: subtree (%s) delete failed (%u)", pszDN, dwError);
    goto cleanup;
}

/*
 * Delete DC Account
 */
DWORD
VmDirLdapDeleteDCAccount(
    LDAP *pLd,
    PCSTR   pszDomainName,
    PCSTR   pszDCHostName,             // Self host name
    BOOLEAN bActuallyDelete
    )
{
    DWORD       dwError = 0;
    PSTR        pszDCDN = NULL;
    PSTR        pszDomainDN = NULL;
    PSTR        pszLowerCaseDCHostName = NULL;

    dwError = VmDirAllocASCIIUpperToLower( pszDCHostName, &pszLowerCaseDCHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszDCDN, "%s=%s,%s=%s,%s", ATTR_CN, pszLowerCaseDCHostName,
                                             ATTR_OU, VMDIR_DOMAIN_CONTROLLERS_RDN_VAL, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bActuallyDelete)
    {
        dwError = ldap_delete_ext_s(pLd, pszDCDN, NULL, NULL);
        switch (dwError)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Tried deleting DC account object (%s), No Such Object.", pszDCDN);
                dwError = LDAP_SUCCESS;
                break;

            case LDAP_SUCCESS:
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "DC account (%s) deleted", pszDCDN);
                break;

            default:
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "DC account object (%s) deletion failed, error (%d).", pszDCDN,
                                dwError);
                break;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Would have deleted DC account (%s)", pszDCDN);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDCDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseDCHostName);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLdapDeleteDCAccount (%s) failed with error (%u)",
                     VDIR_SAFE_STRING(pszDCDN), dwError);
    goto cleanup;
}

/*
 * Delete a service account, that belongs to this Domain Controller
 */
DWORD
VmDirLdapDeleteServiceAccount(
    LDAP    *pLd,
    PCSTR   pszDomainName,
    PCSTR   pszServiceName,
    PCSTR   pszDCHostName,         // Self host name
    BOOLEAN bActuallyDelete
    )
{
    DWORD       dwError = 0;
    PSTR        pszMSADN = NULL;
    PSTR        pszDomainDN = NULL;
    PSTR        pszUPN = NULL;
    PSTR        pszUpperCaseDomainName = NULL;
    PSTR        pszLowerCaseDCHostName = NULL;

    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower( pszDCHostName, &pszLowerCaseDCHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszUPN, "%s/%s@%s", pszServiceName, pszLowerCaseDCHostName, pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszMSADN, "%s=%s,%s=%s,%s", ATTR_CN, pszUPN,
                                             ATTR_CN, VMDIR_MSAS_RDN_VAL, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bActuallyDelete)
    {
        dwError = ldap_delete_ext_s(pLd, pszMSADN, NULL, NULL);
        switch (dwError)
        {
            case LDAP_NO_SUCH_OBJECT:
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Tried deleting Service account object (%s), No Such Object.",
                                pszMSADN);
                dwError = LDAP_SUCCESS;
                break;

            case LDAP_SUCCESS:
                VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Service account (%s) deleted", pszMSADN);
                break;

            default:
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Service account object (%s) deletion failed, error (%d).", pszMSADN,
                                dwError);
                break;
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Would have deleted the service account (%s)", pszMSADN);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszMSADN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseDCHostName);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapDeleteServiceAccount (%s) failed with error (%u)",
                    VDIR_SAFE_STRING(pszMSADN), dwError);
    goto cleanup;
}

/*
 * Add an account to a builtin group
 */
static
DWORD
_VmDirLdapSetupAccountMembership(
    LDAP*   pLd,
    PCSTR   pszDomainDN,
    PCSTR   pszBuiltinGroupName,
    PCSTR   pszAccountDN
    )
{
    DWORD       dwError = 0;
    LDAPMod     mod = {0};
    LDAPMod*    mods[2] = {&mod, NULL};
    PSTR        vals[2] = {(PSTR)pszAccountDN, NULL};
    PSTR        pszGroupDN = NULL;

    // set DomainControllerGroupDN
    dwError = VmDirAllocateStringPrintf( &pszGroupDN,
                                             "cn=%s,cn=%s,%s",
                                             pszBuiltinGroupName,
                                             VMDIR_BUILTIN_CONTAINER_NAME,
                                             pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    mod.mod_op = LDAP_MOD_ADD;
    mod.mod_type = (PSTR)ATTR_MEMBER;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_modify_ext_s(
                            pLd,
                            pszGroupDN,
                            mods,
                            NULL,
                            NULL);
    if ( dwError == LDAP_TYPE_OR_VALUE_EXISTS )
    {
        dwError = 0;    // already a member of group
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirLdapSetupAccountMembership (%s)", pszAccountDN);

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszGroupDN);

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapSetupAccountMembership failed. Error(%u)", dwError);

    goto cleanup;
}

/*
 * query single attribute value of a DN via ldap
 * "*ppByte" is NULL terminated.
 */
DWORD
VmDirLdapGetSingleAttribute(
    LDAP*   pLD,
    PCSTR   pszDN,
    PCSTR   pszAttr,
    PBYTE*  ppByte,
    DWORD*  pdwLen
    )
{
    DWORD           dwError=0;
    PBYTE           pLocalByte = NULL;
    BerValue**      ppBerValues = NULL;

    if ( !pLD || !pszDN || !pszAttr || !ppByte || !pdwLen )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapGetAttributeValues( pLD,
                                           pszDN,
                                           pszAttr,
                                           NULL,
                                           &ppBerValues);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( ppBerValues[0] == NULL || ppBerValues[0]->bv_val == NULL )
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( ppBerValues[1] != NULL )   // more than one attribute value
    {
        dwError = VMDIR_ERROR_INVALID_RESULT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateAndCopyMemory(
                            ppBerValues[0]->bv_val,
                            ppBerValues[0]->bv_len + 1,
                            (PVOID*)&pLocalByte);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppByte = pLocalByte;
    *pdwLen = (DWORD)ppBerValues[0]->bv_len;
    pLocalByte = NULL;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pLocalByte);
    if(ppBerValues)
    {
        ldap_value_free_len(ppBerValues);
    }

    return dwError;

error:

    goto cleanup;
}

/*
 * reset attribute value via ldap
 */
DWORD
VmDirLdapModReplaceAttribute(
    LDAP*   pLd,
    PCSTR   pszDN,
    PCSTR   pszAttribute,
    PCSTR   pszValue
    )
{
    DWORD       dwError = 0;
    LDAPMod     mod = {0};
    LDAPMod*    mods[2] = {&mod, NULL};
    PSTR        vals[2] = {(PSTR)pszValue, NULL};

    mod.mod_op = LDAP_MOD_REPLACE;
    mod.mod_type = (PSTR)pszAttribute;
    mod.mod_vals.modv_strvals = vals;

    dwError = ldap_modify_ext_s(
                            pLd,
                            pszDN,
                            mods,
                            NULL,
                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "_VmDirLdapModReplaceAttribute (%s)(%s)", pszDN, pszAttribute);

cleanup:

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirLdapModReplaceAttribute failed. Error(%u)", dwError);

    goto cleanup;
}

/*
 * Replace multiple attributes.
 * Each attribute has single string value.
 */
DWORD
VmDirLdapModReplAttributesValue(
    LDAP*   pLd,
    PCSTR   pszDN,
    PCSTR*  ppszAttValPair
    )
{
    DWORD       dwError = 0;
    DWORD       dwCnt = 0;
    LDAPMod*    pLdapMod = NULL;
    LDAPMod**   ppLdapMods = NULL;
    PSTR*       ppStrVal = NULL;

    for (dwCnt=0; ppszAttValPair[dwCnt*2] != NULL; dwCnt++) ;

    dwError = VmDirAllocateMemory( sizeof(*ppLdapMods)*(dwCnt+1), (PVOID)&ppLdapMods );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory( sizeof(*pLdapMod)*(dwCnt), (PVOID)&pLdapMod );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory( sizeof(*ppStrVal)*(dwCnt*2), (PVOID)&ppStrVal );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; ppszAttValPair[dwCnt*2] != NULL; dwCnt++)
    {
        pLdapMod[dwCnt].mod_op      = LDAP_MOD_REPLACE;
        pLdapMod[dwCnt].mod_type    = (PSTR) ppszAttValPair[dwCnt*2];

        ppStrVal[dwCnt*2]           = (PSTR) ppszAttValPair[dwCnt*2 + 1];
        pLdapMod[dwCnt].mod_vals.modv_strvals = ppStrVal+(dwCnt*2);

        ppLdapMods[dwCnt]           = pLdapMod+dwCnt;
    }

    dwError = ldap_modify_ext_s(
                pLd,
                pszDN,
                ppLdapMods,
                NULL,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pLdapMod);
    VMDIR_SAFE_FREE_MEMORY(ppLdapMods);
    VMDIR_SAFE_FREE_MEMORY(ppStrVal);

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirLdapModReplAttributesValue failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmDirGetDCContainerDN(
    PCSTR pszDomain,
    PSTR* ppszContainerDN
    )
{

    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PSTR  pszContainerDN = NULL;
    if (IsNullOrEmptyString(pszDomain) || !ppszContainerDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszContainerDN,
                    "OU=%s,%s",
                    VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszContainerDN = pszContainerDN;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    return dwError;

error:
    if (ppszContainerDN)
    {
        *ppszContainerDN = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pszContainerDN);
    goto cleanup;

}

DWORD
VmDirGetServerAccountDN(
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszServerDN
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PSTR  pszServerDN = NULL;

    if (IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppszServerDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszServerDN,
                    "CN=%s,OU=%s,%s",
                    pszMachineName,
                    VMDIR_DOMAIN_CONTROLLERS_RDN_VAL,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszServerDN = pszServerDN;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    if (ppszServerDN)
    {
        *ppszServerDN = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszServerDN);

    goto cleanup;
}

DWORD
VmDirGetServerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszGUID
    )
{
    DWORD dwError = 0;
    PSTR  pszFilter = "(objectclass=computer)";
    PSTR  pszAccountDN = NULL;
    PSTR  pszAttrMachineGUID = ATTR_VMW_MACHINE_GUID;
    PSTR  ppszAttrs[] = { pszAttrMachineGUID, NULL };
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR  pszGUID = NULL;

    dwError = VmDirGetServerAccountDN(pszDomain, pszMachineName, &pszAccountDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszAccountDN,
                    LDAP_SCOPE_BASE,
                    pszFilter,
                    ppszAttrs,
                    FALSE, /* get values      */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    //searching by name should yield just one
    if (ldap_count_entries(pLd, pResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLd, pEntry, pszAttrMachineGUID);

    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
                        &pszGUID,
                        "%s",
                        ppValues[0]->bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGUID = pszGUID;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);

    return dwError;

error:

    if (ppszGUID)
    {
        *ppszGUID = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszGUID);

    goto cleanup;
}

DWORD
VmDirSetServerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PCSTR pszGUID
    )
{
    DWORD dwError = 0;
    PCSTR pszAttrObjectGUID = ATTR_VMW_MACHINE_GUID;
    PSTR  pszAccountDN = NULL;

    dwError = VmDirGetServerAccountDN(pszDomain, pszMachineName, &pszAccountDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapModReplaceAttribute(
                    pLd,
                    pszAccountDN,
                    pszAttrObjectGUID,
                    pszGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirGetComputerAccountDN(
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszAccountDN
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PSTR  pszAccountDN = NULL;

    if (IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppszAccountDN)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszAccountDN,
                    "CN=%s,OU=%s,%s",
                    pszMachineName,
                    VMDIR_COMPUTERS_RDN_VAL,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAccountDN = pszAccountDN;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    return dwError;

error:

    if (ppszAccountDN)
    {
        *ppszAccountDN = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);

    goto cleanup;
}

DWORD
VmDirGetComputerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PSTR* ppszGUID
    )
{
    DWORD dwError = 0;
    PSTR  pszFilter = "(objectclass=computer)";
    PSTR  pszAccountDN = NULL;
    PSTR  pszAttrMachineGUID = ATTR_VMW_MACHINE_GUID;
    PSTR  ppszAttrs[] = { pszAttrMachineGUID, NULL };
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR  pszGUID = NULL;

    dwError = VmDirGetComputerAccountDN(
                pszDomain,
                pszMachineName,
                &pszAccountDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pLd,
                    pszAccountDN,
                    LDAP_SCOPE_BASE,
                    pszFilter,
                    ppszAttrs,
                    FALSE, /* get values      */
                    NULL,  /* server controls */
                    NULL,  /* client controls */
                    NULL,  /* timeout         */
                    0,     /* size limit      */
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    //searching by name should yield just one
    if (ldap_count_entries(pLd, pResult) != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pLd, pResult);
    if (!pEntry)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pLd, pEntry, pszAttrMachineGUID);

    if (!ppValues || (ldap_count_values_len(ppValues) != 1))
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(
                        &pszGUID,
                        "%s",
                        ppValues[0]->bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGUID = pszGUID;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);

    return dwError;

error:

    if (ppszGUID)
    {
        *ppszGUID = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszGUID);

    goto cleanup;
}

DWORD
VmDirSetComputerGuidInternal(
    LDAP* pLd,
    PCSTR pszDomain,
    PCSTR pszMachineName,
    PCSTR pszGUID
    )
{
    DWORD dwError = 0;
    PCSTR pszAttrObjectGUID = ATTR_VMW_MACHINE_GUID;
    PSTR  pszAccountDN = NULL;

    dwError = VmDirGetComputerAccountDN(
                pszDomain,
                pszMachineName,
                &pszAccountDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapModReplaceAttribute(
                    pLd,
                    pszAccountDN,
                    pszAttrObjectGUID,
                    pszGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirGetMemberships(
    PVMDIR_CONNECTION pConnection,
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    )
{
    DWORD dwError = 0;
    PSTR pszFilter = NULL;
    PSTR pszAttrMemberOf = ATTR_MEMBEROF; // memberOf
    PSTR  ppszAttrs[] = { pszAttrMemberOf, NULL};
    DWORD dwCount = 0;
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct berval** ppValues = NULL;
    PSTR *ppszMemberships = NULL;
    DWORD dwMemberships = 0;
    DWORD i = 0;

    if (pConnection == NULL ||
        pConnection->pLd == NULL ||
        IsNullOrEmptyString(pszUPNName) ||
        pppszMemberships == NULL ||
        pdwMemberships == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszFilter, "(%s=%s)", ATTR_KRB_UPN, pszUPNName); // userPrincipalName
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                    pConnection->pLd,
                    "",
                    LDAP_SCOPE_SUBTREE,
                    pszFilter,
                    (PSTR*)ppszAttrs,
                    0,
                    NULL,
                    NULL,
                    NULL,
                    -1,
                    &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwCount = ldap_count_entries(pConnection->pLd, pResult);
    if (dwCount == 0)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (dwCount > 1)
    {
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = ldap_first_entry(pConnection->pLd, pResult);
    if (!pEntry)
    {
        dwError = LDAP_NO_SUCH_OBJECT;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppValues = ldap_get_values_len(pConnection->pLd, pEntry, pszAttrMemberOf);
    if (!ppValues)
    {
        dwMemberships = 0;
    }
    else
    {
        dwMemberships = ldap_count_values_len(ppValues);
    }

    if (dwMemberships)
    {
        dwError = VmDirAllocateMemory(dwMemberships * sizeof(PSTR), (PVOID)&ppszMemberships);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (i = 0; ppValues[i] != NULL; i++)
        {
            PCSTR pszMemberOf = ppValues[i]->bv_val;

            dwError = VmDirAllocateStringA(pszMemberOf, &ppszMemberships[i]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *pppszMemberships = ppszMemberships;
    *pdwMemberships = dwMemberships;

cleanup:

    if(ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VMDIR_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:
    if (ppszMemberships != NULL && dwMemberships > 0)
    {
        for (i = 0; i < dwMemberships; i++)
        {
            VMDIR_SAFE_FREE_STRINGA(ppszMemberships[i]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppszMemberships);
    }
    goto cleanup;
}

DWORD
VmDirGetReplicateCycleCountInternal(
    PVMDIR_CONNECTION   pConnection,
    DWORD*              pdwCycleCount
    )
{
    DWORD           dwError = 0;
    LDAPMessage*    pResult = NULL;
    struct berval** ppValues = NULL;
    PSTR            ppszAttrs[] = { ATTR_SERVER_RUNTIME_STATUS, NULL };
    DWORD           dwCycleCount = 0;

    if (pConnection == NULL         ||
        pConnection->pLd == NULL    ||
        pdwCycleCount == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(
                                pConnection->pLd,
                                REPLICATION_STATUS_DN,
                                LDAP_SCOPE_BASE,
                                "objectclass=*",   /* filter */
                                &ppszAttrs[0],     /* attrs[]*/
                                FALSE,
                                NULL,              /* serverctrls */
                                NULL,              /* clientctrls */
                                NULL,              /* timeout */
                                0,
                                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pConnection->pLd, pResult) == 1)
    {
        int             iCnt = 0;
        LDAPMessage*    pEntry = ldap_first_entry(pConnection->pLd, pResult);

        if (pEntry)
        {
            PSTR    pszTmp = NULL;
            BOOLEAN bHasCount = FALSE;

            ppValues = ldap_get_values_len(pConnection->pLd, pEntry, ATTR_SERVER_RUNTIME_STATUS);
            for (iCnt=0; ppValues && iCnt < ldap_count_values_len(ppValues); iCnt++)
            {
                if ( (pszTmp = VmDirStringStrA( ppValues[iCnt]->bv_val, REPL_STATUS_CYCLE_COUNT )) != NULL )
                {
                    errno = 0;
                    dwCycleCount = strtol( pszTmp + REPL_STATUS_CYCLE_COUNT_LEN, NULL, 10 );
                    if ( errno )
                    {
                        dwError = VMDIR_ERROR_BAD_ATTRIBUTE_DATA;
                        BAIL_ON_VMDIR_ERROR(dwError);
                    }

                    bHasCount = TRUE;
                    break;
                }
            }

            if ( bHasCount == FALSE )
            {
                dwError = VMDIR_ERROR_NOT_FOUND;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }
    else
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwCycleCount = dwCycleCount;

cleanup:
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirSetDomainFuncLvlInternal(
    LDAP* pLd,
    PCSTR pszDomainName,
    DWORD dwFuncLvl
    )
{
    DWORD  dwError = 0;
    PSTR pszDomainDN = NULL;
    PSTR   ppszVals [] = { NULL, NULL };
    LDAPMod addReplace = { 0 };
    LDAPMod *mods[2] = { 0 };
    PSTR pszFuncLvl = NULL;

    // Valid LDAP?
    if (!pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Get the value into place.
    dwError = VmDirAllocateStringPrintf(
                  &pszFuncLvl,
                  "%d",
                  dwFuncLvl
                  );
    BAIL_ON_VMDIR_ERROR(dwError);

    ppszVals[0] = pszFuncLvl;

    /* Initialize the attribute, specifying 'REPLACE' as the operation */
    addReplace.mod_op     = LDAP_MOD_REPLACE;
    addReplace.mod_type   = ATTR_DOMAIN_FUNCTIONAL_LEVEL;
    addReplace.mod_values = ppszVals;

    /* Fill the attributes array (remember it must be NULL-terminated) */
    mods[0] = &addReplace;
    mods[1] = NULL;

    // Get the DomainDN
    dwError = VmDirSrvCreateDomainDN(
                  pszDomainName,
                  &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_modify_ext_s(pLd, pszDomainDN, mods, NULL, NULL);

    if (dwError)
    {

        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Failed to set domain functional "
			"level to %s, error (%d)\n", pszFuncLvl, dwError);
    }
    else
    {
	VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Update domain functional level to "
		       "%s.\n", pszFuncLvl);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszFuncLvl);
    return dwError;

error:
    goto cleanup;
}

/*
 * Return a list of DC names in the federation.
 */
DWORD
VmDirGetAllDCInternal(
    LDAP*   pLd,
    PCSTR   pszDomainName,
    PVMDIR_STRING_LIST* ppStrList
    )
{
    DWORD           dwError = 0;
    PCSTR           pszFilter = "objectclass=computer";
    PCSTR           pszAttrCN = ATTR_CN;
    PCSTR           ppszAttrs[] = { pszAttrCN, NULL };
    PSTR            pszTmp = NULL;
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    PSTR            pszBaseDN = NULL;
    PSTR            pszDomainDN = NULL;
    PVMDIR_STRING_LIST  pStrList = NULL;
    struct berval**     ppValues = NULL;

    if (!pLd || !pszDomainName || !ppStrList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize( &pStrList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                &pszBaseDN, "%s=%s,%s",
                ATTR_OU, VMDIR_DOMAIN_CONTROLLERS_RDN_VAL, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                pLd,
                pszBaseDN,
                LDAP_SCOPE_ONELEVEL,
                pszFilter,
                (PSTR*)ppszAttrs,
                FALSE, /* get values also */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    for ( pEntry = ldap_first_entry(pLd, pResult);
          pEntry;
          pEntry = ldap_next_entry(pLd, pEntry))
    {
        if (ppValues)
        {
            ldap_value_free_len(ppValues);
        }
        VMDIR_SAFE_FREE_MEMORY(pszTmp);

        ppValues = ldap_get_values_len(pLd, pEntry, pszAttrCN);

        if (ppValues && ldap_count_values_len(ppValues) > 0)
        {
            dwError = VmDirAllocateStringA(ppValues[0]->bv_val, &pszTmp);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pStrList, pszTmp);
            BAIL_ON_VMDIR_ERROR(dwError);
            pszTmp = NULL;
        }
    }

    *ppStrList = pStrList;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszTmp);
    VMDIR_SAFE_FREE_MEMORY(pszBaseDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:
    VmDirStringListFree(pStrList);
    goto cleanup;
}

DWORD
VmDirGetPSCVersionInternal(
    LDAP* pLd,
    PSTR* ppszPSCVer
    )
{
    DWORD dwError = 0;
    PSTR pszPSCVer = NULL;
    LDAPMessage *pResult = NULL;
    PCSTR  pszAttrPSCVer = ATTR_PSC_VERSION;
    PCSTR  ppszAttrs[] = { pszAttrPSCVer, NULL };
    PCSTR  pszFilter = "objectclass=*";
    struct berval** ppValues = NULL;

    if (!pLd || !ppszPSCVer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = ldap_search_ext_s(
                pLd,
                "",
                LDAP_SCOPE_BASE,
                pszFilter,
                (PSTR*)ppszAttrs,
                FALSE, /* get values also */
                NULL,  /* server controls */
                NULL,  /* client controls */
                NULL,  /* timeout         */
                0,     /* size limit      */
                &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ldap_count_entries(pLd, pResult) > 0)
    {
        LDAPMessage *pEntry = ldap_first_entry(pLd, pResult);

        if (pEntry)
        {
            ppValues = ldap_get_values_len(pLd, pEntry, pszAttrPSCVer);

            if (ppValues && ldap_count_values_len(ppValues) > 0)
            {

                dwError = VmDirAllocateStringA(ppValues[0]->bv_val,
                                   &pszPSCVer);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    if (IsNullOrEmptyString(pszPSCVer))
    {
        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                   "%s not found, defaulting to %s",
                   ATTR_PSC_VERSION,
                   VMDIR_DFL_5_5);

        dwError = VmDirAllocateStringA(VMDIR_DFL_5_5,
                           &pszPSCVer);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppszPSCVer = pszPSCVer;

cleanup:

    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }

    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    return dwError;

error:

    if (ppszPSCVer)
    {
        *ppszPSCVer = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszPSCVer);
    goto cleanup;
}

VOID
VmDirFreeDCVersionInfo(
    PVMDIR_DC_VERSION_INFO pDCVerInfo
    )
{
    DWORD dwCnt = 0;

    if (pDCVerInfo)
    {
	for (dwCnt=0 ; dwCnt < pDCVerInfo->dwSize ; dwCnt++)
	{
	    VMDIR_SAFE_FREE_MEMORY(pDCVerInfo->ppszServer[dwCnt]);
	    VMDIR_SAFE_FREE_MEMORY(pDCVerInfo->ppszVersion[dwCnt]);
	}

	VMDIR_SAFE_FREE_MEMORY(pDCVerInfo->ppszServer);
	VMDIR_SAFE_FREE_MEMORY(pDCVerInfo->ppszVersion);
	VMDIR_SAFE_FREE_MEMORY(pDCVerInfo);
    }
}

DWORD
VmDirGetObjectAttribute(
    LDAP*   pLd,
    PCSTR   pszDomain,
    PCSTR   pszSearchDNPrefix,
    PCSTR   pszObjectClass,
    PCSTR   pszAttribute,
    int     scope,
    PSTR**  pppszValues,
    DWORD*  pdwNumValues
    )
{
    DWORD           dwError = 0;
    DWORD           dwAttrs = 0;
    PSTR*           ppszValues = NULL;
    PSTR            pszSearchBase = NULL;
    PSTR            pszDomainDN = NULL;
    PSTR            pszFilter = NULL;
    DWORD           idx = 0;
    PCSTR           ppszAttrs[] = {pszAttribute, NULL};
    LDAPMessage*    pSearchRes = NULL;
    LDAPMessage*    pEntry = NULL;

    if (!pLd ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszSearchDNPrefix) ||
        IsNullOrEmptyString(pszObjectClass) ||
        IsNullOrEmptyString(pszAttribute)
        )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSrvCreateDomainDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszSearchBase,
                                            "%s,%s",
                                            pszSearchDNPrefix,
                                            pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszFilter,
                    "(objectclass=%s)",
                    pszObjectClass
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_search_ext_s(
                            pLd,
                            pszSearchBase,
                            scope,
                            pszFilter,  /* filter */
                            (PSTR*)ppszAttrs,
                            FALSE,      /* attrsonly */
                            NULL,       /* serverctrls */
                            NULL,       /* clientctrls */
                            NULL,       /* timeout */
                            0,          /* sizelimit */
                            &pSearchRes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwAttrs = ldap_count_entries(pLd, pSearchRes);

    if (dwAttrs)
    {
        dwError = VmDirAllocateMemory(sizeof(PSTR) * dwAttrs, (VOID*)&ppszValues);
        BAIL_ON_VMDIR_ERROR(dwError);

        for ( pEntry = ldap_first_entry(pLd, pSearchRes);
              pEntry != NULL;
              pEntry = ldap_next_entry(pLd, pEntry), ++idx)
        {
            dwError = VmDirGetSingleAttributeFromEntry(
                        pLd,
                        pEntry,
                        pszAttribute,
                        FALSE,
                        &ppszValues[idx]);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *pdwNumValues = dwAttrs;
    *pppszValues = ppszValues;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszSearchBase);

    if (pSearchRes)
    {
        ldap_msgfree(pSearchRes);
    }

    return dwError;
error:
    for (idx = 0; idx < dwAttrs; ++idx)
    {
        VMDIR_SAFE_FREE_STRINGA(ppszValues[idx]);
    }
    VMDIR_SAFE_FREE_MEMORY(ppszValues);
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}
