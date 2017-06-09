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
VmDirSrvSetupDomainInstance(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          bSetupHost,
    BOOLEAN          bFirstNodeBootstrap,
    PCSTR            pszFQDomainName,
    PCSTR            pszDomainDN,
    PCSTR            pszUsername,
    PCSTR            pszPassword
    );

static
DWORD
VmDirSrvCreateOUContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName
    );

static
DWORD
VmDirSrvModifyPersistedDSERoot(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PSTR             pszRootNamingContextDN,
    PSTR             pszSchemaNamingContextDN,
    PSTR             pszSubSchemaSubEntryDN,
    PSTR             pszDefaultAdminDN
    );

static
DWORD
VmDirSrvCreateUserDN(
    PCSTR pszUsername,
    PCSTR pszParentDN,
    PSTR* ppszUserDN
    );

static
DWORD
VmDirSrvCreateUser(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    ENTRYID          eId,
    PCSTR            pszUsername,
    PCSTR            pszFirstname,
    PCSTR            pszSurname,
    PCSTR            pszAccount,
    PCSTR            pszPassword,
    PCSTR            pszUserDN,
    PCSTR            pszUserSid,
    PCSTR            pszKrbUPN
    );

static
DWORD
VmDirSrvCreateBuiltInAdminGroup(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszGroupName,
    PCSTR            pszUserDN,
    PCSTR            pszMemberDN,
    PSTR             pszAdminsGroupSid
    );

static
DWORD
_VmDirSrvCreateBuiltInGroup(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszGroupName,
    PCSTR            pszDN
    );

static
DWORD
VmDirSrvCreateBuiltinContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName
    );

VMDIR_FIRST_REPL_CYCLE_MODE   gFirstReplCycleMode;

