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
import java.net.URISyntaxException;
import java.util.Arrays;
import java.util.Comparator;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.server.config.IIdmConfig;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.utils.AccountPasswordResetterMock;
import com.vmware.identity.idm.server.utils.IMockInitializer;
import com.vmware.identity.idm.server.utils.IdmConfigProviderMock;
import com.vmware.identity.idm.server.utils.PooledConnectionProviderMock;
import com.vmware.identity.idm.server.utils.RegistryProviderMock;
import com.vmware.identity.idm.server.utils.RegistryKeyMock;
import com.vmware.identity.interop.directory.DirectoryException;
import com.vmware.identity.interop.directory.IAccountPasswordResetter;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryNoSuchKeyOrValueException;

import org.junit.Assert;
import org.apache.commons.lang.ObjectUtils;
import org.easymock.EasyMock;
import org.easymock.LogicalOperator;
import org.junit.BeforeClass;
import org.junit.Test;

public class IdmAccountMgrTest {

    @BeforeClass
    public static void setup() throws URISyntaxException {
        LDAP_URI = new URI(LDAP_STR);
    }

    @Test
    public void testNoAccount() throws Exception {

        try(
            PooledConnectionProviderMock connection =
                new PooledConnectionProviderMock(
                    new PooledConnectionInitializer(true, false, false));
            AccountPasswordResetterMock resetter =
                new AccountPasswordResetterMock(
                    new AccountPwdResetterInitializer(null, false));
            RegistryProviderMock registry =
                new RegistryProviderMock(
                    new RegistryProviderInitializer(
                        false /*fail*/, true/*get*/, true/*set*/, false/*hasAccount*/,
                        false/*pwdOnly*/, false/*getPoll*/, false/*saveFailThenSucceed*/)
                );
            IdmConfigProviderMock config =
                new IdmConfigProviderMock(
                    new IdmConfigInitializer()); ){

            IdmServiceAccountMgr mgr = new IdmServiceAccountMgr(
                connection, registry, config, resetter.getResetter());

            // should create an account
        }
    }

    @Test
    public void testExistingAccount() throws Exception {

        try(
            PooledConnectionProviderMock connection =
                new PooledConnectionProviderMock(
                    new PooledConnectionInitializer(false, false, false));
            AccountPasswordResetterMock resetter =
                new AccountPasswordResetterMock(
                    new AccountPwdResetterInitializer(null, false));
            RegistryProviderMock registry =
                new RegistryProviderMock(
                    new RegistryProviderInitializer(
                        false /*fail*/, true/*get*/, false/*set*/, true/*hasAccount*/,
                        false/*pwdOnly*/, false/*getPoll*/, false/*saveFailThenSucceed*/)

                );
            IdmConfigProviderMock config =
                new IdmConfigProviderMock(
                    new IdmConfigInitializer()); ){

            IdmServiceAccountMgr mgr = new IdmServiceAccountMgr(
                connection, registry, config, resetter.getResetter());

            // should get acct info fro registry
        }
    }

    @Test
    public void testStartPollNoRenew() throws Exception {

        try(
            PooledConnectionProviderMock connection =
                new PooledConnectionProviderMock(
                    new PooledConnectionInitializer(false, false, false));
            AccountPasswordResetterMock resetter =
                new AccountPasswordResetterMock(
                    new AccountPwdResetterInitializer("", false));
            RegistryProviderMock registry =
                new RegistryProviderMock(
                    new RegistryProviderInitializer(
                        false /*fail*/, true/*get*/, false/*set*/, true/*hasAccount*/,
                        false/*pwdOnly*/, true/*getPoll*/, false/*saveFailThenSucceed*/)

                );
            IdmConfigProviderMock config =
                new IdmConfigProviderMock(
                    new IdmConfigInitializer()); ){

            IdmServiceAccountMgr mgr = new IdmServiceAccountMgr(
                connection, registry, config, resetter.getResetter());

            mgr.startMonitorRenew();
            Thread.sleep(1000);
            mgr.stopMonitorRenew();
        }
    }

