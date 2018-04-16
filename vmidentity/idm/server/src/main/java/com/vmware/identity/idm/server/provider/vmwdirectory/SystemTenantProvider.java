/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */

package com.vmware.identity.idm.server.provider.vmwdirectory;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.provider.ILdapConnectionProvider;
import com.vmware.identity.idm.server.provider.IPooledConnectionProvider;

import org.apache.commons.lang.NotImplementedException;
import org.apache.commons.lang.Validate;

public class SystemTenantProvider extends VMDirProvider {

    private final String baseGroupDn;
    private boolean useIdsCreds; // used for testing

    // todo: add ops config
    public SystemTenantProvider(
	String tenantName, IIdentityStoreData store )
    {
    	this(tenantName, store, null, null, false);
    }

    // todo: add ops config
    // for unit tests
    public SystemTenantProvider(
	    String tenantName, IIdentityStoreData store,
        IPooledConnectionProvider pooledConnectionProvider,
        ILdapConnectionProvider ldapConnectionProvider,
        boolean idsCreds)
    {
        super(tenantName, getIDS(store, idsCreds), pooledConnectionProvider, ldapConnectionProvider );
        this.baseGroupDn = this.getGroupsDN(this.getDomain()).toLowerCase();
        this.useIdsCreds = idsCreds;
    }

    @Override
    protected String getUsername()
    {
        if (this.useIdsCreds) {
            return getStoreDataEx().getUserName();
        } else {
            return IdmServerConfig.getInstance().getDirectoryConfigStoreUserName();
        }
    }

    @Override
    protected String getPassword()
    {
        if (this.useIdsCreds) {
            return getStoreDataEx().getPassword();
        } else {
            return IdmServerConfig.getInstance().getDirectoryConfigStorePassword();
        }
    }

    @Override
    protected AuthenticationType getAuthType()
    {
        if (this.useIdsCreds) {
            return getStoreDataEx().getAuthenticationType();
        } else {
           return IdmServerConfig.getInstance().getDirectoryConfigStoreAuthType();
        }
    }

    @Override
    protected String getUsersDN(String domain) {
        return this.getStoreDataEx().getUserBaseDn();
    }

    @Override
    protected String getGroupsDN(String domain) {
        return this.getStoreDataEx().getGroupBaseDn();
    }

    @Override
    protected boolean groupDnInscope(String groupDn)
    {
        return (ServerUtils.isNullOrEmpty(groupDn) == false) &&
            (groupDn.toLowerCase().endsWith(this.baseGroupDn));
    }

    @Override
    protected String USER_PRINC_QUERY_BY_USER_PRINCIPAL() {
        return USER_PRINC_QUERY_BY_USER_PRINCIPAL;
    }

    @Override
    protected String USER_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCT() {
        return USER_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCT;
    }

    @Override
    protected String USER_PRINC_QUERY_BY_OBJECTSID() {
        return USER_PRINC_QUERY_BY_OBJECTSID;
    }

    @Override
    protected String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL() {
        return USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL;
    }

    @Override
    protected String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT() {
        return USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT;
    }

    @Override
    protected String USER_OR_SVC_PRINC_QUERY_BY_ATTRIBUTE() {
        return USER_OR_SVC_PRINC_QUERY_BY_ATTRIBUTE;
    }

    @Override
    protected String USER_OR_SVC_PRINC_QUERY_BY_UPN_ATTRIBUTE() {
        return USER_OR_SVC_PRINC_QUERY_BY_UPN_ATTRIBUTE;
    }

    @Override
    protected String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL() {
        return USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL;
    }

    @Override
    protected String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT() {
        return USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT;
    }

    @Override
    protected String USER_ALL_PRINC_QUERY() {
        return USER_ALL_PRINC_QUERY;
    }

    @Override
    protected String USER_ALL_BY_ACCOUNT_NAME() {
        return USER_ALL_BY_ACCOUNT_NAME;
    }

    @Override
    protected String USER_ALL_QUERY() {
        return USER_ALL_QUERY;
    }

    @Override
    protected String FSP_PRINC_QUERY_BY_EXTERNALID() {
        // should not be called
        throw new NotImplementedException();
    }

    private static IIdentityStoreData getIDS( IIdentityStoreData storeData, boolean idsCreds) {
        ValidateUtil.validateNotNull( storeData, "storeData" );
            ValidateUtil.validateNotEmpty( storeData.getName(), "storeData.getName()" );
            ValidateUtil.validateNotNull(
            storeData.getExtendedIdentityStoreData(), "storeData.getExtendedIdentityStoreData()" );
        Validate.isTrue(
            storeData.getExtendedIdentityStoreData().getProviderType() == IdentityStoreType.IDENTITY_STORE_TYPE_VMWARE_DIRECTORY,
                    "IIdentityStoreData must represent a store of "
                + "'IDENTITY_STORE_TYPE_VMWARE_DIRECTORY' type.");
        IIdentityStoreDataEx dataEx = storeData.getExtendedIdentityStoreData();

        ServerIdentityStoreData data = new ServerIdentityStoreData(DomainType.EXTERNAL_DOMAIN, storeData.getName());

        data.setAccountLinkingUseUPN(dataEx.getCertLinkingUseUPN());
        data.setAttributeMap(dataEx.getAttributeMap());
        data.setAuthnTypes(dataEx.getAuthnTypes());
        data.setCertificates(dataEx.getCertificates());
        data.setConnectionStrings(dataEx.getConnectionStrings());
        data.setFlags(dataEx.getFlags());
        data.setFriendlyName(dataEx.getFriendlyName());
        data.setHintAttributeName(dataEx.getCertUserHintAttributeName());
        data.setProviderType(dataEx.getProviderType());
        data.setSchemaMapping(dataEx.getIdentityStoreSchemaMapping());
        data.setSearchTimeoutSeconds(dataEx.getSearchTimeoutSeconds());
        data.setServicePrincipalName(dataEx.getServicePrincipalName());
        data.setUpnSuffixes(dataEx.getUpnSuffixes());
        data.setUseMachineAccount(dataEx.useMachineAccount());
        if(idsCreds){
            data.setUserName(dataEx.getUserName());
            data.setPassword(dataEx.getPassword());
            data.setAuthenticationType(dataEx.getAuthenticationType());
        }

        // todo: use ops provider settings
        data.setUserBaseDn(dataEx.getUserBaseDn());
        if (ServerUtils.isNullOrEmpty(data.getUserBaseDn())) {
            data.setUserBaseDn(getDomainDN(data.getName()));
        }
        data.setGroupBaseDn(dataEx.getGroupBaseDn());
        if (ServerUtils.isNullOrEmpty(data.getGroupBaseDn())) {
            data.setGroupBaseDn(getDomainDN(data.getName()));
        }

        return data;
    }

