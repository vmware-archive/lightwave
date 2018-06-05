package com.vmware.identity.idm.server;

import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Collections;
import java.util.NoSuchElementException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import org.apache.commons.lang.RandomStringUtils;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.server.provider.LdapConnectionPool;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PooledLdapConnectionIdentity;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.ILdapMessage;
import com.vmware.identity.interop.ldap.ILdapPagedSearchResult;
import com.vmware.identity.interop.ldap.LdapBindMethod;
import com.vmware.identity.interop.ldap.LdapControlNative;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapOption;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.TimevalNative;

@Ignore
public class LdapConnectionPoolTest {

    private static final String connectionString = "ldap://localhost";
    private static final AuthenticationType authType = AuthenticationType.SRP;
    private static final String username = "administrator@vsphere.local";
    private static final String password = "Admin!23";
    private static final String CFG_KEY_LDAPS_KEY_ALIAS = "/slapd.cer";
    private final String newTenantName = RandomStringUtils.randomAlphabetic(5);
    private String invalidConnectionString = "https://localhost";
    private String ldapsConnectionString = "ldaps://openldap-1.ssolabs.eng.vmware.com";
    private String ldapsUsername = "cn=administrator,dc=ssolabs-openldap,dc=eng,dc=vmware,dc=com";
    private String ldapsPassword = "123";
    private static LdapConnectionPool pool;

    @Test
    public void test_borrowConnection_SystemTenant() {

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);

	builder.setUsername(username);
	builder.setPassword(password);