    @Test
    public void testStartPollRenew() throws Exception {

        try(
            PooledConnectionProviderMock connection =
                new PooledConnectionProviderMock(
                    new PooledConnectionInitializer(false, false, false));
            AccountPasswordResetterMock resetter =
                new AccountPasswordResetterMock(
                    new AccountPwdResetterInitializer(PASSWORD_RENEWED, false));
            RegistryProviderMock registry =
                new RegistryProviderMock(
                    new RegistryProviderInitializer(
                        false /*fail*/, true/*get*/, true/*set*/, true/*hasAccount*/,
                        true/*pwdOnly*/, true/*getPoll*/, false/*saveFailThenSucceed*/)
                );
            IdmConfigProviderMock config =
                new IdmConfigProviderMock(
                    new IdmConfigInitializer()); ){

            IdmServiceAccountMgr mgr = new IdmServiceAccountMgr(
                connection, registry, config, resetter.getResetter());

            mgr.startMonitorRenew();
            Thread.sleep(1000);
            mgr.stopMonitorRenew();
        }
    }

    @Test
    public void testNoAccountFailed() throws Exception {

        try(
            PooledConnectionProviderMock connection =
                new PooledConnectionProviderMock(
                    new PooledConnectionInitializer(true, true, true));
            AccountPasswordResetterMock resetter =
                new AccountPasswordResetterMock(
                    new AccountPwdResetterInitializer(null, false));
            RegistryProviderMock registry =
                new RegistryProviderMock(
                    new RegistryProviderInitializer(
                        false /*fail*/, true/*get*/, false/*set*/, false/*hasAccount*/,
                        false/*pwdOnly*/, false/*getPoll*/, false/*saveFailThenSucceed*/)
                );
            IdmConfigProviderMock config =
                new IdmConfigProviderMock(
                    new IdmConfigInitializer()); ){

            IdmServiceAccountMgr mgr = new IdmServiceAccountMgr(
                connection, registry, config, resetter.getResetter());

            Assert.fail("Expected to fail on group membership failure");
        } catch(Exception ex) {
            // expected to fail
        }
    }

    @Test
    public void testRenewSaveRetry() throws Exception {

        try(
            PooledConnectionProviderMock connection =
                new PooledConnectionProviderMock(
                    new PooledConnectionInitializer(false, false, false));
            AccountPasswordResetterMock resetter =
                new AccountPasswordResetterMock(
                    new AccountPwdResetterInitializer(PASSWORD_RENEWED, false));
            RegistryProviderMock registry =
                new RegistryProviderMock(
                    new RegistryProviderInitializer(
                        false /*fail*/, true/*get*/, true/*set*/, true/*hasAccount*/,
                        true/*pwdOnly*/, true/*getPoll*/, true/*saveFailThenSucceed*/)
                );
            IdmConfigProviderMock config =
                new IdmConfigProviderMock(
                    new IdmConfigInitializer()); ){

            IdmServiceAccountMgr mgr = new IdmServiceAccountMgr(
                connection, registry, config, resetter.getResetter());

            mgr.startMonitorRenew();
            Thread.sleep(10*1000); // retry reg save is 5 secs
            mgr.stopMonitorRenew();
        }
    }

    @Test
    public void testEnsureAccountAdmin() throws Exception {

        try(
            PooledConnectionProviderMock connection =
                new PooledConnectionProviderMock(
                    new PooledConnectionInitializer(false, false, false, true));
            AccountPasswordResetterMock resetter =
                new AccountPasswordResetterMock(
                    new AccountPwdResetterInitializer(null, false));
            RegistryProviderMock registry =
                new RegistryProviderMock(
                    new RegistryProviderInitializer(
                        false /*fail*/, true/*get*/, false/*set*/, true/*hasAccount*/,
                        false/*pwdOnly*/, false/*getPoll*/, false/*saveFailThenSucceed*/)

                );
            IdmConfigProviderMock config =
                new IdmConfigProviderMock(
                    new IdmConfigInitializer()); ){

            IdmServiceAccountMgr mgr = new IdmServiceAccountMgr(
                connection, registry, config, resetter.getResetter());

            mgr.ensureAccountAdmin(TENANT, TENANT_ADMIN_ACCT, PASSWORD);
        }
    }