    /**
     * arg1 - userPrincipalName
     * arg2 - tenantizedUserPrincipalName
     * arg3 - additional filter
     */
    private static final String USER_PRINC_QUERY_BY_USER_PRINCIPAL =
        "(&" +
            "(|" +
                "(userPrincipalName=%1$s)" +
                "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%2$s)" +
            ")" +
            "(objectClass=user)(!(vmwSTSSubjectDN=*))(!(objectClass=vmwExternalIdpUser))%3$s)";

    /**
     * arg1 - userPrincipalName
     * arg2 - sAMAccountName
     * arg3 - tenantizedUserPrincipalName
     * arg4 - additional filter
     */
    private static final String USER_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCT =
            "(&(|" +
                  "(userPrincipalName=%1$s)" +
                  "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%3$s)" +
                  "(sAMAccountName=%2$s)" +
               ")(objectClass=user)(!(vmwSTSSubjectDN=*))(!(objectClass=vmwExternalIdpUser))%4$s)";

    private static final String USER_PRINC_QUERY_BY_OBJECTSID =
            "(&(objectSid=%s)(objectClass=user)(!(vmwSTSSubjectDN=*))(!(objectClass=vmwExternalIdpUser)))";

   /**
    * arg1 - userPrincipalName
    * arg2 - tenantizedUserPrincipalName
    */
    private static final String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL =
            "(&(|" +
                 "(userPrincipalName=%1$s)" +
                  "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%2$s)" +
              ")(objectClass=user)(!(objectClass=vmwExternalIdpUser)))";
    /**
     * arg1 - userPrincipalName
     * arg2 - sAMAccountName
     * arg3 - tenantizedUserPrincipalName
     */
     private static final String USER_OR_SVC_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT =
             "(&(|" +
                   "(userPrincipalName=%1$s)" +
                   "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%3$s)" +
                   "(sAMAccountName=%2$s)" +
               ")(objectClass=user)(!(objectClass=vmwExternalIdpUser)))";

    private static final String USER_OR_SVC_PRINC_QUERY_BY_ATTRIBUTE =
            "(&(%1$s=%2$s)(objectClass=user)(!(objectClass=vmwExternalIdpUser)))";

    private static final String USER_OR_SVC_PRINC_QUERY_BY_UPN_ATTRIBUTE =
            "(&(|" +
                   "(userPrincipalName=%1$s)" +
                   "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%2$s)" +
              ")(objectClass=user)(!(objectClass=vmwExternalIdpUser)))";

    /**
    * arg1 - userPrincipalName
    * arg2 - tenantizedUserPrincipalName
    */
    private static final String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL =
            "(&(|" +
                "(userPrincipalName=%1$s)" +
                "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%2$s)" +
              ")" + "(objectClass=user)(!(objectClass=vmwExternalIdpUser)))";
    /**
    * arg1 - userPrincipalName
    * arg2 - sAMAccountName
    * arg3 - tenantizedUserPrincipalName
    */
    private static final String USER_OR_SVC_OR_GROUP_PRINC_QUERY_BY_USER_PRINCIPAL_OR_ACCOUNT =
            "(|" +
                "(&(|" +
                     "(userPrincipalName=%1$s)" +
                     "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%3$s)" +
                     "(sAMAccountName=%2$s)" +
                   ")(objectClass=user)(!(objectClass=vmwExternalIdpUser)))" +
                "(&(sAMAccountName=%2$s)(objectClass=group))" +
             ")";

    // This is to get all users under Users CN
    // We need this do to substring match on users and groups
    private static final String USER_ALL_PRINC_QUERY =
            "(&(objectClass=user)(!(objectClass=computer))(!(vmwSTSSubjectDN=*))(!(objectClass=vmwExternalIdpUser)))";

    private static final String USER_ALL_BY_ACCOUNT_NAME =
            "(&(objectClass=user)(!(objectClass=computer))(!(objectClass=vmwExternalIdpUser))"+
              "(|" +
                  "(sAMAccountName=%1$s*)(sn=%1$s*)(givenName=%1$s*)(cn=%1$s*)(mail=%1$s*)" +
                  "(userPrincipalName=%1$s*)" +
                  "("+ ATTR_TENANTIZED_USER_PRINCIPAL_NAME +"=%1$s*)" +
                "))";

    private static final String USER_ALL_QUERY =
            "(&(objectClass=user)(!(objectClass=computer))(!(objectClass=vmwExternalIdpUser)))";
}