DWORD
VmDirSrvSetupHostInstance(
    PCSTR   pszFQDomainName,
    PCSTR   pszUsername,
    PCSTR   pszPassword,
    PCSTR   pszReplURI,
    UINT32  firstReplCycleMode
    )
{
    DWORD   dwError = 0;

    PCSTR   pszDCsContainerName =         VMDIR_DOMAIN_CONTROLLERS_RDN_VAL;
    PSTR    pszDomainDN = NULL;
    PSTR    pszReplAgrDN = NULL;              // labeledURI=<ldap://192.165.226.127>,<ReplAgrsContainerDN>
    PSTR    pszDCsContainerDN = NULL;         // OU=Domain Controllers,<domain DN>
    PSTR    pszDCAccountDN = NULL;            // CN=<fully qualified host name>,OU=Domain Controllers,<domain DN>
    PSTR    pszDCAccountUPN = NULL;            // <hostname>@<domain name>
    PSTR    pszComputerAccountDN = NULL;      // CN=<fully qualified host name>,OU=Domain Computers,<domain DN>
    PSTR    pszUpperCaseFQDomainName = NULL;
    PSTR    pszLowerCaseHostName = NULL;
    PSTR    pszDefaultAdminDN = NULL;

    PVDIR_SCHEMA_CTX     pSchemaCtx = NULL;
    char                 pszHostName[VMDIR_MAX_HOSTNAME_LEN];
    VDIR_BERVALUE        bv = VDIR_BERVALUE_INIT;

    BOOLEAN                       bInLock = FALSE;
    PSTR                          pszUserDN = NULL;
    PCSTR                         pszUsersContainerName    = "Users";
    PSTR                          pszUsersContainerDN   = NULL; // CN=Users,<domain DN>
    PSTR pszPartnerHostName = NULL;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL,
                   "Setting up a host instance (%s).",
                   VDIR_SAFE_STRING(pszFQDomainName));

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Construct important DNs and create the persisted DSE Root entry

    // Domain DN
    dwError = VmDirSrvCreateDomainDN( pszFQDomainName, &pszDomainDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Domain Controllers container DN
    dwError = VmDirAllocateStringAVsnprintf(&pszDCsContainerDN, "%s=%s,%s", ATTR_OU, pszDCsContainerName, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // vdcpromo sets this key.
    dwError = VmDirGetRegKeyValue( VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMDIR_REG_KEY_DC_ACCOUNT,
                                   pszHostName,
                                   sizeof(pszHostName)-1);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower( pszHostName, &pszLowerCaseHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Domain controller account DN
    dwError = VmDirSrvCreateDN( pszLowerCaseHostName, pszDCsContainerDN, &pszDCAccountDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Domain controller account UPN
    dwError = VmDirAllocASCIILowerToUpper( pszFQDomainName, &pszUpperCaseFQDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(&pszDCAccountUPN, "%s@%s", pszLowerCaseHostName, pszUpperCaseFQDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Default administrator DN
    dwError = VmDirAllocateStringAVsnprintf( &pszDefaultAdminDN, "cn=%s,cn=%s,%s",
                                             pszUsername, pszUsersContainerName, pszDomainDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (firstReplCycleMode != FIRST_REPL_CYCLE_MODE_USE_COPIED_DB)
    {
        // Modify persisted DSE Root entry
        dwError = VmDirSrvModifyPersistedDSERoot( pSchemaCtx, pszDomainDN, SCHEMA_NAMING_CONTEXT_DN,
                                                  SUB_SCHEMA_SUB_ENTRY_DN, pszDefaultAdminDN);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    // set gVmdirServerGlobals.bvDefaultAdminDN
    dwError = VmDirAllocateBerValueAVsnprintf(
                &gVmdirServerGlobals.bvDefaultAdminDN,
                "%s",
                pszDefaultAdminDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirNormalizeDN( &gVmdirServerGlobals.bvDefaultAdminDN, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set systemDomainDN
    dwError = VmDirAllocateBerValueAVsnprintf(
                &gVmdirServerGlobals.systemDomainDN,
                "%s",
                pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirNormalizeDN( &gVmdirServerGlobals.systemDomainDN, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set dcAccountDN
    dwError = VmDirAllocateBerValueAVsnprintf(
                &gVmdirServerGlobals.dcAccountDN,
                "%s",
                pszDCAccountDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirNormalizeDN( &gVmdirServerGlobals.dcAccountDN, pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set dcAccountUPN
    dwError = VmDirAllocateBerValueAVsnprintf(
                &gVmdirServerGlobals.dcAccountUPN,
                "%s",
                pszDCAccountUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set replInterval and replPageSize
    gVmdirServerGlobals.replInterval = VmDirStringToIA(VMDIR_DEFAULT_REPL_INTERVAL);
    gVmdirServerGlobals.replPageSize = VmDirStringToIA(VMDIR_DEFAULT_REPL_PAGE_SIZE);

    // Set utdVector
    VmDirFreeBervalContent(&bv);
    bv.lberbv.bv_val = "";
    bv.lberbv.bv_len = 0;
    dwError = VmDirBervalContentDup( &bv, &gVmdirServerGlobals.utdVector );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Create Administrator DN
    dwError = VmDirSrvCreateDN( pszUsersContainerName, pszDomainDN, &pszUsersContainerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateUserDN( pszUsername, pszUsersContainerDN, &pszUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set DomainControllerGroupDN for first,second+ host setup
    dwError = VmDirAllocateBerValueAVsnprintf(
                &gVmdirServerGlobals.bvDCGroupDN,
                "cn=%s,cn=%s,%s",
                VMDIR_DC_GROUP_NAME,
                VMDIR_BUILTIN_CONTAINER_NAME,
                pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirNormalizeDN( &(gVmdirServerGlobals.bvDCGroupDN), pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    // set ServicesRootDN for first,second+ host setup
    dwError = VmDirAllocateBerValueAVsnprintf(
                &gVmdirServerGlobals.bvServicesRootDN,
                "cn=%s,%s",
                VMDIR_SERVICES_CONTAINER_NAME,
                pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirNormalizeDN( &(gVmdirServerGlobals.bvServicesRootDN), pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pszReplURI)) // 1st directory instance is being setup
    {
        dwError = VmDirSrvSetupDomainInstance( pSchemaCtx, TRUE, TRUE, pszFQDomainName, pszDomainDN, pszUsername,
                                               pszPassword );
        BAIL_ON_VMDIR_ERROR(dwError);

        // Create Domain Controllers container
        dwError = VmDirSrvCreateOUContainer( pSchemaCtx, pszDCsContainerDN, pszDCsContainerName );
        BAIL_ON_VMDIR_ERROR(dwError);

        //wake up repliation thread so that it can dynamically adding peers
        VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirSrvSetupHostInstance: wakeup replication thread with gVmdirGlobals.replAgrsCondition");
        VmDirConditionSignal(gVmdirGlobals.replAgrsCondition);
        VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);
    }
    else
    {
        extern VOID VmDirNewPartner(PCSTR);

        dwError = VmDirReplURIToHostname((PSTR)pszReplURI, &pszPartnerHostName);
        BAIL_ON_VMDIR_ERROR(dwError);

        VmDirNewPartner(pszPartnerHostName);

        // Wake up replication thread waiting on adding self to Raft cluster
        VMDIR_LOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);
        VmDirConditionSignal(gVmdirGlobals.replAgrsCondition);
        VMDIR_UNLOCK_MUTEX(bInLock, gVmdirGlobals.replAgrsMutex);
    }

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszReplAgrDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCsContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccountDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszComputerAccountDN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseFQDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszUsersContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszUserDN);
    VMDIR_SAFE_FREE_MEMORY(pszDefaultAdminDN);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseHostName);

    VmDirFreeBervalContent(&bv);

    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirSrvSetupHostInstance failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirSrvSetupTenantInstance(
    PCSTR pszFQDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PSTR  pszDomainDN = NULL;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Setting up a tenant instance (%s).",
			               VDIR_SAFE_STRING(pszFQDomainName));

    dwError = VmDirSrvCreateDomainDN(pszFQDomainName, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvSetupDomainInstance(
                    pSchemaCtx,
                    FALSE,
                    FALSE,
                    pszFQDomainName,
                    pszDomainDN,
                    pszUsername,
                    pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirSrvSetupTenantInstance failed. Error(%u)", dwError);
    goto cleanup;
}

static
DWORD
VmDirSrvSetupDomainInstance(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          bSetupHost,
    BOOLEAN          bFirstNodeBootstrap,
    PCSTR            pszFQDomainName,
    PCSTR            pszDomainDN,
    PCSTR            pszUsername,
    PCSTR            pszPassword
    )
{
    DWORD dwError = 0;

    PCSTR pszUsersContainerName    = "Users";
    PCSTR pszBuiltInContainerName  = "Builtin";
    PCSTR pszBuiltInAdministratorsGroupName = "Administrators";

    PSTR pszUsersContainerDN   = NULL; // CN=Users,<domain DN>
    PSTR pszBuiltInContainerDN = NULL; // CN=BuiltIn,<domain DN>
    PSTR pszUserDN = NULL;
    PSTR pszBuiltInAdministratorsGroupDN = NULL;
    PSTR pszDCGroupDN = NULL;
    PSTR pszTenantRealmName = NULL;

    PSECURITY_DESCRIPTOR_RELATIVE pSecDescRel = NULL;
    ULONG                         ulSecDescRel = 0;
    SECURITY_INFORMATION          SecInfo = 0;

    PSTR pszAdminSid = NULL;
    PSTR pszAdminsGroupSid = NULL;
    PSTR pszAdminUserKrbUPN = NULL;

    int i = 0;
    int startOfRdnInd = 0;

    // Create host/tenant domain

    dwError = VmDirSrvCreateDomain(pSchemaCtx, bSetupHost, pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Create Users container

    dwError = VmDirSrvCreateDN( pszUsersContainerName, pszDomainDN, &pszUsersContainerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateContainer( pSchemaCtx, pszUsersContainerDN, pszUsersContainerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Create Builtin container

    dwError = VmDirSrvCreateDN( pszBuiltInContainerName, pszDomainDN, &pszBuiltInContainerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateBuiltinContainer( pSchemaCtx, pszBuiltInContainerDN, pszBuiltInContainerName );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bSetupHost)
    {
        // only do this for the very first node startup.
        if (bFirstNodeBootstrap)
        {
            dwError = VmDirKrbInit();
            BAIL_ON_VMDIR_ERROR(dwError);

            // prepare administrator krb UPN for the very first node
            dwError = VmDirAllocateStringAVsnprintf(
                            &pszAdminUserKrbUPN,
                            "%s@%s",
                            pszUsername,
                            gVmdirKrbGlobals.pszRealm);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {   // setup tenant scenario.
        // Though we only support system domain kdc, we need UPN for SRP to function.
        dwError = VmDirKrbRealmNameNormalize(pszFQDomainName, &pszTenantRealmName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringAVsnprintf(
                        &pszAdminUserKrbUPN,
                        "%s@%s",
                        pszUsername,
                        pszTenantRealmName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Create Admin user

    dwError = VmDirSrvCreateUserDN( pszUsername, pszUsersContainerDN, &pszUserDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_USER_RID_ADMIN,
                                        &pszAdminSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateUser( pSchemaCtx,
                                  (bSetupHost && bFirstNodeBootstrap) ? DEFAULT_ADMINISTRATOR_ENTRY_ID : 0,
                                  pszUsername, pszUsername, pszFQDomainName, pszUsername, pszPassword,
                                  pszUserDN, pszAdminSid, pszAdminUserKrbUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetAdministratorPasswordNeverExpires();
    BAIL_ON_VMDIR_ERROR(dwError);

    // Create BuiltInAdministrators group

    dwError = VmDirAllocateStringAVsnprintf( &pszBuiltInAdministratorsGroupDN, "cn=%s,%s",
                                             pszBuiltInAdministratorsGroupName, pszBuiltInContainerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_ALIAS_RID_ADMINS,
                                        &pszAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Create the admin group for tenant setup or for first host setup.
    //
    if (bSetupHost == FALSE || bFirstNodeBootstrap == TRUE)
    {
        dwError = VmDirSrvCreateBuiltInAdminGroup( pSchemaCtx, pszBuiltInAdministratorsGroupName,
                                                   pszBuiltInAdministratorsGroupDN, pszUserDN,
                                                   pszAdminsGroupSid );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //
    // Create DCadmins group only for the very first
    // host setup.
    //
    if ( bSetupHost && bFirstNodeBootstrap )
    {
        // create DCAdmins Group
        dwError = VmDirAllocateStringAVsnprintf( &pszDCGroupDN,
                                                 "cn=%s,%s",
                                                 VMDIR_DC_GROUP_NAME,
                                                 pszBuiltInContainerDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirSrvCreateBuiltInGroup( pSchemaCtx,
                                               VMDIR_DC_GROUP_NAME,
                                               pszDCGroupDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Set up SD for the entries created during instance set up
    // Default allows administrator VMDIR_ENTRY_ALL_ACCESS,
    // oneself VMDIR_ENTRY_GENERIC_WRITE
    dwError = VmDirSrvCreateDefaultSecDescRel( pszUserDN, pszAdminsGroupSid,
                                               &pSecDescRel, &ulSecDescRel, &SecInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    // add the same sd for all the objects created during instance set-up

    // Set SD for the Domain objects
    for (i = (int) VmDirStringLenA(pszDomainDN) - 1; i >= 0; i-- )
    {
        if (i == 0 || pszDomainDN[i] == RDN_SEPARATOR_CHAR)
        {
            startOfRdnInd = (i == 0) ? 0 : i + 1 /* for , */;
            dwError = VmDirSetSecurityDescriptorForDn((PSTR)pszDomainDN + startOfRdnInd, SecInfo, pSecDescRel, ulSecDescRel);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    dwError = VmDirSetSecurityDescriptorForDn((PSTR)pszDomainDN, SecInfo, pSecDescRel, ulSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set SD for the administrator object

    dwError = VmDirSetSecurityDescriptorForDn(pszUserDN, SecInfo, pSecDescRel, ulSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set SD for Users container

    dwError = VmDirSetSecurityDescriptorForDn(pszUsersContainerDN, SecInfo, pSecDescRel, ulSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set SD for Builtin container

    dwError = VmDirSetSecurityDescriptorForDn(pszBuiltInContainerDN, SecInfo, pSecDescRel, ulSecDescRel);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bSetupHost == FALSE || bFirstNodeBootstrap == TRUE)
    {

        // Set SD for BuiltInAdministrators group

        dwError = VmDirSetSecurityDescriptorForDn(pszBuiltInAdministratorsGroupDN, SecInfo, pSecDescRel, ulSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (bSetupHost && bFirstNodeBootstrap)
    {
        // Set SD for BuiltIn DC group
        dwError = VmDirSetSecurityDescriptorForDn(pszDCGroupDN, SecInfo, pSecDescRel, ulSecDescRel);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszUsersContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszUserDN);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInAdministratorsGroupDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCGroupDN);
    VMDIR_SAFE_FREE_MEMORY(pszTenantRealmName);

    VMDIR_SAFE_FREE_MEMORY(pSecDescRel);

    VMDIR_SAFE_FREE_MEMORY(pszAdminSid);
    VMDIR_SAFE_FREE_MEMORY(pszAdminsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszAdminUserKrbUPN);

    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirSrvSetupDomainInstance failed. Error(%u)", dwError);
    goto cleanup;
}

static
DWORD
VmDirSrvCreateOUContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,              OC_TOP,
            ATTR_OBJECT_CLASS,              OC_ORGANIZATIONAL_UNIT,
            ATTR_OU,                        (PSTR)pszContainerName,
            NULL
    };

    dwError = VmDirSimpleEntryCreate( pSchemaCtx, ppszAttributes, (PSTR)pszContainerDN, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

static
DWORD
VmDirSrvCreateBuiltinContainer(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszContainerDN,
    PCSTR            pszContainerName
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,              OC_TOP,
            ATTR_OBJECT_CLASS,              OC_BUILTIN_DOMAIN,
            ATTR_CN,                        (PSTR)pszContainerName,
            NULL
    };

    dwError = VmDirSimpleEntryCreate( pSchemaCtx, ppszAttributes, (PSTR)pszContainerDN, 0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

static
DWORD
VmDirSrvModifyPersistedDSERoot(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PSTR             pszRootNamingContextDN,
    PSTR             pszSchemaNamingContextDN,
    PSTR             pszSubSchemaSubEntryDN,
    PSTR             pszDefaultAdminDN
    )
{
    DWORD dwError = 0;
    PSTR ppszPersistedDSERootAttrs[] =
    {
            ATTR_ROOT_DOMAIN_NAMING_CONTEXT,    pszRootNamingContextDN,
            ATTR_DEFAULT_NAMING_CONTEXT,        pszRootNamingContextDN,
            ATTR_SCHEMA_NAMING_CONTEXT,         pszSchemaNamingContextDN,
            ATTR_SUB_SCHEMA_SUB_ENTRY,          pszSubSchemaSubEntryDN,
            ATTR_NAMING_CONTEXTS,               pszRootNamingContextDN,
            ATTR_NAMING_CONTEXTS,               pszSchemaNamingContextDN,
            ATTR_DEFAULT_ADMIN_DN,              pszDefaultAdminDN,
            NULL
    };

    VDIR_OPERATION  op = {0};
    PSTR            pszLocalErrMsg = NULL;
    VDIR_BERVALUE   bvDSERootDN = VDIR_BERVALUE_INIT;
    int             i = 0;

    dwError = VmDirInitStackOperation( &op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "VmDirSrvModifyPersistedDSERoot: VmDirInitStackOperation failed with error code: %d.", dwError );

    // Setup target DN

    bvDSERootDN.lberbv.bv_val = PERSISTED_DSE_ROOT_DN;
    bvDSERootDN.lberbv.bv_len = VmDirStringLenA( bvDSERootDN.lberbv.bv_val );

    dwError = VmDirNormalizeDN( &bvDSERootDN, op.pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirBervalContentDup( &bvDSERootDN, &op.reqDn );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
            "VmDirSrvModifyPersistedDSERoot: BervalContentDup failed with error code: %d.", dwError );

    op.pBEIF = VmDirBackendSelect(op.reqDn.lberbv.bv_val);
    assert(op.pBEIF);

    dwError = VmDirBervalContentDup( &op.reqDn, &op.request.modifyReq.dn );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "VmDirSrvModifyPersistedDSERoot: BervalContentDup failed with error code: %d.", dwError );

    // Setup mods

    for (i = 0; ppszPersistedDSERootAttrs[i] != NULL; i += 2 )
    {
        dwError = VmDirAppendAMod( &op, MOD_OP_REPLACE,
                                   ppszPersistedDSERootAttrs[i],
                                   (int) VmDirStringLenA(ppszPersistedDSERootAttrs[i]),
                                   ppszPersistedDSERootAttrs[i + 1],
                                   VmDirStringLenA(ppszPersistedDSERootAttrs[i + 1]) );
        BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                    "VmDirSrvModifyPersistedDSERoot: VmDirAppendAMod failed with error code: %d.", dwError );
    }

    dwError = VmDirAppendAMod( &op, MOD_OP_DELETE, ATTR_INVOCATION_ID, ATTR_INVOCATION_ID_LEN,
                               gVmdirServerGlobals.invocationId.lberbv.bv_val,
                               gVmdirServerGlobals.invocationId.lberbv.bv_len );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                    "VmDirSrvModifyPersistedDSERoot: VmDirAppendAMod failed with error code: %d.", dwError );

    // Modify

    dwError = VmDirInternalModifyEntry( &op );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, pszLocalErrMsg,
                "VmDirSrvModifyPersistedDSERoot: InternalModifyEntry failed. DN: %s, Error code: %d, Error string: %s",
                op.reqDn.lberbv.bv_val, dwError, VDIR_SAFE_STRING( op.ldapResult.pszErrMsg ) );

cleanup:

    VmDirFreeBervalContent(&bvDSERootDN);
    VmDirFreeOperationContent(&op);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);

    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, VDIR_SAFE_STRING(pszLocalErrMsg) );
    goto cleanup;
}

static
DWORD
VmDirSrvCreateUserDN(
    PCSTR pszUsername,
    PCSTR pszParentDN,
    PSTR* ppszUserDN
    )
{
    DWORD dwError = 0;
    PSTR  pszUserDN = NULL;

    dwError = VmDirAllocateStringAVsnprintf(
                    &pszUserDN,
                    "cn=%s,%s",
                    pszUsername,
                    pszParentDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszUserDN = pszUserDN;

cleanup:

    return dwError;

error:

    *ppszUserDN = NULL;

    goto cleanup;
}

static
DWORD
VmDirSrvCreateUser(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    ENTRYID          eId,
    PCSTR            pszUsername,
    PCSTR            pszFirstname,
    PCSTR            pszSurname,
    PCSTR            pszAccount,
    PCSTR            pszPassword,
    PCSTR            pszUserDN,
    PCSTR            pszUserSid,
    PCSTR            pszKrbUPN      // if not null, kerberostize this user
    )
{
    DWORD   dwError = 0;

    {
        PSTR ppszAttributes[] =
        {
                ATTR_OBJECT_CLASS,      OC_USER,
                ATTR_CN,                (PSTR)pszUsername,
                ATTR_SN,                (PSTR)pszSurname,
                ATTR_GIVEN_NAME,        (PSTR)pszFirstname,
                ATTR_USER_PASSWORD,     (PSTR)pszPassword,
                ATTR_SAM_ACCOUNT_NAME,  (PSTR)pszAccount,
                ATTR_OBJECT_SID,        (PSTR)pszUserSid,
                NULL
        };

        PSTR ppszAttributesWithKrbUPN[] =
        {
                ATTR_OBJECT_CLASS,      OC_USER,
                ATTR_CN,                (PSTR)pszUsername,
                ATTR_SN,                (PSTR)pszSurname,
                ATTR_GIVEN_NAME,        (PSTR)pszFirstname,
                ATTR_USER_PASSWORD,     (PSTR)pszPassword,
                ATTR_SAM_ACCOUNT_NAME,  (PSTR)pszAccount,
                ATTR_OBJECT_SID,        (PSTR)pszUserSid,
                ATTR_KRB_UPN,           (PSTR)pszKrbUPN,
                NULL
        };

        dwError = VmDirSimpleEntryCreate(
                        pSchemaCtx,
                        (pszKrbUPN == NULL) ? ppszAttributes : ppszAttributesWithKrbUPN,
                        (PSTR)pszUserDN,
                        eId);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmDirSrvCreateBuiltInAdminGroup(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszGroupName,
    PCSTR            pszUserDN,
    PCSTR            pszMemberDN,
    PSTR             pszAdminsGroupSid
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,    OC_GROUP,
            ATTR_CN,              (PSTR)pszGroupName,
            ATTR_SAM_ACCOUNT_NAME,(PSTR)pszGroupName,
            ATTR_MEMBER,          (PSTR)pszMemberDN,
            ATTR_OBJECT_SID,      pszAdminsGroupSid,
            NULL
    };

    dwError = VmDirSimpleEntryCreate(
                    pSchemaCtx,
                    ppszAttributes,
                    (PSTR)pszUserDN,
                    0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}

static
DWORD
_VmDirSrvCreateBuiltInGroup(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    PCSTR            pszGroupName,
    PCSTR            pszDN
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,    OC_GROUP,
            ATTR_CN,              (PSTR)pszGroupName,
            ATTR_SAM_ACCOUNT_NAME,(PSTR)pszGroupName,
            NULL
    };

    dwError = VmDirSimpleEntryCreate(
                    pSchemaCtx,
                    ppszAttributes,
                    (PSTR)pszDN,
                    0);
    BAIL_ON_VMDIR_ERROR(dwError);

error:

    return dwError;
}