    private static class PooledConnectionInitializer
        implements IMockInitializer<ILdapConnectionEx>, Comparator<LdapMod> {

        boolean add;
        boolean delete;
        boolean fail;
        boolean tenantGroup;

        public PooledConnectionInitializer(boolean add, boolean delete, boolean fail) {
            this(add, delete, fail, false);
        }
        public PooledConnectionInitializer(boolean add, boolean delete, boolean fail, boolean tenantGroup) {
            this.add = add;
            this.delete = delete;
            this.fail = fail;
            this.tenantGroup = tenantGroup;
        }

        @Override
        public void initialize(ILdapConnectionEx object) {

            if (this.add) {
                object.addObject(
                    EasyMock.eq(STS_DN), EasyMock.anyObject(LdapMod[].class));
                EasyMock.expectLastCall();

                if (this.fail) {
                    object.modifyObject(
                        EasyMock.eq(STS_ACCOUNTS_GROUP_DN), EasyMock.anyObject(LdapMod.class));
                    EasyMock.expectLastCall().andThrow(new NoSuchObjectLdapException(11111, "object does not exist"));
                } else {
                    object.modifyObject(
                        EasyMock.eq(STS_ACCOUNTS_GROUP_DN), EasyMock.anyObject(LdapMod.class));
                    EasyMock.expectLastCall();
                }
            }

            if (this.delete) {
                object.deleteObject( EasyMock.eq(STS_DN));
                EasyMock.expectLastCall().anyTimes();
            }

            if (this.tenantGroup) {
                object.modifyObject(
                    EasyMock.eq(TENANT_ADMIN_GROUP_DN),
                    EasyMock.<LdapMod>cmp(
                        new LdapMod(LdapModOperation.ADD, "member",
                            ServerUtils.getLdapValue(Arrays.asList(STS_ACCOUNTS_GROUP_DN)
                        )), this, LogicalOperator.EQUAL )
                );
                EasyMock.expectLastCall();
            }
        }

		@Override
		public int compare(LdapMod o1, LdapMod o2) {
			if (o1 == o2) {
                return 0;
            }
            else if ( o1 == null || o2 == null ) {
                return -1;
            } else if ( ObjectUtils.equals(o1.getOperation(), o2.getOperation()) &&
                       ObjectUtils.equals(o1.getModType(), o2.getModType()) &&
                       arrayEquals(o1.getValues(), o2.getValues())) {
                return 0;
            } else {
                System.out.printf("O1: op=%s,attr=%s,val=%s\n",
                    o1.getOperation().toString(), o1.getModType(), getLdapValuesStr(o1.getValues()));
                System.out.printf("O2: op=%s,attr=%s,val=%s\n",
                    o2.getOperation().toString(), o2.getModType(), getLdapValuesStr(o2.getValues()));
                return 1;
            }
		}
    }

    private static String getLdapValuesStr(LdapValue[] a1) {
        StringBuilder b = new StringBuilder();
        if (a1 != null) {
            for(LdapValue v: a1) {
                b.append(v==null? "(NULL)": v.toString());
                b.append(",");
            }
        }
        return b.toString();
    }
    private static boolean arrayEquals(LdapValue[] a1, LdapValue[] a2) {
        if (a1 == null && a2 == null) {
            return true;
        } else if (a1==null||a2==null) {
            return false;
        } else if (a1.length != a2.length) {
            return false;
        } else {
            for(int i = 0; i < a1.length; i++) {
                if (a1[i] == null && a2[i] == null) {
                    continue;
                } else if ( a1[i] == null || a2[i] == null ) {
                    return false;
                } else {
                    if (ObjectUtils.equals(a1[i].toString(), a2[i].toString()) == false) {
                        return false;
                    }
                }
            }
            return true;
        }
    }

    private static class RegistryProviderInitializer implements IMockInitializer<IRegistryAdapter> {

        private boolean hasAccount;
        private boolean fail;
        private boolean get;
        private boolean set;
        private boolean pwdOnly;
        private boolean getPoll;
        private boolean saveFailThenSucceed;

