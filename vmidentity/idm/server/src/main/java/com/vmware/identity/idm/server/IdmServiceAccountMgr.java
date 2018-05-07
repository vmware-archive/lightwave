/*
 *
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.idm.server;

import java.net.URI;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.config.DefaultIdmServerConfigProvider;
import com.vmware.identity.idm.server.config.IIdmConfig;
import com.vmware.identity.idm.server.config.IIdmConfigProvider;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.provider.IAccountInfo;
import com.vmware.identity.idm.server.provider.IAccountProvider;
import com.vmware.identity.idm.server.provider.IPooledConnectionProvider;
import com.vmware.identity.idm.server.provider.LdapConnectionPool;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PooledLdapConnectionIdentity;
import com.vmware.identity.interop.directory.DefaultAccountPasswordResetter;
import com.vmware.identity.interop.directory.IAccountPasswordResetter;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;
import com.vmware.identity.interop.registry.IRegistryProvider;
import com.vmware.identity.interop.registry.DefaultRegistryProvider;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;

public class IdmServiceAccountMgr implements Runnable, IAccountProvider {

    private static String STS_ACCOUNT_DN = "stsAccountDN";
    private static String STS_ACCOUNT_UPN = "stsAccountUpn";
    private static String STS_ACCOUNT_PWD = "stsAccountPassword";
    private static String STS_ACCOUNT_PWD_RESET_POLL_INTERVAL = "stsAccountPollInterval";
    private static String STS_ACCOUNT_AUTH_TYPE = "stsAccountAuth";
    private static String STS_ACCOUNT_FORMAT = "sts/%s"; // sts/<dc account>
    private static String STS_ACCOUNT_DN_FORMAT = "cn=%s,cn=Managed Service Accounts,%s"; //
    private static String STS_ACCOUNT_OBJECT_CLASS = "msDS-ManagedServiceAccount";
    public static final String STS_ACCOUNTS_GROUP_NAME = "STSAccounts";
    private static String STS_ACCOUNTS_GROUP_DN_FORMAT = "cn=" + STS_ACCOUNTS_GROUP_NAME + ",%s";
    private static String ADMINISTRATORS_GROUP_DN_FORMAT = "cn=Administrators,cn=Builtin,%s";
    private static int DEFAULT_POLL_INTERVAL_SECS = 8*60*60; // 8 hrs
    private static int REGISTRY_UPDATE_RETRY_STARTUP = 5;
    private static int REGISTRY_UPDATE_INTERVAL_SEC_STARTUP = 1;
    private static int REGISTRY_UPDATE_RETRY_PWD_UPDATE = 30;
    private static int REGISTRY_UPDATE_INTERVAL_SEC_PWD_UPDATE = 5;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmServiceAccountMgr.class);

    private IPooledConnectionProvider poolProvider;
    private IRegistryProvider registryProvider;
    private IIdmConfigProvider configProvider;
    private IAccountPasswordResetter pwdResetter;
    private AtomicBoolean shouldRunRenew;
    private AtomicReference<ServiceAccount> stsAccount;
    private Thread pwdRenewThread;

    public IdmServiceAccountMgr() throws Exception {
        this(null, null, null, null);
    }

    public IdmServiceAccountMgr(
        IPooledConnectionProvider poolProvider,
        IRegistryProvider registryProvider,
        IIdmConfigProvider configProvider,
        IAccountPasswordResetter pwdResetter
    ) throws Exception{
        try{
            if (poolProvider == null) {
                logger.trace("using default connection pool provider");
                this.poolProvider = LdapConnectionPool.getInstance();
            } else {
                logger.trace("using passed in connection pool provider");
                this.poolProvider = poolProvider;
            }
            if(configProvider == null) {
                logger.trace("using default idmconfig provider");
                this.configProvider = new DefaultIdmServerConfigProvider();
            } else {
                logger.trace("using passed in idmconfig provider");
                this.configProvider = configProvider;
            }
            if(registryProvider == null) {
                logger.trace("using default registry provider");
                this.registryProvider = new DefaultRegistryProvider();
            } else {
                logger.trace("using passed in registry provider");
                this.registryProvider = registryProvider;
            }
            if(pwdResetter == null) {
                logger.trace("using default password resetter");
                this.pwdResetter = new DefaultAccountPasswordResetter();
            } else {
                logger.trace("using passed in password resetter");
                this.pwdResetter = pwdResetter;
            }
            this.shouldRunRenew = new AtomicBoolean(true);
            this.pwdRenewThread = new Thread(this);
            this.stsAccount = new AtomicReference<ServiceAccount>(null);
            this.ensureAccountExists();
        } catch(Exception ex) {
            logger.error("Failed to ensure sts account", ex);
            throw ex;
        }
    }

    public void startMonitorRenew() {
        logger.trace("starting pwd monitor thread");
        this.pwdRenewThread.start();
    }

    public void stopMonitorRenew(){
        logger.trace("marking pwd monitor thread to stop");
        this.shouldRunRenew.set(false);
    }

    public IAccountInfo getAccount() {
        try {
            return this.stsAccount.get();
        } catch(Throwable th) {
            logger.error("Failed to retrieve sts account information", th);
            throw th;
        }
    }

    public void ensureAccountAdmin(String tenant, String adminAccount, String adminPwd) throws Exception {

        try{
            // add sts group to Administrators of the tenant
            String adminsDn = String.format(ADMINISTRATORS_GROUP_DN_FORMAT, ServerUtils.getDomainDN(tenant));
            IIdmConfig cfg = this.configProvider.getConfig();
            try( PooledLdapConnection conn = this.borrowConnection(
                tenant, cfg.getTenantAdminUserName(tenant, adminAccount), adminPwd)){

                String domain = this.stsAccount.get().domain();

                conn.getConnection().modifyObject(
                    adminsDn,
                    new LdapMod(LdapModOperation.ADD, "member",
                        ServerUtils.getLdapValue(Arrays.asList(
                            String.format(STS_ACCOUNTS_GROUP_DN_FORMAT, ServerUtils.getDomainDN(domain))
                        )))
                );
            }
        } catch(Throwable th) {
            logger.error("Failed to add sts accounts group to tenant admins", th);
            throw th;
        }
    }

    private void ensureAccountExists() throws Exception {
        ServiceAccount act = getFromRegistry();
        if(act == null) {
            logger.info("Sts account info in registry does not exist creating new sts account.");
            IIdmConfig cfg = this.configProvider.getConfig();
            String uname = cfg.getDirectoryConfigStoreUserName(); // machine acct upn
            PrincipalId dc = ServerUtils.getPrincipalId(uname);
            String pwd = cfg.getDirectoryConfigStorePassword(); // machine acct pwd
            String stsAccountName = String.format(STS_ACCOUNT_FORMAT, dc.getName());
            String stsAccountUpn = stsAccountName + ServerUtils.UPN_SEPARATOR + dc.getDomain();
            AuthenticationType authType = cfg.getDirectoryConfigStoreAuthType();
            String domainDn = ServerUtils.getDomainDN(dc.getDomain());
            String accountDn = String.format(
                STS_ACCOUNT_DN_FORMAT,
                stsAccountUpn,
                domainDn);
            act = new ServiceAccount(accountDn, stsAccountUpn, pwd, authType);
            this.createAccountInLdap(act);
            try{
                this.saveToRegistry(act, false);
            } catch(Exception ex){
                // if we failed creation we want to remove, so next startup attempts it a new
                logger.error(
                    String.format(
                        "Failed to save sts account [%s] info to registry. will cleanup sts account.", act.upn()),
                        ex);
                this.removeAccountFromLdap(act);
                throw ex;
            }
            logger.info("Successfully created Sts account [{}]", act.upn());
        } else {
            logger.info("Sts account exists [{}].", act.upn());
        }
        this.stsAccount.set(act);
    }

    private ServiceAccount getFromRegistry() {
        ServiceAccount account = null;
        IRegistryAdapter regAdapter = this.registryProvider.getRegistryAdapter();
        try(IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ))
        {
            String accountDn = regAdapter.getStringValue(
                rootKey, IdmServerConfig.CONFIG_ROOT_KEY, STS_ACCOUNT_DN, true);
            String accountUpn = regAdapter.getStringValue(
                rootKey, IdmServerConfig.CONFIG_ROOT_KEY, STS_ACCOUNT_UPN, true);
            String accountPwd = regAdapter.getStringValue(
                rootKey, IdmServerConfig.CONFIG_ROOT_KEY, STS_ACCOUNT_PWD, true);
            String accountAuth = regAdapter.getStringValue(
                rootKey, IdmServerConfig.CONFIG_ROOT_KEY, STS_ACCOUNT_AUTH_TYPE, true);

            if (!ServerUtils.isNullOrEmpty(accountDn) &&
                !ServerUtils.isNullOrEmpty(accountUpn) &&
                !ServerUtils.isNullOrEmpty(accountPwd) &&
                !ServerUtils.isNullOrEmpty(accountAuth)
                ) {
                    account = new ServiceAccount(
                        accountDn, accountUpn, accountPwd, AuthenticationType.valueOf(accountAuth));
            }
        }
        return account;
    }

    private void saveToRegistry(ServiceAccount act, boolean pwdUpdate) throws Exception {
        Exception error = null;
        int retries = pwdUpdate ? REGISTRY_UPDATE_RETRY_PWD_UPDATE : REGISTRY_UPDATE_RETRY_STARTUP;
        int interval = pwdUpdate ? REGISTRY_UPDATE_INTERVAL_SEC_PWD_UPDATE : REGISTRY_UPDATE_INTERVAL_SEC_STARTUP;
        for( int i = 0; i < retries; i++){
            try{
                IRegistryAdapter regAdapter = this.registryProvider.getRegistryAdapter();
                try(IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ))
                {
                    try( IRegistryKey idmConfigKey =
                            regAdapter.openKey(
                                rootKey, IdmServerConfig.CONFIG_ROOT_KEY, 0,
                                (int)RegKeyAccess.KEY_SET_VALUE|(int)RegKeyAccess.KEY_READ)) {
                        regAdapter.setStringValue(idmConfigKey, STS_ACCOUNT_PWD, act.password());
                        if (!pwdUpdate) {
                            regAdapter.setStringValue(idmConfigKey, STS_ACCOUNT_DN, act.dn());
                            regAdapter.setStringValue(idmConfigKey, STS_ACCOUNT_UPN, act.upn());
                            regAdapter.setStringValue(idmConfigKey, STS_ACCOUNT_AUTH_TYPE, act.authentication().toString());
                        }
                    }
                }
                error = null;
                break;
            } catch(Exception ex) {
                logger.error(
                    String.format("Save to registry failed on iteration: %d", i),
                    ex);
                error = ex;
                Thread.sleep(interval*1000);
            }
        }
        if (error != null) {
            logger.error("registry update failed through all retries.");
            throw error;
        }
    }

    private int getPollIntervalSecs() {
        int pollIntervalSecs = DEFAULT_POLL_INTERVAL_SECS;
        try{
            IRegistryAdapter regAdapter = this.registryProvider.getRegistryAdapter();
            try(IRegistryKey rootKey = regAdapter.openRootKey((int) RegKeyAccess.KEY_READ)){
                Integer pollInterval = regAdapter.getIntValue(
                    rootKey, IdmServerConfig.CONFIG_ROOT_KEY, STS_ACCOUNT_PWD_RESET_POLL_INTERVAL, true);
                if (pollInterval != null) {
                    pollIntervalSecs = pollInterval.intValue();
                }
            }
        } catch(Throwable th) {
             // if we failed to read we will fallback to default
            logger.error("Failed to read pwd reset poll interval. will use default.", th);
            pollIntervalSecs = DEFAULT_POLL_INTERVAL_SECS;
        }
        return pollIntervalSecs;
    }

    private void createAccountInLdap(ServiceAccount act) throws Exception {

        try( PooledLdapConnection conn = this.borrowConnection()){
            String samAccount = act.accountName();
            String domain = act.domain();
            LdapMod[] ldapMods = new LdapMod[] {
                new LdapMod(LdapModOperation.ADD, "objectClass",
                        new LdapValue[] {
                                LdapValue.fromString(STS_ACCOUNT_OBJECT_CLASS)}),
                new LdapMod(LdapModOperation.ADD, "cn",
                        new LdapValue[] {
                                LdapValue.fromString(samAccount)}),
                new LdapMod(LdapModOperation.ADD, "samAccountName",
                    new LdapValue[] {
                        LdapValue.fromString(samAccount)}),
                new LdapMod(LdapModOperation.ADD, "userPrincipalName",
                    new LdapValue[] {
                        LdapValue.fromString(act.upn())}),
                new LdapMod(LdapModOperation.ADD, "userPassword",
                    new LdapValue[] {
                        LdapValue.fromString(act.password())}),
            };

            // create account
            conn.getConnection().addObject(act.dn(), ldapMods);

            try{
                // add account to sts group
                conn.getConnection().modifyObject(
                    String.format(STS_ACCOUNTS_GROUP_DN_FORMAT, ServerUtils.getDomainDN(domain)),
                    new LdapMod(LdapModOperation.ADD, "member",
                    ServerUtils.getLdapValue(Arrays.asList(act.dn())))
                );
            } catch(Exception ex) {
                // adding sts account to sts group failed cleaning up account
                // so it can be re-created on next startup
                logger.error(
                    String.format("failed to add sts account [%s] to sts accounts group", act.upn()),
                    ex);
                try{
                    this.removeAccountFromLdap(act);
                } catch(Exception ex1) {
                    logger.error(
                        String.format("failed to remove sts account [%s]", act.upn()),
                        ex1);
                }
                throw ex;
            }
        } catch(Throwable th) {
            logger.error("Failed to create sts account in ldap", th);
            throw th;
        }
    }
    private void removeAccountFromLdap(ServiceAccount act) throws Exception {

        try( PooledLdapConnection conn = this.borrowConnection()){
            // remove account
            conn.getConnection().deleteObject(act.dn());
        } catch(Throwable th) {
            logger.error("Failed to remove sts account from ldap", th);
            throw th;
        }
    }

    private PooledLdapConnection borrowConnection() throws Exception {
        IIdmConfig cfg = this.configProvider.getConfig();
        return this.borrowConnection(
            cfg.getDirectoryConfigStoreDomain(),
            cfg.getDirectoryConfigStoreUserName(),
            cfg.getDirectoryConfigStorePassword());
    }

    private PooledLdapConnection borrowConnection(String tenant, String accountName, String pwd) throws Exception {
        Exception latestEx = null;
        IIdmConfig cfg = this.configProvider.getConfig();
        for (URI connectionString : cfg.getSystemDomainConnectionInfo()) {
            PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(
                connectionString.toString(), cfg.getDirectoryConfigStoreAuthType());
            PooledLdapConnectionIdentity pooledLdapConnectionIdentity =
                builder
                    .setTenantName(tenant)
                    .setUsername(accountName)
                    .setPassword(pwd)
                    .setUseGCPort(false)
                    .build();
            try {
                return this.poolProvider.borrowConnection(pooledLdapConnectionIdentity);
            } catch (Exception e) {
                logger.error(
                    String.format(
                        "borrow connection(%s) failed",
                        pooledLdapConnectionIdentity.toString()),
                    e);
                latestEx = e;
            }
        }
        throw latestEx;
    }

    @Override
    public void run() {
        try(IDiagnosticsContextScope ctxt =
            DiagnosticsContextFactory.createContext("StsAccountChecker", "", "", "") ) {
            while(this.shouldRunRenew.get()) {
                try {
                    ServiceAccount act = this.stsAccount.get();
                    String newPwd = this.tryResetPwd(act);
                    if (ServerUtils.isNullOrEmpty(newPwd) == false) {
                        act = new ServiceAccount(act.dn(), act.upn(), newPwd, act.authentication());
                        this.stsAccount.set(act);
                        this.saveToRegistry( act, true);
                        logger.info("StsAccountChecker: successfully updated pwd for [{}]", act.upn());
                    }
                } catch (Throwable th) {
                    logger.error("Failed to check/renew account pwd", th);
                }

                try {
                    Thread.sleep(1000 * this.getPollIntervalSecs());
                } catch (InterruptedException e) {
                    logger.error("StsAccountChecker Thread was interrupted.");
                }
            }
        }
    }

    private String tryResetPwd(ServiceAccount act) throws Exception {
        IIdmConfig cfg = this.configProvider.getConfig();
        Exception latestEx = null;
        for (URI uri : cfg.getSystemDomainConnectionInfo()) {
            try {
                return this.pwdResetter.ResetAccountPassword(
                        uri.getHost(), cfg.getDirectoryConfigStoreDomain(),
                        act.upn(), act.dn(), act.password(), false);
            } catch (Exception e) {
                latestEx = e;
            }
        }
        throw latestEx;
    }

    private static class ServiceAccount implements IAccountInfo {
        private String accountDn;
        private String accountUpn;
        private String accountPassword;
        private AuthenticationType accountAuth;
        private String domain;
        private String accountName;

        public ServiceAccount(
            String accountDn, String accountUpn, String accountPassword,
            AuthenticationType accountAuth) {

            ValidateUtil.validateNotEmpty(accountDn, "accountDn");
            ValidateUtil.validateNotEmpty(accountUpn, "accountUpn");
            ValidateUtil.validateNotEmpty(accountPassword, "accountPassword");

            if ( (accountAuth != AuthenticationType.PASSWORD) &&
                 (accountAuth != AuthenticationType.SRP) ) {
                     throw new IllegalArgumentException(
                         String.format("Authentcation type '[%s]' is not supported", accountAuth.toString()));
                 }

            this.accountDn = accountDn;
            this.accountUpn = accountUpn;
            this.accountPassword = accountPassword;
            this.accountAuth = accountAuth;
            String[] parts = accountUpn.split("@");
            this.accountName = parts[0];
            this.domain = parts[1];
        }

        public String upn() {
            return this.accountUpn;

        }
        public String dn() {
            return this.accountDn;
        }

        public String domain() {
            return this.domain;
        }

        public String accountName() {
            return this.accountName;
        }

        @Override
        public String userName() {
            if (this.accountAuth == AuthenticationType.PASSWORD ) {
                return this.accountDn;
            } else { //this.accountAuth == AuthenticationType.SRP
                return this.accountUpn;
            }
        }

        @Override
        public String password() {
            return this.accountPassword;
        }

        @Override
        public AuthenticationType authentication() {
            return this.accountAuth;
        }
    }
}