	try {
		PooledLdapConnection pooledConnection = LdapConnectionPool.getInstance().borrowConnection(builder.build());
		ILdapConnectionEx connection = pooledConnection.getConnection();

	    connection.search("", LdapScope.SCOPE_SUBTREE, "", new String[0], false);
	} catch (Exception e) {
	    Assert.fail(e.getMessage());
	}
    }

    @Test
    public void test_borrowConnection_newTenant() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName);

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName);

	pool.borrowConnection(builder.build());
	pool.cleanPool(newTenantName);

	try {
	    pool.borrowConnection(builder.build());
	} catch (IllegalStateException e) {
	    return;
	    // expected
	}

	Assert.fail("Should not reach here");
    }

    @Test(expected = IllegalStateException.class)
    public void test_borrowConnection_tenantDoesNoExist_ShouldThrow() throws Exception {

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName("InvalidTenantName");

	LdapConnectionPool.getInstance().borrowConnection(builder.build());
    }

    @Test(expected = NoSuchElementException.class)
    public void test_borrowConnection_SystemTenant_InvalidConnection() throws Exception {

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(
		invalidConnectionString, authType);

	builder.setUsername(username);
	builder.setPassword(password);

	LdapConnectionPool.getInstance().borrowConnection(builder.build());
    }

    @Test
    public void test_borrowConnection_newTenant_LdapsConnection() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName);

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(ldapsConnectionString,
		AuthenticationType.PASSWORD);

	builder.setTenantName(newTenantName);
	builder.setUsername(ldapsUsername);
	builder.setPassword(ldapsPassword);

	builder.setIdsTrustedCertificates(getLdapsCertificate());

	LdapConnectionPool.getInstance().borrowConnection(builder.build());

	pool.cleanPool(newTenantName);
    }

    @Test
    public void test_createConnectioPool_CallTwice() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName);
	pool.createPool(newTenantName);

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName);

	pool.borrowConnection(builder.build());
	pool.cleanPool(newTenantName);

	try {
	    pool.borrowConnection(builder.build());
	} catch (IllegalStateException e) {
	    return;
	    // expected
	}

	Assert.fail("Should not reach here");
    }

    @Test
    public void test_returnConnection() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName);

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName);

	PooledLdapConnectionIdentity identity = builder.build();

	PooledLdapConnection conn = pool.borrowConnection(identity);
	pool.returnConnection(conn);
    }

    @Test
    public void test_returnConnection_NullIdentity() {
        LdapConnectionPool pool = LdapConnectionPool.getInstance();
        pool.createPool(newTenantName);
        LdapConnectionExTest conn = new LdapConnectionExTest();
        PooledLdapConnection pooledLdapConnectionWithNullIdentity = new PooledLdapConnection(conn, null, pool);
        pool.returnConnection(pooledLdapConnectionWithNullIdentity);
        Assert.assertTrue(conn.isClosed());
    }

    @Test
    public void test_returnConnection_BuildNewIdentity() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName);

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName);

	PooledLdapConnection conn = pool.borrowConnection(builder.build());
	pool.returnConnection(conn);
    }

    // Throws NullPointerException because the key is not found in the pool
    @Test(expected = NullPointerException.class)
    public void test_returnConnection_ModifiedIdentity_ShouldThrow() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName);

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName);

	PooledLdapConnectionIdentity identity = builder.build();
	PooledLdapConnection conn = pool.borrowConnection(identity);
	builder.setUseGCPort(true);
	identity = builder.build();
	pool.returnConnection(new PooledLdapConnection(conn.getConnection(), identity, pool));
    }

    @Test(expected = IllegalStateException.class)
    public void test_returnConnection_ReturnTwise_ShouldThrow() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName);

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName);

	PooledLdapConnectionIdentity identity = builder.build();
	PooledLdapConnection conn = pool.borrowConnection(identity);

	pool.returnConnection(conn);

	pool.returnConnection(conn);
    }

    @Test
    public void test_createConnectioPool_CaseInsensitive() throws Exception {

	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.createPool(newTenantName.toLowerCase());

	PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName.toUpperCase());

	pool.borrowConnection(builder.build());
	pool.cleanPool(newTenantName);
    }

    @Test
    public void test_createConnectioPool_MultiThreading() throws Exception {

	final PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(connectionString,
		authType);
	builder.setUsername(username);
	builder.setPassword(password);
	builder.setTenantName(newTenantName.toUpperCase());

	ExecutorService executorService = Executors.newFixedThreadPool(50);

	for (int i = 0; i < 50; i++) {
	    executorService.execute(new Runnable() {
		@Override
		public void run() {
		    LdapConnectionPool pool = LdapConnectionPool.getInstance();
		    pool.createPool(newTenantName);
		    try {
			PooledLdapConnectionIdentity identity = builder.build();
			PooledLdapConnection conn = pool.borrowConnection(identity);
			pool.returnConnection(conn);
		    } catch (Exception e) {
			Assert.fail(e.getMessage() + " " + e.getCause().getMessage());
		    }
		}
	    });
	}

	executorService.shutdown();
	executorService.awaitTermination(1, TimeUnit.MINUTES);
	LdapConnectionPool pool = LdapConnectionPool.getInstance();
	pool.cleanPool(newTenantName);

	try {
	    pool.borrowConnection(builder.build());
	} catch (IllegalStateException e) {
	    // expected
	    return;
	}

	Assert.fail("Should not reach here");
    }

    private static Collection<X509Certificate> getLdapsCertificate() {
	try {

	    CertificateFactory cf = CertificateFactory.getInstance("X.509");
	    X509Certificate slapdCert = (X509Certificate) cf.generateCertificate(LdapProviderTest.class
		    .getResourceAsStream(CFG_KEY_LDAPS_KEY_ALIAS));

	    if (slapdCert != null)
		return java.util.Collections.singletonList(slapdCert);
	    else
		return null;

	} catch (Exception e) {
	    throw new IllegalStateException(e);
	}
    }

    private static class LdapConnectionExTest implements ILdapConnectionEx {

        private boolean isClosed = false;

        @Override
        public void setOption(LdapOption option, int value) {

        }

        @Override
        public void setOption(LdapOption option, boolean value) {

        }

        @Override
        public void bindConnection(String dn, String cred, LdapBindMethod method) {

        }

        @Override
        public void bindSaslConnection(String userName, String domainName,
                String userPassword, String krbConfPath) {

        }

        @Override
        public void addObject(String dn, LdapMod[] attributes) {

        }

        @Override
        public void addObject(String dn, Collection<LdapMod> attributes) {

        }

        @Override
        public void modifyObject(String dn, LdapMod attribute) {

        }

        @Override
        public void modifyObject(String dn, LdapMod[] attributes) {

        }

        @Override
        public void modifyObject(String dn, Collection<LdapMod> attributes) {

        }

        @Override
        public ILdapMessage search(String base, LdapScope scope, String filter,
                String[] attributes, boolean attributesOnly) {
            return null;
        }

        @Override
        public ILdapMessage search(String base, LdapScope scope, String filter,
                Collection<String> attributes, boolean attributesOnly) {
            return null;
        }

        @Override
        public ILdapMessage search_ext(String base, LdapScope scope,
                String filter, Collection<String> attributes,
                boolean attributesOnly, Collection<LdapControlNative> sctrls,
                Collection<LdapControlNative> cctrls, TimevalNative timeout,
                int sizelimit) {
            return null;
        }

        @Override
        public ILdapPagedSearchResult search_one_page(String base,
                LdapScope scope, String filter, Collection<String> attributes,
                int pageSize, ILdapPagedSearchResult prevPagedSearchResult) {
            return null;
        }

        @Override
        public Collection<ILdapMessage> paged_search(String base,
                LdapScope scope, String filter, Collection<String> attributes,
                int pageSize) {
            return null;
        }

        @Override
        public void deleteObject(String dn) {

        }

        @Override
        public void deleteObjectTree(String dn) {

        }

        @Override
        public void close() {
            this.isClosed = true;
        }

        boolean isClosed() {
            return this.isClosed;
        }

        @Override
        public void bindSaslSrpConnection(String upn, String userPassword) {

        }

        @Override
        public void bindSaslConnection(String userName, String domainName,
                String userPassword) {

        }

        @Override
        public Collection<ILdapMessage> paged_search(String base,
                LdapScope scope, String filter, Collection<String> attributes,
                int pageSize, int limit) {
            return null;
        }
    }
}