        public RegistryProviderInitializer(
            boolean fail, boolean get, boolean set, boolean hasAccount, boolean pwdOnly, boolean getPoll, boolean saveFailThenSucceed) {
            this.fail = fail;
            this.hasAccount = hasAccount;
            this.get = get;
            this.set = set;
            this.pwdOnly = pwdOnly;
            this.getPoll = getPoll;
            this.saveFailThenSucceed = saveFailThenSucceed;
        }

        @Override
        public void initialize(IRegistryAdapter registry) {
            IRegistryKey key = new RegistryKeyMock();
            EasyMock.expect(registry.openRootKey((int) RegKeyAccess.KEY_READ))
                .andReturn(key).anyTimes();
            if (this.get ) {
                if (this.fail) {
                    EasyMock.expect(registry.getStringValue(
                        EasyMock.anyObject(IRegistryKey.class),
                        EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                        EasyMock.eq(STS_ACCOUNT_DN), EasyMock.eq(true))).andThrow(new RegistryNoSuchKeyOrValueException());
                } else {
                    EasyMock.expect(registry.getStringValue(
                        EasyMock.anyObject(IRegistryKey.class),
                        EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                        EasyMock.eq(STS_ACCOUNT_DN), EasyMock.eq(true)))
                            .andReturn(this.hasAccount?STS_DN:null).anyTimes();
                    EasyMock.expect(registry.getStringValue(
                            EasyMock.anyObject(IRegistryKey.class),
                            EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                            EasyMock.eq(STS_ACCOUNT_UPN), EasyMock.eq(true)))
                                .andReturn(this.hasAccount?STS_UPN:null).anyTimes();
                    EasyMock.expect(registry.getStringValue(
                            EasyMock.anyObject(IRegistryKey.class),
                            EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                            EasyMock.eq(STS_ACCOUNT_PWD), EasyMock.eq(true)))
                                .andReturn(this.hasAccount?PASSWORD:null).anyTimes();
                    EasyMock.expect(registry.getStringValue(
                            EasyMock.anyObject(IRegistryKey.class),
                            EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                            EasyMock.eq(STS_ACCOUNT_AUTH_TYPE), EasyMock.eq(true)))
                                .andReturn(this.hasAccount?STS_AUTH_TYPE.toString():null).anyTimes();
                    if (this.getPoll) {
                        EasyMock.expect(registry.getIntValue(
                            EasyMock.anyObject(IRegistryKey.class),
                            EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                            EasyMock.eq(STS_ACCOUNT_PWD_RESET_POLL_INTERVAL), EasyMock.eq(true)))
                                .andReturn(new Integer(STS_POLL)).anyTimes();
                    }
                }
            }
            if (this.set) {
                if (this.fail) {
                    EasyMock.expect(registry.openKey(
                        EasyMock.anyObject(IRegistryKey.class),
                        EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                        EasyMock.eq(0),
                        EasyMock.eq((int)RegKeyAccess.KEY_SET_VALUE|(int)RegKeyAccess.KEY_READ)
                        )).andThrow(new RegistryNoSuchKeyOrValueException());
                }
                if ( (!this.fail) || this.saveFailThenSucceed) {
                    EasyMock.expect(registry.openKey(
                        EasyMock.anyObject(IRegistryKey.class),
                        EasyMock.eq(IdmServerConfig.CONFIG_ROOT_KEY),
                        EasyMock.eq(0),
                        EasyMock.eq((int)RegKeyAccess.KEY_SET_VALUE|(int)RegKeyAccess.KEY_READ)
                        )).andReturn(key).anyTimes();

                    registry.setStringValue(
                        EasyMock.anyObject(IRegistryKey.class),
                        EasyMock.eq(STS_ACCOUNT_PWD),
                        EasyMock.eq(this.pwdOnly ? PASSWORD_RENEWED : PASSWORD)
                        );
                    EasyMock.expectLastCall().anyTimes();

                    if (!this.pwdOnly) {
                        registry.setStringValue(
                            EasyMock.anyObject(IRegistryKey.class),
                            EasyMock.eq(STS_ACCOUNT_DN),
                            EasyMock.eq(STS_DN)
                            );
                        EasyMock.expectLastCall().anyTimes();
                        registry.setStringValue(
                            EasyMock.anyObject(IRegistryKey.class),
                            EasyMock.eq(STS_ACCOUNT_UPN),
                            EasyMock.eq(STS_UPN)
                            );
                        EasyMock.expectLastCall().anyTimes();
                        registry.setStringValue(
                            EasyMock.anyObject(IRegistryKey.class),
                            EasyMock.eq(STS_ACCOUNT_AUTH_TYPE),
                            EasyMock.eq(STS_AUTH_TYPE.toString())
                            );
                        EasyMock.expectLastCall().anyTimes();
                    }
                }
            }
        }

    }

