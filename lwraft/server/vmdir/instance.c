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
    PCSTR            pszGroupSid,
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

//
// Set security descriptor for objects that were created prior to the creation
// of the various users and groups we need to exist in order to create the
// security descriptor in the first place.
//
DWORD
_VmDirAclServerObjects(
    PVMDIR_SECURITY_DESCRIPTOR pSecDescAnonymousRead,
    PVMDIR_SECURITY_DESCRIPTOR pSecDescNoDelete,
    PVMDIR_SECURITY_DESCRIPTOR pSecDescFullAccess
    )
{
    DWORD dwError = 0;

    dwError = VmDirSetRecursiveSecurityDescriptorForDn(
                RAFT_LOGS_CONTAINER_DN,
                pSecDescFullAccess);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSecurityDescriptorForDn(
                RAFT_PERSIST_STATE_DN,
                pSecDescNoDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSecurityDescriptorForDn(
                RAFT_CONTEXT_DN,
                pSecDescNoDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSecurityDescriptorForDn(
                PERSISTED_DSE_ROOT_DN,
                pSecDescAnonymousRead);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetRecursiveSecurityDescriptorForDn(
                SCHEMA_NAMING_CONTEXT_DN,
                pSecDescAnonymousRead);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetSecurityDescriptorForDn(
                CFG_ROOT_DN,
                pSecDescNoDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

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
    PSTR    pszDCsContainerDN = NULL;         // OU=Domain Controllers,<domain DN>
    PSTR    pszDCAccountDN = NULL;            // CN=<fully qualified host name>,OU=Domain Controllers,<domain DN>
    PSTR    pszDCAccountUPN = NULL;            // <hostname>@<domain name>
    PSTR    pszUpperCaseFQDomainName = NULL;
    PSTR    pszLowerCaseHostName = NULL;
    PSTR    pszDefaultAdminDN = NULL;

    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    char                pszHostName[VMDIR_MAX_HOSTNAME_LEN];
    VDIR_BERVALUE       bv = VDIR_BERVALUE_INIT;

    BOOLEAN bInLock = FALSE;
    PCSTR   pszUsersContainerName    = "Users";
    PSTR    pszPartnerHostName = NULL;
    VMDIR_SECURITY_DESCRIPTOR SecDescServices = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescAnonymousRead = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescNoDelete = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescFullAccess = {0};

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
    dwError = VmDirAllocateStringPrintf(&pszDCsContainerDN, "%s=%s,%s", ATTR_OU, pszDCsContainerName, pszDomainDN);
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

    dwError = VmDirAllocateStringPrintf(&pszDCAccountUPN, "%s@%s", pszLowerCaseHostName, pszUpperCaseFQDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Default administrator DN
    dwError = VmDirAllocateStringPrintf( &pszDefaultAdminDN, "cn=%s,cn=%s,%s",
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

    if (IsNullOrEmptyString(pszReplURI)) // 1st directory instance is being setup
    {
        dwError = VmDirSrvSetupDomainInstance(
                pSchemaCtx,
                TRUE,
                TRUE,
                pszFQDomainName,
                pszDomainDN,
                pszUsername,
                pszPassword,
                &SecDescServices,
                &SecDescAnonymousRead,
                &SecDescNoDelete,
                &SecDescFullAccess);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Set default security descriptor for vmwraftlogentry class
        dwError = VmDirSetDefaultSecurityDescriptorForClass(
                OC_CLASS_RAFT_LOG_ENTRY, &SecDescFullAccess);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Go back and ACL objects that were created early.
        dwError = _VmDirAclServerObjects(
                &SecDescAnonymousRead, &SecDescNoDelete, &SecDescFullAccess);
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
    VmDirSchemaCtxRelease(pSchemaCtx);
    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCsContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccountDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseFQDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszDefaultAdminDN);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseHostName);
    VMDIR_SAFE_FREE_MEMORY(SecDescServices.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescAnonymousRead.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescNoDelete.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescFullAccess.pSecDesc);
    VmDirFreeBervalContent(&bv);
    return dwError;

error:
    VmDirLog(LDAP_DEBUG_ANY, "VmDirSrvSetupHostInstance failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
_VmDirAclRootDomainObject(
    PCSTR pszDn,
    PCSTR pszUserDn,
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
    )
{
    DWORD dwError = 0;
    PVDIR_ENTRY pEntry = NULL;
    PSECURITY_DESCRIPTOR_RELATIVE pCurrentSecDesc = NULL;
    ULONG ulLength = 0;

    dwError = VmDirSimpleDNToEntry(pszDn, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSecurityDescriptorForEntry(
                pEntry,
                OWNER_SECURITY_INFORMATION |
                    GROUP_SECURITY_INFORMATION |
                    DACL_SECURITY_INFORMATION |
                    SACL_SECURITY_INFORMATION,
                    &pCurrentSecDesc,
                    &ulLength);
    if (dwError == VMDIR_ERROR_NO_SECURITY_DESCRIPTOR)
    {
        dwError = VmDirSetSecurityDescriptorForDn(pszDn, pSecDesc);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (dwError == ERROR_SUCCESS)
    {
        dwError = VmDirAddAceToSecurityDescriptor(pEntry, pCurrentSecDesc, pszUserDn, VMDIR_RIGHT_DS_READ_PROP | VMDIR_RIGHT_DS_DELETE_OBJECT);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        BAIL_ON_VMDIR_ERROR(dwError);
    }
cleanup:
    VMDIR_SAFE_FREE_MEMORY(pCurrentSecDesc);
    VmDirFreeEntry(pEntry);
    return dwError;
error:
    goto cleanup;
}

//
// Takes a DN of the form "dc=foo,dc=bar" and the respective admin DN (e.g.,
// "cn=Administrator,cn=users,dc=foo,dc=bar") and gives that admin user read
// access to the top-level portion of the domain. So, if "vsphere.local"
// already exists and someone creates the tenant "secondary.local" we want
// the secondary.local admin to be able to see "dc=local" in searches. Also,
// we give the admin user permission to delete the top-level domain object.
// (they'll only ever be able to do that if all domains in that TLD are gone,
// so there's no security concern w.r.t. them deleting anything they shouldn't).
//
DWORD
_VmDirAclDomainObjects(
    PCSTR pszDomainDN,
    PCSTR pszAdminUserDn, // DN of the admin user for the domain being created.
    PVMDIR_SECURITY_DESCRIPTOR pSecDesc
    )
{
    DWORD dwError = 0;
    int i = 0;
    int startOfRdnInd = 0;
    BOOLEAN bAcledRootObject = FALSE; // Have we already ACL'ed the root domain object?

    for (i = (int)VmDirStringLenA(pszDomainDN) - 1; i >= 0; --i)
    {
        if (i == 0 || pszDomainDN[i] == RDN_SEPARATOR_CHAR)
        {
            startOfRdnInd = (i == 0) ? 0 : i + 1 /* for , */;
            if (!bAcledRootObject)
            {
                dwError = _VmDirAclRootDomainObject(
                            pszDomainDN + startOfRdnInd,
                            pszAdminUserDn,
                            pSecDesc);
                BAIL_ON_VMDIR_ERROR(dwError);

                bAcledRootObject = TRUE;
            }
            else
            {
                dwError = VmDirSetSecurityDescriptorForDn(pszDomainDN + startOfRdnInd, pSecDesc);
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirSrvSetupDomainInstance(
    PVDIR_SCHEMA_CTX pSchemaCtx,
    BOOLEAN          bSetupHost,
    BOOLEAN          bFirstNodeBootstrap,
    PCSTR            pszFQDomainName,
    PCSTR            pszDomainDN,
    PCSTR            pszUsername,
    PCSTR            pszPassword,
    PVMDIR_SECURITY_DESCRIPTOR pSecDescServicesOut,         // OPTIONAL
    PVMDIR_SECURITY_DESCRIPTOR pSecDescAnonymousReadOut,    // OPTIONAL
    PVMDIR_SECURITY_DESCRIPTOR pSecDescNoDeleteOut,         // OPTIONAL
    PVMDIR_SECURITY_DESCRIPTOR pSecDescFullAccessOut        // OPTIONAL
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
    VMDIR_SECURITY_DESCRIPTOR SecDescFullAccess = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescNoDelete = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescNoDeleteChild = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescAnonymousRead = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescServices = {0};
    VMDIR_SECURITY_DESCRIPTOR SecDescDomain = {0};
    PSTR pszAdminSid = NULL;
    PSTR pszAdminsGroupSid = NULL;
    PSTR pszDomainAdminsGroupSid = NULL;
    PSTR pszDomainClientsGroupSid = NULL;
    PSTR pszAdminUserKrbUPN = NULL;

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
            dwError = VmDirAllocateStringPrintf(
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

        dwError = VmDirAllocateStringPrintf(
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

    dwError = VmDirAllocateStringPrintf( &pszBuiltInAdministratorsGroupDN, "cn=%s,%s",
                                             pszBuiltInAdministratorsGroupName, pszBuiltInContainerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_ALIAS_RID_ADMINS,
                                        &pszAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_ADMINS_RID,
                                        &pszDomainAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGenerateWellknownSid(pszDomainDN,
                                        VMDIR_DOMAIN_CLIENTS_RID,
                                        &pszDomainClientsGroupSid);
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
        dwError = VmDirAllocateStringPrintf( &pszDCGroupDN,
                                                 "cn=%s,%s",
                                                 VMDIR_DC_GROUP_NAME,
                                                 pszBuiltInContainerDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirSrvCreateBuiltInGroup( pSchemaCtx,
                                               VMDIR_DC_GROUP_NAME,
                                               pszDomainAdminsGroupSid,
                                               pszDCGroupDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    //
    // Create default security descriptor for internally-created entries.
    //
    dwError = VmDirSrvCreateSecurityDescriptor(
                VMDIR_ENTRY_ALL_ACCESS_NO_DELETE_CHILD_BUT_DELETE_OBJECT,
                pszUserDN,
                pszAdminsGroupSid,
                pszDomainAdminsGroupSid,
                pszDomainClientsGroupSid,
                FALSE,
                FALSE,
                FALSE,
                FALSE,
                FALSE,
                &SecDescFullAccess);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Create the default security descriptor for the builtin container, which
    // doesn't have the DELETE_CHILD permission (as we don't want the builtin
    // groups to be deletable by default). Note that an admin can still delete
    // these entries if they adjust the ACL.
    //
    dwError = VmDirSrvCreateSecurityDescriptor(
                VMDIR_ENTRY_ALL_ACCESS_NO_DELETE_CHILD,
                pszUserDN,
                pszAdminsGroupSid,
                pszDomainAdminsGroupSid,
                pszDomainClientsGroupSid,
                TRUE,
                FALSE,
                FALSE,
                FALSE,
                FALSE,
                &SecDescNoDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

    //
    // Create the default security descriptor for the users container, which
    // doesn't have the DELETE_CHILD permission (as we don't want the administrator
    // account to be deletable by default) but does inherit the delete-object
    // permission so that future users can be deleted. Note that an admin can
    // still delete the administrator account if they adjust the ACL.
    //
    dwError = VmDirSrvCreateSecurityDescriptor(
                VMDIR_ENTRY_ALL_ACCESS_NO_DELETE_CHILD_BUT_DELETE_OBJECT,
                pszUserDN,
                pszAdminsGroupSid,
                pszDomainAdminsGroupSid,
                pszDomainClientsGroupSid,
                FALSE,
                FALSE,
                FALSE,
                FALSE,
                FALSE,
                &SecDescNoDeleteChild);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateSecurityDescriptor(
                VMDIR_ENTRY_ALL_ACCESS_NO_DELETE_CHILD,
                pszUserDN,
                pszAdminsGroupSid,
                pszDomainAdminsGroupSid,
                pszDomainClientsGroupSid,
                FALSE,
                TRUE,
                FALSE,
                FALSE,
                FALSE,
                &SecDescAnonymousRead);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateSecurityDescriptor(
                VMDIR_ENTRY_ALL_ACCESS,
                pszUserDN,
                pszAdminsGroupSid,
                pszDomainAdminsGroupSid,
                pszDomainClientsGroupSid,
                FALSE,
                FALSE,
                FALSE,
                TRUE,
                FALSE,
                &SecDescServices);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateSecurityDescriptor(
                VMDIR_ENTRY_ALL_ACCESS,
                pszUserDN,
                pszAdminsGroupSid,
                pszDomainAdminsGroupSid,
                pszDomainClientsGroupSid,
                TRUE,
                FALSE,
                FALSE,
                FALSE,
                !bSetupHost,
                &SecDescDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirAclDomainObjects(pszDomainDN, pszUserDN, &SecDescDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set SD for the administrator object
    dwError = VmDirSetSecurityDescriptorForDn(pszUserDN, &SecDescNoDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set SD for Users container
    dwError = VmDirSetSecurityDescriptorForDn(pszUsersContainerDN, &SecDescNoDeleteChild);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set SD for Builtin container
    dwError = VmDirSetSecurityDescriptorForDn(pszBuiltInContainerDN, &SecDescNoDelete);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bSetupHost == FALSE || bFirstNodeBootstrap == TRUE)
    {
        // Set SD for BuiltInAdministrators group
        dwError = VmDirSetSecurityDescriptorForDn(pszBuiltInAdministratorsGroupDN, &SecDescNoDelete);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (bSetupHost && bFirstNodeBootstrap)
    {
        // Set SD for BuiltIn DC group
        dwError = VmDirSetSecurityDescriptorForDn(pszDCGroupDN, &SecDescNoDelete);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pSecDescServicesOut)
    {
        *pSecDescServicesOut = SecDescServices;
        SecDescServices.pSecDesc = NULL;
    }

    if (pSecDescAnonymousReadOut)
    {
        *pSecDescAnonymousReadOut = SecDescAnonymousRead;
        SecDescAnonymousRead.pSecDesc = NULL;
    }

    if (pSecDescNoDeleteOut)
    {
        *pSecDescNoDeleteOut = SecDescNoDelete;
        SecDescNoDelete.pSecDesc = NULL;
    }

    if (pSecDescFullAccessOut)
    {
        *pSecDescFullAccessOut = SecDescFullAccess;
        SecDescFullAccess.pSecDesc = NULL;
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszUsersContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszUserDN);
    VMDIR_SAFE_FREE_MEMORY(pszBuiltInAdministratorsGroupDN);
    VMDIR_SAFE_FREE_MEMORY(pszDCGroupDN);
    VMDIR_SAFE_FREE_MEMORY(pszTenantRealmName);

    VMDIR_SAFE_FREE_MEMORY(SecDescFullAccess.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescNoDelete.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescNoDeleteChild.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescAnonymousRead.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescServices.pSecDesc);
    VMDIR_SAFE_FREE_MEMORY(SecDescDomain.pSecDesc);

    VMDIR_SAFE_FREE_MEMORY(pszAdminSid);
    VMDIR_SAFE_FREE_MEMORY(pszAdminsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszDomainAdminsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszDomainClientsGroupSid);
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

    dwError = VmDirAllocateStringPrintf(
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
    PCSTR            pszGroupSid,
    PCSTR            pszDN
    )
{
    DWORD dwError = 0;
    PSTR ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,     OC_GROUP,
            ATTR_CN,               (PSTR)pszGroupName,
            ATTR_SAM_ACCOUNT_NAME, (PSTR)pszGroupName,
            ATTR_OBJECT_SID,       (PSTR)pszGroupSid,
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