    private static class AccountPwdResetterInitializer implements IMockInitializer<IAccountPasswordResetter> {

        String ret;
        boolean fail;

        public AccountPwdResetterInitializer(String ret, boolean fail) {
            this.ret = ret;
            this.fail = fail;
        }

        @Override
        public void initialize(IAccountPasswordResetter object) {
            if ( this.fail) {
                EasyMock.expect(object.ResetAccountPassword(
                    LDAP_HOST, DOMAIN, STS_UPN, STS_DN, PASSWORD, false))
                    .andThrow(new DirectoryException(111, "reset pwd failed"));
            } else if ( this.ret != null ) { // do not expect to call
                EasyMock.expect(object.ResetAccountPassword(
                    LDAP_HOST, DOMAIN, STS_UPN, STS_DN, PASSWORD, false)).andReturn(this.ret).anyTimes();
            }
        }
    }

    private static class IdmConfigInitializer implements IMockInitializer<IIdmConfig> {
        public IdmConfigInitializer() {}

        @Override
        public void initialize(IIdmConfig cfg) {
            EasyMock.expect(cfg.getDirectoryConfigStoreAuthType()).andReturn(STS_AUTH_TYPE).anyTimes();
            EasyMock.expect(cfg.getTenantAdminUserName(TENANT, TENANT_ADMIN_ACCT)).andReturn(TENANT_ADMIN_UPN).anyTimes();
            EasyMock.expect(cfg.getDirectoryConfigStoreDomain()).andReturn(DOMAIN).anyTimes();
            EasyMock.expect(cfg.getDirectoryConfigStorePassword()).andReturn(PASSWORD).anyTimes();
            EasyMock.expect(cfg.getDirectoryConfigStoreUserName()).andReturn(DC_UPN).anyTimes();
            EasyMock.expect(cfg.getSystemDomainConnectionInfo())
                .andReturn(Arrays.<URI>asList(LDAP_URI)).anyTimes();
        }

    }
    private static URI LDAP_URI;
    private static final String LDAP_HOST = "localhost";
    private static final String LDAP_STR = "ldap://localhost:389";
    private static final String DOMAIN = "lw-testdom.com";
    private static final String PASSWORD = "password";
    private static final String PASSWORD_RENEWED = "password1";
    private static final String DC_UPN = "hostname.lw-testdom.com@LW-TESTDOM.COM";
    private static final String STS_UPN = "sts/hostname.lw-testdom.com@LW-TESTDOM.COM";
    private static final String STS_DN = "cn=sts/hostname.lw-testdom.com@LW-TESTDOM.COM,cn=Managed Service Accounts,DC=LW-TESTDOM,DC=COM";
    private static final AuthenticationType STS_AUTH_TYPE = AuthenticationType.SRP;
    private static final int STS_POLL = 5;
    private static final String STS_ACCOUNTS_GROUP_DN = "cn=STSAccounts,DC=LW-TESTDOM,DC=COM";
    private static final String TENANT = "coke";
    private static final String TENANT_ADMIN_ACCT = "Administrator";
    private static final String TENANT_ADMIN_UPN = "Administrator@coke";
    private static final String TENANT_ADMIN_GROUP_DN = "cn=Administrators,cn=Builtin,DC=coke";

    private static String STS_ACCOUNT_DN = "stsAccountDN";
    private static String STS_ACCOUNT_UPN = "stsAccountUpn";
    private static String STS_ACCOUNT_PWD = "stsAccountPassword";
    private static String STS_ACCOUNT_AUTH_TYPE = "stsAccountAuth";
    private static String STS_ACCOUNT_PWD_RESET_POLL_INTERVAL = "stsAccountPollInterval";
}

