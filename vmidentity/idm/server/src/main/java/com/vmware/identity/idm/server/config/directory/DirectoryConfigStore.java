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

/**
 * VMware Identity Service
 *
 * (LDAP) Directory Configuration Manager
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.identity.idm.server.config.directory;

import java.net.URI;
import java.security.PrivateKey;
import java.security.SecureRandom;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;

import sun.misc.BASE64Decoder;
import sun.misc.BASE64Encoder;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AlternativeOCSP;
import com.vmware.identity.idm.AlternativeOCSPList;
import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeConfig;
import com.vmware.identity.idm.AttributeConsumerService;
import com.vmware.identity.idm.CertificateInUseException;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.CertificateUtil;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.DuplicateCertificateException;
import com.vmware.identity.idm.DuplicateProviderException;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdentityStoreDataEx;
import com.vmware.identity.idm.IdentityStoreAttributeMapping;
import com.vmware.identity.idm.IdentityStoreObjectMapping;
import com.vmware.identity.idm.IdentityStoreSchemaMapping;
import com.vmware.identity.idm.IdentityStoreType;
import com.vmware.identity.idm.LocalISRegistrationException;
import com.vmware.identity.idm.NoSuchCertificateException;
import com.vmware.identity.idm.NoSuchExternalIdpConfigException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.NoSuchRelyingPartyException;
import com.vmware.identity.idm.NoSuchResourceServerException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.SignatureAlgorithm;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.server.CryptoAESE;
import com.vmware.identity.idm.server.IdmCertificate;
import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.IConfigStore;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.idm.server.config.ServerIdentityStoreData;
import com.vmware.identity.idm.server.provider.LdapConnectionPool;
import com.vmware.identity.idm.server.provider.PooledLdapConnection;
import com.vmware.identity.idm.server.provider.PooledLdapConnectionIdentity;
import com.vmware.identity.idm.server.provider.localos.LocalOsIdentityProvider;
import com.vmware.identity.interop.directory.Directory;
import com.vmware.identity.interop.ldap.AlreadyExistsLdapException;
import com.vmware.identity.interop.ldap.AttributeOrValueExistsLdapException;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;
import com.vmware.identity.interop.ldap.LdapFilterString;
import com.vmware.identity.interop.ldap.LdapMod;
import com.vmware.identity.interop.ldap.LdapMod.LdapModOperation;
import com.vmware.identity.interop.ldap.LdapScope;
import com.vmware.identity.interop.ldap.LdapValue;
import com.vmware.identity.interop.ldap.NoSuchAttributeLdapException;
import com.vmware.identity.interop.ldap.NoSuchObjectLdapException;

public class DirectoryConfigStore implements IConfigStore {

    private static final String SYSTEM_TENANT = "SystemTenant";
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(DirectoryConfigStore.class);

    public static final int FLAG_AUTHN_TYPE_ALLOW_NONE = 0x0;
    public static final int FLAG_AUTHN_TYPE_ALLOW_PASSWORD = 0x1;
    public static final int FLAG_AUTHN_TYPE_ALLOW_WINDOWS = 0x2;
    public static final int FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE = 0x4;
    public static final int FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID = 0x8;

    private static final String CLAIM_GROUP_DELIMITER = "#";
    private final Collection<URI> _uris;
    private final String _configRootDn; // dc=vsphere,dc=local

    private final String tenantCredNamePrefix = "TenantCredential";
    private final String tenantTrustedCertChainPrefix = "TrustedCertChain";
    private final String OBJECT_CLASS_CISLDU = "vmwCisLdu";

    public DirectoryConfigStore(Collection<URI> uris, String configRootDn) {
	ValidateUtil.validateNotEmpty(configRootDn, "domain");

	this._uris = uris;
	this._configRootDn = configRootDn;
    }

    private String randomKey() {
	SecureRandom random = new SecureRandom();
	StringBuilder sb = new StringBuilder();
	for (int i = 0; i < 16; i++)
	    sb.append((char) (random.nextInt(96) + 32));
	return sb.toString();
    }

    @Override
    public void addTenant(Tenant tenant, String adminAccountName, char[] adminPwd) throws Exception {
	ValidateUtil.validateNotNull(tenant, "tenant");
	ValidateUtil.validateNotEmpty(tenant.getName(), "tenant.getName()");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String identityManagerDn = this.getRootSystemConfigDn(connection, true);

	    if (ServerUtils.isNullOrEmpty(tenant._guid)) {
		tenant._guid = UUID.randomUUID().toString();
	    }

	    // Generate tenantKey
	    tenant._tenantKey = this.randomKey();

	    // ensure tenants container
	    String tenantsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, identityManagerDn,
		    TenantsContainerLdapObject.getInstance(), TenantsContainerLdapObject.CONTAINER_TENANTS, true);

	    TenantLdapObject tenantObject = TenantLdapObject.getInstance();
	    String tenantDn = tenantObject.getDnFromObject(tenantsContainerDn, tenant);
	    tenantObject.createObject(connection, tenantDn, tenant);

	    this.createSystemDomainIdentityProviderForTenant(connection, tenant.getName(), tenantDn, adminAccountName,
		    adminPwd);
	}
    }

    @Override
    public void deleteTenant(String name) throws Exception {
	ValidateUtil.validateNotEmpty(name, "name");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.lookupTenantsRootDn(connection, name);
	    if (ServerUtils.isNullOrEmpty(tenantsRootDn) == false) {
		String defaultTenant = this.getDefaultTenant(connection);

		if (name.equalsIgnoreCase(defaultTenant)) {
		    this.setDefaultTenant(connection, null);
		}

		TenantLdapObject tenantConfigObject = TenantLdapObject.getInstance();
		tenantConfigObject.deleteObject(connection, tenantsRootDn);
	    }
	}
    }

    @Override
    public Tenant getTenant(String name) throws Exception {
	ValidateUtil.validateNotEmpty(name, "name");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    return this.getTenant(connection, name);
	}
    }

    @Override
    public Collection<String> getAllTenants() throws Exception {
	Collection<String> allTenantNames = null;

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    allTenantNames = this.lookupAllTenantsDn(pooledConnection.getConnection());
	}

	return allTenantNames;
    }

    @Override
    public void setTenant(Tenant tenant) throws Exception {
	ValidateUtil.validateNotNull(tenant, "tenant");
	ValidateUtil.validateNotEmpty(tenant.getName(), "tenant.getName()");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenant.getName());

	    TenantLdapObject tenantConfigObject = TenantLdapObject.getInstance();
	    tenantConfigObject.updateObject(connection, tenantsRootDn, tenant);
	}
    }

    @Override
    public String getDefaultTenant() throws Exception {
	String defaultTenant = null;

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    defaultTenant = this.getDefaultTenant(connection);
	}

	return defaultTenant;
    }

    @Override
    public void setDefaultTenant(String tenantName) throws Exception {
	// should we allow null to "erase" the default tenant ?
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    this.ensureTenantExists(connection, tenantName);
	    this.setDefaultTenant(connection, tenantName);
	}
    }

    @Override
    public String getSystemTenant() throws Exception {
	String systemTenant = null;

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    systemTenant = this.getSystemTenant(connection);
	}

	return systemTenant;
    }

    @Override
    public void setSystemTenant(String tenantName) throws Exception {
	// should we allow null to "erase" the default tenant ?
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    this.ensureTenantExists(connection, tenantName);
	    this.setSystemTenant(connection, tenantName);
	}
    }

    @Override
    public Collection<List<Certificate>> getTenantCertChains(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    return getTenantCertificatesForTenant(tenantName, tenantsRootDn, connection);
	}
    }

    @Override
    public List<Certificate> getTenantCertificate(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    return getTenantCertificateForTenant(tenantName, tenantsRootDn, connection);
	}
    }

    private int getCurrMaxCredIndex(Set<String> credDns) {
	int currMaxCredIndex = 0;

	if (credDns == null || credDns.isEmpty()) {
	    return 0;
	}

	// found the biggest index
	for (String credDn : credDns) {
	    if (credDn != null && !credDn.isEmpty()) {
		// A sample credDn looks like:
		// CN=TenantCredential-i,CN=ldu-guid,CN=Ldus,CN=ComponentManager,dc=vsphere,dc=local
		// for systemTenant
		String indexStr = credDn.substring(credDn.indexOf('-') + 1, credDn.indexOf(','));
		if (indexStr != null && !indexStr.isEmpty()) {
		    int currIndex = Integer.parseInt(indexStr);
		    if (currMaxCredIndex < currIndex) {
			currMaxCredIndex = currIndex;
		    }
		}
	    }
	}

	return currMaxCredIndex;
    }

    private TenantCredentialInformation getLastSetTenantCred(String tenantName, String tenantRootDn,
	    ILdapConnectionEx connection) throws Exception {
	TenantCredentialsLdapObject tenantCredsObj = TenantCredentialsLdapObject.getInstance();
	TenantCredentialInformation lateSetTenantCred = null;

	// multi-tenantCredential is supported,
	// we only return the latest set tenant credential's certChain ( we need
	// decide what algorithm to use
	// in order to determine which tenantCredential record should be used to
	// return information for this API
	Set<String> tenantCredsDns = tenantCredsObj.lookupObjects(connection, tenantRootDn, LdapScope.SCOPE_ONE_LEVEL,
		null, TenantCredentialsLdapObject.OBJECT_CLASS);

	if (tenantCredsDns != null && tenantCredsDns.size() >= 1) {

	    String tenantCredsDn = this.getTenantCredentialDn(getCurrMaxCredIndex(tenantCredsDns), tenantRootDn);
	    lateSetTenantCred = tenantCredsObj.retrieveObject(connection, tenantCredsDn, LdapScope.SCOPE_BASE, null);
	}

	return lateSetTenantCred;
    }

    private List<Certificate> getTenantCertificateForTenant(String tenantName, String tenantRootDn,
	    ILdapConnectionEx connection) throws Exception {
	TenantCredentialInformation lateSetTenantCred = getLastSetTenantCred(tenantName, tenantRootDn, connection);

	return (lateSetTenantCred == null) ? null : lateSetTenantCred.getCertificateChain();
    }

    private Collection<List<Certificate>> getTenantCertificatesForTenant(String tenantName, String tenantRootDn,
	    ILdapConnectionEx connection) throws Exception {
	Collection<List<Certificate>> certChains = Collections.emptyList();

	Collection<TenantTrustedCertificateChain> trustedCertChains = retrieveObjectsCollection(connection,
		tenantRootDn, ContainerLdapObject.CONTAINER_TRUSTED_CERTIFICATE_CHAINS,
		TenantTrustedCertChainLdapObject.getInstance(), null);

	if (trustedCertChains != null && !trustedCertChains.isEmpty()) {
	    certChains = new ArrayList<List<Certificate>>(trustedCertChains.size());
	    for (TenantTrustedCertificateChain tenantCred : trustedCertChains) {
		certChains.add(tenantCred.getCertificateChain());
	    }
	}

	return certChains;
    }

    @Override
    public void setTenantCredentials(String tenantName, Collection<Certificate> tenantCertificates,
	    PrivateKey tenantPrivateKey) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    setTenantCredentialsForTenant(tenantName, tenantsRootDn, connection, tenantCertificates, tenantPrivateKey);
	}
    }

    private void setTenantCredentialsForTenant(String tenantName, String tenantsRootDn, ILdapConnectionEx connection,
	    Collection<Certificate> tenantCertificates, PrivateKey tenantPrivateKey) throws Exception {
	TenantCredentialsLdapObject tenantCredsObj = TenantCredentialsLdapObject.getInstance();
	Set<String> tenantCredsDns = tenantCredsObj.lookupObjects(connection, tenantsRootDn, LdapScope.SCOPE_ONE_LEVEL,
		null, TenantCredentialsLdapObject.OBJECT_CLASS);

	// construct the new tenantCredential's CN by indexing it with the
	// highest index plus 1
	int credsIndex = getCurrMaxCredIndex(tenantCredsDns) + 1;

	TenantCredentialInformation tenantCredInfo = new TenantCredentialInformation(
		this.getTenantCredentialCn(credsIndex), tenantPrivateKey,
		new ArrayList<Certificate>(tenantCertificates));

	String tenantCredsDn = tenantCredsObj.getDnFromObject(tenantsRootDn, tenantCredInfo);
	tenantCredsObj.createObject(connection, tenantCredsDn, tenantCredInfo);

	// Duplicate the tenant signing certs in trustedCertificatChain for
	// better lookup
	// MEMO: upon a signing credential deletion, needs to remove this
	// trusted CertChain record
	setTenantTrustedCertificateChainForTenant(tenantName, tenantsRootDn, connection, tenantCertificates);
    }

    private void deleteTenantCredentialsForTenant(String tenantName, String tenantsRootDn,
	    ILdapConnectionEx connection, Collection<Certificate> tenantCertificates) throws Exception {
	TenantCredentialsLdapObject tenantCredsObj = TenantCredentialsLdapObject.getInstance();
	Set<String> tenantCredsDns = tenantCredsObj.lookupObjects(connection, tenantsRootDn, LdapScope.SCOPE_ONE_LEVEL,
		null, TenantCredentialsLdapObject.OBJECT_CLASS);

	for (String tenantCredsDn : tenantCredsDns) {
	    List<TenantCredentialInformation> tenantCred = tenantCredsObj.searchObjects(connection, tenantCredsDn,
		    LdapScope.SCOPE_BASE);
	    if (tenantCred != null && !tenantCred.isEmpty() && tenantCred.size() == 1) {
		if (tenantCred.get(0).getCertificateChain().equals(tenantCertificates)) {
		    tenantCredsObj.deleteObject(connection, tenantCredsDn);
		}
	    }
	}
    }

    @Override
    public void setTenantTrustedCertificateChain(String tenantName, Collection<Certificate> tenantCertificates)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    setTenantTrustedCertificateChainForTenant(tenantName, tenantsRootDn, connection, tenantCertificates);
	}
    }

    private void setTenantTrustedCertificateChainForTenant(String tenantName, String tenantsRootDn,
	    ILdapConnectionEx connection, Collection<Certificate> tenantCertificates) throws Exception {
	String tenantCertChainContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_TRUSTED_CERTIFICATE_CHAINS, true);

	TenantTrustedCertChainLdapObject tenantTrustedCertChainObj = TenantTrustedCertChainLdapObject.getInstance();
	Set<String> tenantTrustedCertchainDns = tenantTrustedCertChainObj.lookupObjects(connection,
		tenantCertChainContainerDn, LdapScope.SCOPE_ONE_LEVEL, null,
		TenantTrustedCertChainLdapObject.OBJECT_CLASS);

	// construct the new tenantCredential's CN by indexing it with the
	// highest index plus 1
	int credsIndex = getCurrMaxCredIndex(tenantTrustedCertchainDns) + 1;

	TenantTrustedCertificateChain tenantTrusterCertChainInfo = new TenantTrustedCertificateChain(
		this.getTenantTrustedCertChainCn(credsIndex), new ArrayList<Certificate>(tenantCertificates));

	String tenantTrustedCertChainDn = tenantTrustedCertChainObj.getDnFromObject(tenantCertChainContainerDn,
		tenantTrusterCertChainInfo);
	tenantTrustedCertChainObj.createObject(connection, tenantTrustedCertChainDn, tenantTrusterCertChainInfo);
    }

    private void deleteTenantTrustedCertificateChainForTenant(String tenantName, String tenantsRootDn,
	    ILdapConnectionEx connection, List<Certificate> tenantCertificates) throws Exception {
	String tenantCertChainContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_TRUSTED_CERTIFICATE_CHAINS, true);

	TenantTrustedCertChainLdapObject tenantTrusterCerdChainObj = TenantTrustedCertChainLdapObject.getInstance();
	Set<String> tenantTrustedCertchainDns = tenantTrusterCerdChainObj.lookupObjects(connection,
		tenantCertChainContainerDn, LdapScope.SCOPE_ONE_LEVEL, null,
		TenantTrustedCertChainLdapObject.OBJECT_CLASS);

	for (String certChainDn : tenantTrustedCertchainDns) {
	    List<TenantTrustedCertificateChain> certChain = tenantTrusterCerdChainObj.searchObjects(connection,
		    certChainDn, LdapScope.SCOPE_BASE);
	    if (certChain != null && !certChain.isEmpty() && certChain.size() == 1) {
		if (certChain.get(0).getCertificateChain().equals(tenantCertificates)) {
		    tenantTrusterCerdChainObj.deleteObject(connection, certChainDn);
		}
	    }
	}
    }

    @Override
    public PrivateKey getTenantPrivateKey(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    return getTenantPrivateKeyForTenant(tenantName, tenantsRootDn, connection);
	}
    }

    private PrivateKey getTenantPrivateKeyForTenant(String tenantName, String tenantRootDn, ILdapConnectionEx connection)
	    throws Exception {
	TenantCredentialInformation lateSetTenantCred = getLastSetTenantCred(tenantName, tenantRootDn, connection);

	return (lateSetTenantCred == null) ? null : lateSetTenantCred.getPrivateKey();
    }

    @Override
    public void setAlias(String tenantName, String alias) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(alias, "alias");

	LdapValue[] value = ServerUtils.getLdapValue(alias);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_ALIAS, value);
    }

    @Override
    public void setTenantIDPSelectionEnabled(String tenantName, boolean enableIDPSelection) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	LdapValue[] value = ServerUtils.getLdapValue(enableIDPSelection);
	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_ENABLE_IDP_SELECTION, value);
    }

    @Override
    public TenantAttributes getTokenPolicyExt(String tenantName) throws Exception {
	String tenantsRootDn = null;
	TenantAttributes tenantAttributes = null;

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    tenantsRootDn = this.lookupTenantsRootDn(connection, tenantName);
	    if (ServerUtils.isNullOrEmpty(tenantsRootDn)) {
		throw new RuntimeException(String.format("Tenant '%s' does not exist.", tenantName));
	    }

	    TenantAttributesLdapObject tenantAttributesObject = TenantAttributesLdapObject.getInstance();

	    tenantAttributes = tenantAttributesObject.retrieveObject(connection, tenantsRootDn, LdapScope.SCOPE_BASE,
		    null);
	}

	return tenantAttributes;
    }

    @Override
    public long getClockTolerance(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_CLOCK_TOLERANCE);

	return value != null ? ServerUtils.getNativeLongValue(value) : IConfigStore.DEFAULT_CLOCK_TOLERANCE;
    }

    @Override
    public void setClockTolerance(String tenantName, long milliseconds) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNonNegativeNumber(milliseconds, "milliseconds");

	LdapValue[] value = ServerUtils.getLdapValue(milliseconds);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_CLOCK_TOLERANCE, value);
    }

    @Override
    public String getSignatureAlgorithm(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_SIGNATURE_ALGORITHM);

	return value != null ? ServerUtils.getStringValue(value) : null;
    }

    @Override
    public void setSignatureAlgorithm(String tenantName, String signatureAlgorithm) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(signatureAlgorithm, "signatureAlgorithm");

	LdapValue[] value = ServerUtils.getLdapValue(signatureAlgorithm);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_SIGNATURE_ALGORITHM, value);
    }

    @Override
    public String getBrandName(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_BRAND_NAME);

	return value != null ? ServerUtils.getStringValue(value) : null;
    }

    @Override
    public void setBrandName(String tenantName, String brandName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = ServerUtils.getLdapValue(brandName);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_BRAND_NAME, value);
    }

    @Override
    public String getLogonBannerContent(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_LOGON_BANNER_CONTENT);
	return value != null ? ServerUtils.getStringValue(value) : null;
    }

    @Override
    public String getLogonBannerTitle(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_LOGON_BANNER_TITLE);
	return value != null ? ServerUtils.getStringValue(value) : null;
    }

    @Override
    public boolean getLogonBannerCheckboxFlag(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_LOGON_BANNER_ENABLE_CHECKBOX);
	return value != null ? ServerUtils.getBooleanValue(value) : false;
    }

    @Override
    public void setLogonBannerContent(String tenantName, String logonBannerContent) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	LdapValue[] value = ServerUtils.getLdapValue(logonBannerContent);
	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_LOGON_BANNER_CONTENT, value);
    }

    @Override
    public void setAuthnTypesForProvider(String tenantName, String providerName, boolean password, boolean windows,
	    boolean certificate, boolean rsaSecureID) throws Exception {
	HashSet<Integer> authnTypes = new HashSet<Integer>();
	if (password) {
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_PASSWORD);
	}
	if (windows) {
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_WINDOWS);
	}
	if (certificate) {
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE);
	}
	if (rsaSecureID) {
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID);
	}
	if (authnTypes.size() == 0) {
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_NONE);
	}

	int[] authTypesArray = ArrayUtils.toPrimitive(authnTypes.toArray(new Integer[authnTypes.size()]));

	this.setProviderProperty(tenantName, providerName, IdentityProviderLdapObject.PROPERTY_AUTHN_TYPES,
		ServerUtils.getLdapValue(authTypesArray));
    }

    /**
     * Add/Update property for a given identity provider
     *
     * @param tenantName
     *            name of tenant
     * @param providerName
     *            name of identity source
     * @param propertyName
     *            Name of property to be added
     * @param propertyValue
     *            Value of property to be added
     * @throws Exception
     */
    private void setProviderProperty(String tenantName, String providerName, String propertyName,
	    LdapValue[] propertyValue) throws Exception {
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    // Fetch tenant root dn
	    String tenantsRootDn = this.lookupTenantsRootDn(connection, tenantName);
	    validateTenantExistence(tenantsRootDn);

	    // Fetch identity provider container DN
	    String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, true);

	    // Lookup identity provider dn
	    String identityProviderRootDn = IdentityProviderLdapObject.getInstance().lookupObject(connection,
		    identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, providerName);

	    setProviderProperty(connection, identityProviderRootDn, propertyName, propertyValue);
	}
    }

    /**
     * Set property for an identity provider
     *
     */
    private static void setProviderProperty(ILdapConnectionEx connection, String identityProvidersContainerDn,
	    String propertyName, LdapValue[] propertyValue) throws Exception {
	IdentityProviderLdapObject.getInstance().setObjectPropertyValue(connection, identityProvidersContainerDn,
		propertyName, propertyValue);
    }

    private void validateTenantExistence(String tenant) throws NoSuchTenantException {
	if (ServerUtils.isNullOrEmpty(tenant)) {
	    throw new NoSuchTenantException(String.format("Tenant '%s' does not exist.", tenant));
	}
    }

    @Override
    public void setLogonBannerCheckboxFlag(String tenantName, boolean logonBannerEnableCheckbox) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	LdapValue[] value = ServerUtils.getLdapValue(logonBannerEnableCheckbox);
	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_LOGON_BANNER_ENABLE_CHECKBOX, value);
    }

    @Override
    public void setLogonBannerTitle(String tenantName, String logonBannerTitle) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	LdapValue[] value = ServerUtils.getLdapValue(logonBannerTitle);
	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_LOGON_BANNER_TITLE, value);
    }

    @Override
    public int getDelegationCount(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_DELEGATION_COUNT);

	return value != null ? ServerUtils.getIntValue(value) : IConfigStore.DEFAULT_DELEGATION_COUNT;
    }

    @Override
    public void setDelegationCount(String tenantName, int delegationCount) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNonNegativeNumber(delegationCount, "delegationCount");

	LdapValue[] value = ServerUtils.getLdapValue(delegationCount);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_DELEGATION_COUNT, value);
    }

    @Override
    public int getRenewCount(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_RENEW_COUNT);

	return value != null ? ServerUtils.getIntValue(value) : IConfigStore.DEFAULT_RENEW_COUNT;
    }

    @Override
    public void setRenewCount(String tenantName, int renewCount) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNonNegativeNumber(renewCount, "renewCount");

	LdapValue[] value = ServerUtils.getLdapValue(renewCount);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_RENEW_COUNT, value);
    }

    @Override
    public long getMaximumBearerTokenLifetime(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_MAX_BEARTOKEN_LIFETIME);

	return value != null ? ServerUtils.getNativeLongValue(value) : DEFAULT_MAX_BEARER_LIFETIME;
    }

    @Override
    public void setMaximumBearerTokenLifetime(String tenantName, long maxLifetime) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNonNegativeNumber(maxLifetime, "maxLifetime");

	LdapValue[] value = ServerUtils.getLdapValue(maxLifetime);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_MAX_BEARTOKEN_LIFETIME, value);
    }

    @Override
    public long getMaximumHoKTokenLifetime(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_MAX_HOKTOKEN_LIFETIME);

	return value != null ? ServerUtils.getNativeLongValue(value) : DEFAULT_MAX_HOK_LIFETIME;
    }

    @Override
    public void setMaximumHoKTokenLifetime(String tenantName, long maxLifetime) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNonNegativeNumber(maxLifetime, "maxLifetime");

	LdapValue[] value = ServerUtils.getLdapValue(maxLifetime);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_MAX_HOKTOKEN_LIFETIME, value);
    }

    @Override
    public long getMaximumBearerRefreshTokenLifetime(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName,
		TenantAttributesLdapObject.PROPERTY_MAX_BEARER_REFRESH_TOKEN_LIFETIME);

	return value != null ? ServerUtils.getNativeLongValue(value) : DEFAULT_MAX_BEARER_REFRESH_TOKEN_LIFETIME;
    }

    @Override
    public void setMaximumBearerRefreshTokenLifetime(String tenantName, long maxLifetime) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNonNegativeNumber(maxLifetime, "maxLifetime");

	LdapValue[] value = ServerUtils.getLdapValue(maxLifetime);

	this.setTenantProperty(tenantName, TenantAttributesLdapObject.PROPERTY_MAX_BEARER_REFRESH_TOKEN_LIFETIME, value);
    }

    @Override
    public long getMaximumHoKRefreshTokenLifetime(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName,
		TenantAttributesLdapObject.PROPERTY_MAX_HOK_REFRESH_TOKEN_LIFETIME);

	return value != null ? ServerUtils.getNativeLongValue(value) : DEFAULT_MAX_HOK_REFRESH_TOKEN_LIFETIME;
    }

    @Override
    public void setMaximumHoKRefreshTokenLifetime(String tenantName, long maxLifetime) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNonNegativeNumber(maxLifetime, "maxLifetime");

	LdapValue[] value = ServerUtils.getLdapValue(maxLifetime);

	this.setTenantProperty(tenantName, TenantAttributesLdapObject.PROPERTY_MAX_HOK_REFRESH_TOKEN_LIFETIME, value);
    }

    @Override
    public PasswordExpiration getPasswordExpirationConfiguration(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	PasswordExpiration pwdExpiration = null;
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantRootDn = this.ensureTenantExists(connection, tenantName);

	    TenantPasswordExpirationLdapObject passwordSettings = TenantPasswordExpirationLdapObject.getInstance();
	    TenantPasswordExpiration obj = passwordSettings.retrieveObject(connection, tenantRootDn,
		    LdapScope.SCOPE_BASE, null);
	    if (obj != null) {
		pwdExpiration = obj.getPasswordExpiration();
	    }
	}

	if (pwdExpiration == null) {
	    throw new RuntimeException(String.format("Tenant %s doesn't have key Configurations in Idm.", tenantName));
	}

	return pwdExpiration;
    }

    @Override
    public void updatePasswordExpirationConfiguration(String tenantName, PasswordExpiration config) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(config, "config");
	ValidateUtil.validateNotEmpty(config.getEmailFrom(), "config.getEmailFrom()");
	ValidateUtil.validateNotEmpty(config.getEmailSubject(), "config.getEmailSubject()");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantRootDn = this.ensureTenantExists(connection, tenantName);

	    TenantPasswordExpiration tenantsPasswordExpiration = new TenantPasswordExpiration(tenantName, config);
	    TenantPasswordExpirationLdapObject passwordSettings = TenantPasswordExpirationLdapObject.getInstance();
	    passwordSettings.updateObject(connection, tenantRootDn, LdapScope.SCOPE_BASE, null,
		    tenantsPasswordExpiration);
	}
    }

    @Override
    public Collection<Attribute> getTenantAttributes(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    return retrieveAttributes(connection, tenantsRootDn);
	}
    }

    @Override
    public void setTenantAttributes(String tenantName, Collection<Attribute> attributes) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    saveAttributes(connection, tenantsRootDn, attributes);
	}
    }

    @Override
    public void addCertificateForSystemTenant(String tenantName, Certificate cert, CertificateType certType)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(cert, "idmCert");
	ValidateUtil.validateNotEmpty(CertificateUtil.generateFingerprint((X509Certificate) cert),
		"generateFingerprint for cert");
	ValidateUtil.validateNotNull(cert, "idmCert");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.getSystemTenantCredsDn();

	    addCertificateForTenant(tenantsRootDn, connection, cert, certType);
	}
    }

    @Override
    public void addCertificateForNonSystemTenant(String tenantName, Certificate cert, CertificateType certType)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(cert, "idmCert");
	ValidateUtil.validateNotEmpty(CertificateUtil.generateFingerprint((X509Certificate) cert),
		"generateFingerprint for cert");
	ValidateUtil.validateNotNull(cert, "idmCert");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);
	    addCertificateForTenant(tenantsRootDn, connection, cert, certType);
	}
    }

    private void addCertificateForTenant(String tenantRootDn, ILdapConnectionEx connection, Certificate cert,
	    CertificateType certType) throws Exception {
	IdmCertificate idmCert = new IdmCertificate((X509Certificate) cert, certType);

	try {
	    String idmCertificatesContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantRootDn,
		    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDM_CERTIFICATES, true);

	    IdmCertificateLdapObject idmCertObject = IdmCertificateLdapObject.getInstance();
	    idmCertObject.createObject(connection, idmCertObject.getDnFromObject(idmCertificatesContainerDn, idmCert),
		    idmCert);
	} catch (AlreadyExistsLdapException e) {
	    throw new DuplicateCertificateException(String.format("addCertificateForTenant failed - "
		    + "Certificate already exists in %s", tenantRootDn));
	}
    }

    @Override
    public Collection<Certificate> getAllCertificatesForSystemTenant(String tenantName, CertificateType certType)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateCertType(certType, "certificateType");
	Collection<Certificate> allTrustedCertificates = new ArrayList<Certificate>();

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String lduContainerDn = this.getSystemTenantLduDn();
	    ContainerLdapObject lduContainer = ContainerLdapObject.getInstance();
	    Set<String> tenantCredsRootDns = lduContainer.lookupObjects(connection, lduContainerDn,
		    LdapScope.SCOPE_SUBTREE, null, OBJECT_CLASS_CISLDU);
	    if (tenantCredsRootDns != null && tenantCredsRootDns.size() != 0) {
		for (String tenantCredsRootDn : tenantCredsRootDns) {
		    Collection<Certificate> certsInLdu = getAllCertificatesForTenant(tenantName, tenantCredsRootDn,
			    connection, certType);

		    if (certsInLdu != null && certsInLdu.size() != 0) {
			// Add the current list of chains
			for (Certificate cert : certsInLdu) {
			    allTrustedCertificates.add(cert);
			}
		    }
		}
	    }

	    return allTrustedCertificates;
	}
    }

    @Override
    public Collection<Certificate> getAllCertificatesForNonSystemTenant(String tenantName, CertificateType certType)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateCertType(certType, "certificateType");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    return getAllCertificatesForTenant(tenantName, tenantsRootDn, connection, certType);
	}
    }

    private Collection<Certificate> getAllCertificatesForTenant(String tenantName, String tenantRootDn,
	    ILdapConnectionEx connection, CertificateType certType) throws Exception {
	Collection<Certificate> certSet = new ArrayList<Certificate>();
	Collection<Certificate> allCertsInContainer = new ArrayList<Certificate>();
	Collection<Certificate> allStsCerts = Collections.emptySet();

	String certTypeFilter = String.format("(%s=%s)", IdmCertificateLdapObject.PROPERTY_CERT_TYPE, // attribute
												      // name
		LdapFilterString.encode(certType.toString()));
	Collection<IdmCertificate> certs = retrieveObjectsCollection(connection, tenantRootDn,
		ContainerLdapObject.CONTAINER_IDM_CERTIFICATES, certTypeFilter, IdmCertificateLdapObject.getInstance(),
		null);

	if (null != certs && certs.size() > 0) {
	    for (IdmCertificate cert : certs) {
		allCertsInContainer.add(cert.getCertificate());
	    }
	    certSet.addAll(allCertsInContainer);
	}

	if (certType == CertificateType.STS_TRUST_CERT) {
	    allStsCerts = getAllStsCertificatesForTenant(tenantName, tenantRootDn, connection);
	    if (allStsCerts != null && !allStsCerts.isEmpty())
		certSet.addAll(allStsCerts);
	}

	return certSet;
    }

    @Override
    public Collection<Certificate> getTrustedCertificatesForTenant(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    return getTrustedCertificatesForTenant(tenantName, tenantsRootDn, connection);
	}
    }

    /* retrieve all the trusted root certificates */
    private Collection<Certificate> getTrustedCertificatesForTenant(String tenantName, String tenantRootDn,
	    ILdapConnectionEx connection) throws Exception {
	List<Certificate> certSet = new ArrayList<Certificate>();

	Collection<List<Certificate>> trustedCertChains = getTenantCertificatesForTenant(tenantName, tenantRootDn,
		connection);

	if (trustedCertChains != null && !trustedCertChains.isEmpty()) {
	    for (List<Certificate> certChain : trustedCertChains) {
		if (certChain != null && !certChain.isEmpty()) {
		    certSet.add(getRootCert(certChain));
		}
	    }
	}

	return certSet;
    }

    @Override
    public Collection<Certificate> getStsIssuersCertificatesForTenant(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    return getStsIssuersCertificatesForTenant(tenantName, tenantsRootDn, connection);
	}
    }

    /* retrieve all the trusted leaf certificates */
    private Collection<Certificate> getStsIssuersCertificatesForTenant(String tenantName, String tenantRootDn,
	    ILdapConnectionEx connection) throws Exception {
	List<Certificate> certSet = new ArrayList<Certificate>();

	Collection<List<Certificate>> trustedCertChains = getTenantCertificatesForTenant(tenantName, tenantRootDn,
		connection);

	if (trustedCertChains != null && !trustedCertChains.isEmpty()) {
	    for (List<Certificate> certChain : trustedCertChains) {
		if (certChain != null && !certChain.isEmpty()) {
		    certSet.add(getLeafCert(certChain));
		}
	    }
	}

	return certSet;
    }

    /* retrieve all sts certificates */
    private Collection<Certificate> getAllStsCertificatesForTenant(String tenantName, String tenantRootDn,
	    ILdapConnectionEx connection) throws Exception {
	List<Certificate> certSet = new ArrayList<Certificate>();

	Collection<List<Certificate>> trustedCertChains = getTenantCertificatesForTenant(tenantName, tenantRootDn,
		connection);

	if (trustedCertChains != null && !trustedCertChains.isEmpty()) {
	    for (List<Certificate> certChain : trustedCertChains) {
		if (certChain != null && !certChain.isEmpty()) {
		    for (Certificate cert : certChain)
			certSet.add(cert);
		}
	    }
	}

	return certSet;
    }

    private Certificate getRootCert(List<Certificate> certChain) {
	return certChain.get(certChain.size() - 1);
    }

    private Certificate getLeafCert(List<Certificate> certChain) {
	return certChain.get(0);
    }

    private boolean isIssuedByCert(List<Certificate> certChain, X509Certificate certToCheck) {
	if (certChain == null || certChain.isEmpty())
	    return false;

	for (Certificate cert : certChain) {
	    X509Certificate x509Cert = (X509Certificate) cert;
	    if (x509Cert.getSubjectDN().getName().compareToIgnoreCase(certToCheck.getIssuerDN().getName()) == 0) {
		return true;
	    }
	}

	return false;
    }

    @Override
    public void deleteCertificateForSystemTenant(String tenantName, String fingerprint, CertificateType certType)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(fingerprint, "fingerprint");
	ValidateUtil.validateCertType(certType, "certificateType");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.getSystemTenantCredsDn();

	    deleteCertificateForTenant(tenantName, tenantsRootDn, connection, fingerprint, certType);
	}
    }

    @Override
    public void deleteCertificateForNonSystemTenant(String tenantName, String fingerprint, CertificateType certType)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(fingerprint, "fingerprint");
	ValidateUtil.validateCertType(certType, "certificateType");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    deleteCertificateForTenant(tenantName, tenantsRootDn, connection, fingerprint, certType);
	}
    }

    private void deleteCertificateForTenant(String tenantName, String tenantsRootDn, ILdapConnectionEx connection,
	    String fingerprint, CertificateType certType) throws Exception {
	if (certType == CertificateType.STS_TRUST_CERT) {
	    deleteStsCertificateForTenant(tenantName, tenantsRootDn, connection, fingerprint);
	} else if (certType == CertificateType.LDAP_TRUSTED_CERT) {
	    // for LDAP_TYPE only need delete the certificate object itself
	    deleteSingleCertForTenant(tenantName, tenantsRootDn, connection, fingerprint,
		    CertificateType.LDAP_TRUSTED_CERT, false);
	}
    }

    // Only delete the certificate itself
    private void deleteSingleCertForTenant(String tenantName, String tenantsRootDn, ILdapConnectionEx connection,
	    String fingerprint, CertificateType certType, boolean isCertExistInCertChains)
	    throws NoSuchCertificateException {
	String idmCertificatesContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDM_CERTIFICATES, false);

	if (ServerUtils.isNullOrEmpty(idmCertificatesContainerDn) == true) {
	    if (!isCertExistInCertChains) {
		throw new NoSuchCertificateException(String.format(
			"deleteCertificate failed - NO such certificate of type %s" + "found in tenant %s",
			certType.toString(), tenantName));
	    } else {
		return;
	    }
	}

	IdmCertificateLdapObject idmCertObject = IdmCertificateLdapObject.getInstance();
	String certTypeFilter = String.format("(%s=%s)", IdmCertificateLdapObject.PROPERTY_CERT_TYPE, // attribute
												      // name
		LdapFilterString.encode(certType.toString()));
	String idmCertificateDn = idmCertObject.lookupObject(connection, idmCertificatesContainerDn,
		LdapScope.SCOPE_ONE_LEVEL, fingerprint, certTypeFilter);

	if (ServerUtils.isNullOrEmpty(idmCertificateDn) == true) {
	    if (!isCertExistInCertChains) {
		throw new NoSuchCertificateException(String.format(
			"deleteCertificate failed - NO such certificate of type %s" + "found in tenant %s",
			certType.toString(), tenantName));
	    }
	} else {
	    idmCertObject.deleteObject(connection, idmCertificateDn);
	}
    }

    private void deleteStsCertificateForTenant(String tenantName, String tenantsRootDn, ILdapConnectionEx connection,
	    String fingerprint) throws Exception {
	String tenantsCertChainRootDn = tenantsRootDn;

	// Trusted Cert chains are always stored under Services Dn.So need to
	// make sure
	// it always get Certchains from there for both system and non-system
	// tenants.
	if (tenantsRootDn.equalsIgnoreCase(this.getSystemTenantCredsDn())) {
	    tenantsCertChainRootDn = this.ensureTenantExists(connection, tenantName);
	}
	// (1) Check whether this certificate belongs to an active signer
	// certChain
	// (root or leaf of the signing certificate chain)
	boolean isCertExistInCertChains = false;
	List<Certificate> activeCerts = this.getTenantCertificateForTenant(tenantName, tenantsCertChainRootDn,
		connection);
	if (activeCerts != null && !activeCerts.isEmpty()) {
	    IdmCertificate idmRootCert = new IdmCertificate((X509Certificate) getRootCert(activeCerts),
		    CertificateType.STS_TRUST_CERT);
	    if (idmRootCert.getFingerprint().equals(fingerprint)) {
		throw new CertificateInUseException(String.format(
			"deleteCertificate failed - certificate is the root of"
				+ "the active signer Identity in tenant %s", tenantName), idmRootCert.getCertificate());
	    } else {
		IdmCertificate idmLeafCert = new IdmCertificate((X509Certificate) getLeafCert(activeCerts),
			CertificateType.STS_TRUST_CERT);
		if (idmLeafCert.getFingerprint().equals(fingerprint)) {
		    throw new CertificateInUseException(String.format(
			    "deleteCertificate failed - certificate is the root of"
				    + "the active signer Identity in tenant %s", tenantName),
			    idmLeafCert.getCertificate());
		}
	    }
	}

	// (2) Walk through all the trusted certificateChains
	// to delete the ones whose root is this particular certificate
	Collection<List<Certificate>> certChains = this.getTenantCertificatesForTenant(tenantName,
		tenantsCertChainRootDn, connection);
	List<Certificate> candidateRootCerts = new ArrayList<Certificate>();
	for (List<Certificate> certChain : certChains) {
	    if (certChain == null || certChain.isEmpty())
		continue;

	    X509Certificate rootCert = (X509Certificate) getRootCert(certChain);
	    if (CertificateUtil.generateFingerprint(rootCert).equals(fingerprint)) {
		isCertExistInCertChains = true;
		for (int i = 0; i < certChain.size() - 1; i++) {
		    // save certs (but not the root itseft)
		    // as potential candidateRootCerts for step (3) to consume
		    candidateRootCerts.add(certChain.get(i));
		}
		// remove the trustedCertificateChain and possible signer
		// Identity if there is one
		// since its root will be deleted.
		deleteTenantTrustedCertificateChainForTenant(tenantName, tenantsCertChainRootDn, connection, certChain);
		deleteTenantCredentialsForTenant(tenantName, tenantsCertChainRootDn, connection, certChain);
	    } else {
		X509Certificate leafCert = (X509Certificate) getLeafCert(certChain);
		if (CertificateUtil.generateFingerprint(leafCert).equals(fingerprint)) {
		    isCertExistInCertChains = true;
		    // remove the trustedCertificateChain and possible signer
		    // Identity if there is one
		    // since its leaf will be deleted.
		    deleteTenantTrustedCertificateChainForTenant(tenantName, tenantsCertChainRootDn, connection,
			    certChain);
		    deleteTenantCredentialsForTenant(tenantName, tenantsCertChainRootDn, connection, certChain);
		}
	    }
	}

	// (3) Walk through the rest of trusted root STS certificates
	// to retrieve any certificate that is issued by this particular
	// certificate
	// those certificate should be deleted since its issuer has been removed
	Collection<Certificate> trustedRootCerts = this.getTrustedCertificatesForTenant(tenantName,
		tenantsCertChainRootDn, connection);
	for (Certificate cert : trustedRootCerts) {
	    // Check to see whether cert is issued by 'certificate'
	    // (make sure 'cert' is not rootCertToDelete itself)

	    if (cert != null && !CertificateUtil.generateFingerprint((X509Certificate) cert).equals(fingerprint)
		    && isIssuedByCert(candidateRootCerts, (X509Certificate) cert)) {
		deleteCertificateForTenant(tenantName, tenantsRootDn, connection,
			CertificateUtil.generateFingerprint((X509Certificate) cert), CertificateType.STS_TRUST_CERT);
	    }
	}

	// (4) remove the certificate itself if it also lives under
	// 'IdmCertificates' container
	deleteSingleCertForTenant(tenantName, tenantsRootDn, connection, fingerprint, CertificateType.STS_TRUST_CERT,
		isCertExistInCertChains);
    }

    @Override
    public void addRelyingParty(String tenantName, RelyingParty rp) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(rp, "rp");
	ValidateUtil.validateNotEmpty(rp.getName(), "rp.getName()");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    ContainerLdapObject relyingPartiesContainer = ContainerLdapObject.getInstance();
	    String relyingPartiesContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    relyingPartiesContainer, ContainerLdapObject.CONTAINER_RELYING_PARTIES, true);

	    RelyingPartyLdapObject relyingPartyConfig = RelyingPartyLdapObject.getInstance();

	    String relyingPartyDn = relyingPartyConfig.getDnFromObject(relyingPartiesContainerDn, rp);
	    relyingPartyConfig.createObject(connection, relyingPartyDn, rp);

	    DirectoryConfigStore.saveRelyingPartyAssertionConsumerServices(connection, relyingPartyDn,
		    rp.getAssertionConsumerServices());
	    DirectoryConfigStore.saveRelyingPartyAttributeConsumerServices(connection, relyingPartyDn,
		    rp.getAttributeConsumerServices());
	    DirectoryConfigStore.saveRelyingPartySingleLogoutServices(connection, relyingPartyDn,
		    rp.getSingleLogoutServices());
	    DirectoryConfigStore.saveRelyingPartySignatureAlgorithms(connection, relyingPartyDn,
		    rp.getSignatureAlgorithms());
	}
    }

    @Override
    public void deleteRelyingParty(String tenantName, String rpName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(rpName, "rpName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    ContainerLdapObject relyingPartiesContainer = ContainerLdapObject.getInstance();
	    String relyingPartiesContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    relyingPartiesContainer, ContainerLdapObject.CONTAINER_RELYING_PARTIES, false);

	    if (ServerUtils.isNullOrEmpty(relyingPartiesContainerDn) == false) {
		RelyingPartyLdapObject relyingPartyConfig = RelyingPartyLdapObject.getInstance();

		String relyingPartyDn = relyingPartyConfig.lookupObject(connection, relyingPartiesContainerDn,
			LdapScope.SCOPE_ONE_LEVEL, rpName);

		if (ServerUtils.isNullOrEmpty(relyingPartyDn) == false) {
		    relyingPartyConfig.deleteObject(connection, relyingPartyDn);
		} else {
		    throw new NoSuchRelyingPartyException(String.format("The relying party %s does not exist", rpName));
		}
	    }
	}
    }

    @Override
    public RelyingParty getRelyingParty(String tenantName, String rpName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(rpName, "rpName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    RelyingParty relyingParty = null;

	    ContainerLdapObject relyingPartiesContainer = ContainerLdapObject.getInstance();
	    String relyingPartiesContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    relyingPartiesContainer, ContainerLdapObject.CONTAINER_RELYING_PARTIES, false);

	    if (ServerUtils.isNullOrEmpty(relyingPartiesContainerDn) == false) {
		RelyingPartyLdapObject relyingPartyConfig = RelyingPartyLdapObject.getInstance();

		relyingParty = relyingPartyConfig.retrieveObject(connection, relyingPartiesContainerDn,
			LdapScope.SCOPE_ONE_LEVEL, rpName);

		if (relyingParty != null) {
		    String relyingPartyDn = relyingPartyConfig.getDnFromObject(relyingPartiesContainerDn, relyingParty);
		    relyingParty.setAssertionConsumerServices(DirectoryConfigStore
			    .retrieveRelyingPartyAssertionConsumerServices(connection, relyingPartyDn));
		    relyingParty.setAttributeConsumerServices(DirectoryConfigStore
			    .retrieveRelyingPartyAttributeConsumerServices(connection, relyingPartyDn));
		    relyingParty.setSingleLogoutServices(DirectoryConfigStore.retrieveRelyingPartySingleLogoutServices(
			    connection, relyingPartyDn));
		    relyingParty.setSignatureAlgorithms(DirectoryConfigStore.retrieveRelyingPartySignatureAlgorithms(
			    connection, relyingPartyDn));
		}
	    }

	    return relyingParty;
	}
    }

    @Override
    public RelyingParty getRelyingPartyByUrl(String tenantName, String url) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(url, "url");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    RelyingPartyLdapObject relyingPartyConfig = RelyingPartyLdapObject.getInstance();
	    String urlFilter = relyingPartyConfig.getInSetSearchFilter(RelyingPartyLdapObject.PROPERTY_URL,
		    new String[] { url });

	    RelyingParty relyingParty = null;

	    Collection<RelyingParty> relyingParties = DirectoryConfigStore.getRelyingParties(this, connection,
		    tenantName, urlFilter);

	    if (relyingParties.size() > 0) {
		// maybe we should throw when found > 1?
		// [current code returns 1st instance, so keeping in sync]
		relyingParty = relyingParties.iterator().next();
	    }

	    return relyingParty;
	}
    }

    @Override
    public void setRelyingParty(String tenantName, RelyingParty rp) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(rp, "rp");
	ValidateUtil.validateNotEmpty(rp.getName(), "rp.getName()");

	// future - consider reconciling properties instead of delete/add
	this.deleteRelyingParty(tenantName, rp.getName());
	this.addRelyingParty(tenantName, rp);
    }

    @Override
    public Collection<RelyingParty> getRelyingParties(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    return DirectoryConfigStore.getRelyingParties(this, connection, tenantName, null);
	}
    }

    @Override
    public void addOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(oidcClient, "oidcClient");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    ContainerLdapObject oidcClientsContainer = ContainerLdapObject.getInstance();
	    String oidcClientsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    oidcClientsContainer, ContainerLdapObject.CONTAINER_OIDC_CLIENTS, true);

	    OIDCClientLdapObject oidcClientLdapObject = OIDCClientLdapObject.getInstance();

	    String oidcClientDn = oidcClientLdapObject.getDnFromObject(oidcClientsContainerDn, oidcClient);
	    oidcClientLdapObject.createObject(connection, oidcClientDn, oidcClient);
	}
    }

    @Override
    public void deleteOIDCClient(String tenantName, String clientID) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(clientID, "clientID");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    boolean found = false;

	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    ContainerLdapObject oidcClientsContainer = ContainerLdapObject.getInstance();
	    String oidcClientsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    oidcClientsContainer, ContainerLdapObject.CONTAINER_OIDC_CLIENTS, false);

	    if (ServerUtils.isNullOrEmpty(oidcClientsContainerDn) == false) {
		OIDCClientLdapObject oidcClientLdapObject = OIDCClientLdapObject.getInstance();

		String oidcClientDn = oidcClientLdapObject.lookupObject(connection, oidcClientsContainerDn,
			LdapScope.SCOPE_ONE_LEVEL, clientID);

		if (ServerUtils.isNullOrEmpty(oidcClientDn) == false) {
		    found = true;
		    oidcClientLdapObject.deleteObject(connection, oidcClientDn);
		}
	    }

	    if (!found) {
		throw new NoSuchOIDCClientException(String.format("The OIDC client %s does not exist on tenant %s",
			clientID, tenantName));
	    }
	}
    }

    @Override
    public OIDCClient getOIDCClient(String tenantName, String clientID) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(clientID, "clientID");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    OIDCClient oidcClient = null;

	    ContainerLdapObject oidcClientsContainer = ContainerLdapObject.getInstance();
	    String oidcClientsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    oidcClientsContainer, ContainerLdapObject.CONTAINER_OIDC_CLIENTS, false);

	    if (ServerUtils.isNullOrEmpty(oidcClientsContainerDn) == false) {
		OIDCClientLdapObject oidcClientLdapObject = OIDCClientLdapObject.getInstance();

		oidcClient = oidcClientLdapObject.retrieveObject(connection, oidcClientsContainerDn,
			LdapScope.SCOPE_ONE_LEVEL, clientID);
	    }

	    // throw NoSuchOIDCClientException if no client is found
	    if (oidcClient == null) {
		throw new NoSuchOIDCClientException(String.format("The OIDC client %s does not exist on tenant %s",
			clientID, tenantName));
	    }

	    return oidcClient;
	}
    }

    @Override
    public void setOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception {
        ValidateUtil.validateNotEmpty(tenantName, "tenantName");
        ValidateUtil.validateNotNull(oidcClient, "oidcClient");

        try (PooledLdapConnection pooledConnection = borrowConnection()) {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            boolean found = false;

            String tenantsRootDn = this.ensureTenantExists(connection, tenantName);
            ContainerLdapObject oidcClientsContainer = ContainerLdapObject.getInstance();
            String oidcClientsContainerDn = DirectoryConfigStore.ensureObjectExists(
                    connection,
                    tenantsRootDn,
                    oidcClientsContainer,
                    ContainerLdapObject.CONTAINER_OIDC_CLIENTS,
                    false /* createIfNotExists */);

            if (!ServerUtils.isNullOrEmpty(oidcClientsContainerDn)) {
                OIDCClientLdapObject oidcClientLdapObject = OIDCClientLdapObject.getInstance();
                String oidcClientDn = oidcClientLdapObject.lookupObject(
                        connection,
                        oidcClientsContainerDn,
                        LdapScope.SCOPE_ONE_LEVEL,
                        oidcClient.getClientId());

                if (!ServerUtils.isNullOrEmpty(oidcClientDn)) {
                    found = true;
                    oidcClientLdapObject.updateObject(connection, oidcClientDn, oidcClient);
                }
            }

            if (!found) {
                throw new NoSuchOIDCClientException(
                        String.format("The OIDC client %s does not exist on tenant %s", oidcClient.getClientId(), tenantName));
            }
        }
    }

    @Override
    public Collection<OIDCClient> getOIDCClients(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    return DirectoryConfigStore.getOIDCClients(this, pooledConnection.getConnection(), tenantName, null);
	}
    }

    @Override
    public void addResourceServer(String tenantName, ResourceServer resourceServer) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(resourceServer, "resourceServer");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);
	    ContainerLdapObject resourceServersContainer = ContainerLdapObject.getInstance();
	    String resourceServersContainerDn = DirectoryConfigStore
		    .ensureObjectExists(connection, tenantsRootDn, resourceServersContainer,
			    ContainerLdapObject.CONTAINER_RESOURCE_SERVERS, true /* createIfNotExists */);
	    ResourceServerLdapObject resourceServerLdapObject = ResourceServerLdapObject.getInstance();
	    String resourceServerDn = resourceServerLdapObject.getDnFromObject(resourceServersContainerDn,
		    resourceServer);
	    resourceServerLdapObject.createObject(connection, resourceServerDn, resourceServer);
	}
    }

    @Override
    public void deleteResourceServer(String tenantName, String resourceServerName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(resourceServerName, "resourceServerName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    boolean found = false;

	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);
	    ContainerLdapObject resourceServersContainer = ContainerLdapObject.getInstance();
	    String resourceServersContainerDn = DirectoryConfigStore
		    .ensureObjectExists(connection, tenantsRootDn, resourceServersContainer,
			    ContainerLdapObject.CONTAINER_RESOURCE_SERVERS, false /* createIfNotExists */);

	    if (!ServerUtils.isNullOrEmpty(resourceServersContainerDn)) {
		ResourceServerLdapObject resourceServerLdapObject = ResourceServerLdapObject.getInstance();
		String resourceServerDn = resourceServerLdapObject.lookupObject(connection, resourceServersContainerDn,
			LdapScope.SCOPE_ONE_LEVEL, resourceServerName);
		if (!ServerUtils.isNullOrEmpty(resourceServerDn)) {
		    found = true;
		    resourceServerLdapObject.deleteObject(connection, resourceServerDn);
		}
	    }

	    if (!found) {
		throw new NoSuchResourceServerException(String.format(
			"The resource server %s does not exist on tenant %s", resourceServerName, tenantName));
	    }
	}
    }

    @Override
    public ResourceServer getResourceServer(String tenantName, String resourceServerName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(resourceServerName, "resourceServerName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();

	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);
	    ContainerLdapObject resourceServersContainer = ContainerLdapObject.getInstance();
	    String resourceServersContainerDn = DirectoryConfigStore
		    .ensureObjectExists(connection, tenantsRootDn, resourceServersContainer,
			    ContainerLdapObject.CONTAINER_RESOURCE_SERVERS, false /* createIfNotExists */);

	    ResourceServer resourceServer = null;
	    if (!ServerUtils.isNullOrEmpty(resourceServersContainerDn)) {
		ResourceServerLdapObject resourceServerLdapObject = ResourceServerLdapObject.getInstance();
		resourceServer = resourceServerLdapObject.retrieveObject(connection, resourceServersContainerDn,
			LdapScope.SCOPE_ONE_LEVEL, resourceServerName);
	    }

	    if (resourceServer == null) {
		throw new NoSuchResourceServerException(String.format(
			"The resource server %s does not exist on tenant %s", resourceServerName, tenantName));
	    }

	    return resourceServer;
	}
    }

    @Override
    public void setResourceServer(String tenantName, ResourceServer resourceServer) throws Exception {
        ValidateUtil.validateNotEmpty(tenantName, "tenantName");
        ValidateUtil.validateNotNull(resourceServer, "resourceServer");

        try (PooledLdapConnection pooledConnection = borrowConnection()) {
            ILdapConnectionEx connection = pooledConnection.getConnection();

            boolean found = false;

            String tenantsRootDn = this.ensureTenantExists(connection, tenantName);
            ContainerLdapObject resourceServersContainer = ContainerLdapObject.getInstance();
            String resourceServersContainerDn = DirectoryConfigStore.ensureObjectExists(
                    connection,
                    tenantsRootDn,
                    resourceServersContainer,
                    ContainerLdapObject.CONTAINER_RESOURCE_SERVERS,
                    false /* createIfNotExists */);
            if (!ServerUtils.isNullOrEmpty(resourceServersContainerDn)) {
                ResourceServerLdapObject resourceServerLdapObject = ResourceServerLdapObject.getInstance();
                String resourceServerDn = resourceServerLdapObject.lookupObject(
                        connection,
                        resourceServersContainerDn,
                        LdapScope.SCOPE_ONE_LEVEL,
                        resourceServer.getName());
                if (!ServerUtils.isNullOrEmpty(resourceServerDn)) {
                    found = true;
                    resourceServerLdapObject.updateObject(connection, resourceServerDn, resourceServer);
                }
            }

            if (!found) {
                throw new NoSuchResourceServerException(
                        String.format("The resource server %s does not exist on tenant %s", resourceServer.getName(), tenantName));
            }
        }
    }

    @Override
    public Collection<ResourceServer> getResourceServers(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();

	    Collection<ResourceServer> resourceServers;
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);
	    resourceServers = DirectoryConfigStore.retrieveObjectsCollection(connection, tenantsRootDn,
		    ContainerLdapObject.CONTAINER_RESOURCE_SERVERS, null /* additionalFilter */,
		    ResourceServerLdapObject.getInstance(), null /* callback */);
	    return resourceServers;
	}
    }

    // Helper function for getOIDCClients method
    private static Collection<OIDCClient> getOIDCClients(DirectoryConfigStore directoryConfigStore,
	    ILdapConnectionEx connection, String tenantName, String propertyFilter) {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	String tenantsRootDn = directoryConfigStore.ensureTenantExists(connection, tenantName);

	Collection<OIDCClient> oidcClients = retrieveObjectsCollection(connection, tenantsRootDn,
		ContainerLdapObject.CONTAINER_OIDC_CLIENTS, propertyFilter, OIDCClientLdapObject.getInstance(), null);

	return oidcClients;
    }

    @Override
    public void addProvider(String tenantName, IIdentityStoreData idsData) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(idsData, "idsData");
	ValidateUtil.validateNotEmpty(idsData.getName(), "idsData.getName()");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    Tenant tenant = this.getTenant(connection, tenantName);

	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    if (idsData.getDomainType() == DomainType.LOCAL_OS_DOMAIN) {
		// as of now the decision is that we only allow this for default
		// tenant.
		// we still have an issue of what if default tenant is re-set,
		// but this
		// needs to be covered by the general decision about the default
		// tenant...
		String defaultTenant = this.getDefaultTenant();
		if (tenantName.equalsIgnoreCase(defaultTenant) == false) {
		    throw new RuntimeException("LocalOS domain type is only supported for a default tenant.");
		}

		Collection<IIdentityStoreData> providers = getProviders(tenantName,
			EnumSet.of(DomainType.LOCAL_OS_DOMAIN), true);

		if (providers.size() > 0) {
		    throw new LocalISRegistrationException(String.format(
			    "Identity Provider '%s' with domain type LocalOSDomain, already exists.", providers
				    .iterator().next().getName()));
		}
		idsData = getLocalOsDomainIdentityStoreData(idsData);
	    } else if (idsData.getDomainType() == DomainType.SYSTEM_DOMAIN) {
		throw new RuntimeException("IdentityProvider for system domain cannot be added/modified.");
	    } else if (idsData.getDomainType() == DomainType.EXTERNAL_DOMAIN) {
		ValidateUtil.validateNotNull(idsData.getExtendedIdentityStoreData(),
			"idsData.getExtendedIdentityStoreData()");
		ValidateUtil.validateNotNull(idsData.getExtendedIdentityStoreData().getConnectionStrings(),
			"idsData.getExtendedIdentityStoreData().getConnectionStrings()");

		// No connection string is stored for native AD
		if (idsData.getExtendedIdentityStoreData().getProviderType() != IdentityStoreType.IDENTITY_STORE_TYPE_ACTIVE_DIRECTORY
			&& idsData.getExtendedIdentityStoreData().getConnectionStrings().size() < 1) {
		    throw new IllegalArgumentException("There must be at least 1 connection string provided.");
		}

		for (String str : idsData.getExtendedIdentityStoreData().getConnectionStrings()) {
		    ValidateUtil.validateNotEmpty(str, "connectionString");
		}

		if (false == validateExternalProviderNameConflict(tenantName, idsData)) {
		    throw new DuplicateProviderException(idsData.getName(), idsData.getExtendedIdentityStoreData()
			    .getAlias());
		}
	    } else {
		throw new IllegalArgumentException(String.format("Unsupported domain type '%s'.", idsData
			.getDomainType().toString()));
	    }

	    enCryptPassword(tenant._tenantKey, idsData.getExtendedIdentityStoreData());
	    DirectoryConfigStore.SaveIdentityProviderConfig(this, connection, tenantsRootDn, idsData);
	}
    }

    @Override
    public void registerExternalIdpConfig(String tenantName, IDPConfig idpConfig) throws Exception {
	// validation has been done on IIdentityManager
	PooledLdapConnection pooledConnection = borrowConnection();
	ILdapConnectionEx connection = pooledConnection.getConnection();
	String tenantRootDn = lookupTenantsRootDn(connection, tenantName);

	if (tenantRootDn == null) {
	    throw new NoSuchTenantException(String.format("Tenant [%s] not found", tenantName));
	}

	String externalIDPConfigsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantRootDn,
		ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_EXTERNAL_IDP_CONFIGS, true);

	IDPConfigLdapObject ldapObj = IDPConfigLdapObject.getInstance();
	String ldapObjDN = ldapObj.getDnFromObject(externalIDPConfigsContainerDn, idpConfig);

	boolean ldapObjectExists = true;
	try {
	    ldapObj.lookupObject(connection, ldapObjDN, LdapScope.SCOPE_BASE, null);
	} catch (NoSuchObjectLdapException nsole) {
	    ldapObjectExists = false;
	}
	try {
	    if (ldapObjectExists) {// delete existing object first before
				   // setting the new configuration
		ldapObj.deleteObject(connection, ldapObjDN);
	    }
	    ldapObj.createObject(connection, ldapObjDN, idpConfig);

	    // populate all the subordinate containers
	    saveObjectsCollection(connection, ldapObjDN, ContainerLdapObject.CONTAINER_EXTERNAL_IDP_SSO_SERVICES,
		    SingleSignOnServiceLdapObject.getInstance(), idpConfig.getSsoServices(), null);

	    saveObjectsCollection(connection, ldapObjDN, ContainerLdapObject.CONTAINER_EXTERNAL_IDP_SLO_SERVICES,
		    SingleLogoutServiceLdapObject.getInstance(), idpConfig.getSloServices(), null);

	    TenantTrustedCertificateChain certChain = new TenantTrustedCertificateChain("TrustedCertChain",
		    new ArrayList<Certificate>(idpConfig.getSigningCertificateChain()));
	    // For now, only one trusted certificate chain in this container.
	    Collection<TenantTrustedCertificateChain> chains = Arrays.asList(certChain);
	    saveObjectsCollection(connection, ldapObjDN, ContainerLdapObject.CONTAINER_EXTERNAL_IDP_CERTIFICATE_CHAINS,
		    TenantTrustedCertChainLdapObject.getInstance(), chains, null);

	    Collection<AttributeMapping> attributeMappings = new ArrayList<AttributeMapping>();

	    if (idpConfig.getSubjectFormatMappings() != null) {
		int i = 0;
		for (AttributeConfig config : idpConfig.getSubjectFormatMappings()) {
		    attributeMappings.add(new AttributeMapping(config.getTokenSubjectFormat(), config
			    .getStoreAttribute(), i++));
		}
	    }

	    saveObjectsCollection(connection, ldapObjDN,
		    ContainerLdapObject.CONTAINER_EXTERNAL_IDP_SUBJECT_FORMAT_MAPPINGS,
		    AttributeMappingLdapObject.getInstance(), attributeMappings, null);

	    attributeMappings = new ArrayList<AttributeMapping>();

	    if (idpConfig.getTokenClaimGroupMappings() != null) {
		int i = 0;
		for (TokenClaimAttribute tokenClaimAttr : idpConfig.getTokenClaimGroupMappings().keySet()) {
		    for (String group : idpConfig.getTokenClaimGroupMappings().get(tokenClaimAttr)) {
			attributeMappings.add(new AttributeMapping(tokenClaimAttr.getClaimName()
				+ CLAIM_GROUP_DELIMITER + tokenClaimAttr.getClaimValue(), group, i++));
		    }
		}
	    }

	    saveObjectsCollection(connection, ldapObjDN,
		    ContainerLdapObject.CONTAINER_EXTERNAL_IDP_GROUP_ATTRIBUTE_MAPPINGS,
		    AttributeMappingLdapObject.getInstance(), attributeMappings, null);
	} finally {
	    pooledConnection.close();
	}
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.idm.server.config.IConfigStore#removeIdpConfig(java
     * .lang.String, java.lang.String)
     */
    @Override
    public void removeExternalIdpConfig(String tenantName, String configEntityId) throws Exception {
	// validation has been done on IIdentityManager
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantRootDn = lookupTenantsRootDn(connection, tenantName);
	    if (tenantRootDn == null) {
		throw new NoSuchTenantException(String.format("Tenant [%s] not found", tenantName));
	    }

	    String externalIDPConfigsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantRootDn,
		    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_EXTERNAL_IDP_CONFIGS, true);

	    IDPConfigLdapObject ldapObj = IDPConfigLdapObject.getInstance();
	    String ldapObjDN = String.format("cn=%s, %s", configEntityId, externalIDPConfigsContainerDn);

	    // check whether the configuration already exists.
	    ldapObjDN = ldapObj.lookupObject(connection, ldapObjDN, LdapScope.SCOPE_BASE, null);

	    ldapObj.deleteObject(connection, ldapObjDN);
	} catch (NoSuchObjectLdapException nsole) {
	    throw new NoSuchExternalIdpConfigException(String.format(
		    "External IDP config [%s] does not exist for tenant [%s]", configEntityId, tenantName), nsole);
	}
    }

    @Override
    public Collection<IDPConfig> getExternalIDPConfigs(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	Collection<IDPConfig> idpConfigs = null;
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantConfigDN = lookupTenantsRootDn(connection, tenantName);
	    if (StringUtils.isEmpty(tenantConfigDN)) {
		throw new NoSuchTenantException(String.format("tenant [%s] doesnot exist", tenantName));
	    }

	    idpConfigs = retrieveObjectsCollection(connection, tenantConfigDN,
		    ContainerLdapObject.CONTAINER_EXTERNAL_IDP_CONFIGS, null, IDPConfigLdapObject.getInstance(),
		    new IObjectProcessedCallback<IDPConfig>() {
			@Override
			public void processObjectSaved(ILdapConnectionEx connection, String idpConfigDN,
				IDPConfig idpConfig) {
			    idpConfig.setSsoServices(retrieveObjectsCollection(connection, idpConfigDN,
				    ContainerLdapObject.CONTAINER_EXTERNAL_IDP_SSO_SERVICES,
				    SingleSignOnServiceLdapObject.getInstance(), null));

			    idpConfig.setSloServices(retrieveObjectsCollection(connection, idpConfigDN,
				    ContainerLdapObject.CONTAINER_EXTERNAL_IDP_SLO_SERVICES,
				    SingleLogoutServiceLdapObject.getInstance(), null));

			    try {
				idpConfig.setSigningCertificateChain(retrieveExternalIDPConfigSigningCertificates(
					connection, idpConfigDN));
			    } catch (IDMException ie) {
				String message = "Signing certifiate chain retrieved from directory store"
					+ "is invalid, most likely certs are out of required order"
					+ "(user first, root cast last)";
				logger.error(message);
				throw new IllegalStateException(message);
			    }

			    AttributeConfig[] subjectFormatMappings = retrieveSubjectFormatMap(connection, idpConfigDN);
			    Map<TokenClaimAttribute, List<String>> tokenClaimGroupMappings = retrieveClaimGroupMap(
				    connection, idpConfigDN);
			    // imported null attributeConfig[] will be exported
			    // as empty attributeConfig[]
			    assert (subjectFormatMappings != null);
			    assert (tokenClaimGroupMappings != null);
			    idpConfig.setSubjectFormatMappings(subjectFormatMappings);
			    idpConfig.setTokenClaimGroupMappings(tokenClaimGroupMappings);
			}
		    });

	}
	return idpConfigs;
    }

    private List<X509Certificate> retrieveExternalIDPConfigSigningCertificates(ILdapConnectionEx connection,
	    String idpConfigDN) {
	Collection<TenantTrustedCertificateChain> chains = retrieveObjectsCollection(connection, idpConfigDN,
		ContainerLdapObject.CONTAINER_EXTERNAL_IDP_CERTIFICATE_CHAINS,
		TenantTrustedCertChainLdapObject.getInstance(), null);
	assert (chains.size() == 1);
	List<X509Certificate> x509Certs = new ArrayList<X509Certificate>();
	for (Certificate cert : chains.iterator().next().getCertificateChain()) {
	    x509Certs.add((X509Certificate) cert);
	}
	return x509Certs;
    }

    private AttributeConfig[] retrieveSubjectFormatMap(ILdapConnectionEx connection, String idpConfigDN) {
	Collection<AttributeMapping> mappings = retrieveObjectsCollection(connection, idpConfigDN,
		ContainerLdapObject.CONTAINER_EXTERNAL_IDP_SUBJECT_FORMAT_MAPPINGS,
		AttributeMappingLdapObject.getInstance(), null);

	List<AttributeConfig> configs = new ArrayList<AttributeConfig>();
	for (AttributeMapping mapping : mappings) {
	    configs.add(new AttributeConfig(mapping.getAttributeFrom(), mapping.getAttributeTo()));
	}

	return configs.toArray(new AttributeConfig[configs.size()]);
    }

    private Map<TokenClaimAttribute, List<String>> retrieveClaimGroupMap(ILdapConnectionEx connection,
	    String idpConfigDN) {
	Collection<AttributeMapping> attMappings = retrieveObjectsCollection(connection, idpConfigDN,
		ContainerLdapObject.CONTAINER_EXTERNAL_IDP_GROUP_ATTRIBUTE_MAPPINGS,
		AttributeMappingLdapObject.getInstance(), null);

	Map<TokenClaimAttribute, List<String>> tokenClaimMappings = new HashMap<>();
	for (AttributeMapping mapping : attMappings) {
	    String claimStr = mapping.getAttributeFrom();
	    int pos = claimStr.indexOf("#");
	    if (pos < 0) {
		throw new IllegalStateException("Retrieved an invaid claim from internal identity store: " + claimStr);
	    }
	    String claimName = claimStr.substring(0, pos);
	    String claimValue = claimStr.substring(pos + 1);
	    TokenClaimAttribute tokenClaim = new TokenClaimAttribute(claimName, claimValue);
	    if (tokenClaimMappings.containsKey(tokenClaim)) {
		tokenClaimMappings.get(tokenClaim).add(mapping.getAttributeTo());
	    } else {
		List<String> groupList = new ArrayList<>();
		groupList.add(mapping.getAttributeTo());
		tokenClaimMappings.put(tokenClaim, groupList);
	    }
	}

	return tokenClaimMappings;
    }

    private boolean validateExternalProviderNameConflict(String tenantName, IIdentityStoreData idsDataToAdd)
	    throws Exception {
	Collection<IIdentityStoreData> providers = getProviders(tenantName, EnumSet.allOf(DomainType.class), true);

	// check with existing name/alias from existing providers
	for (IIdentityStoreData provider : providers) {
	    String providerName = provider.getName();
	    assert (null != providerName);
	    if (isDomainNameSame(idsDataToAdd, providerName) || isDomainAliasSame(idsDataToAdd, providerName)) {
		return false;
	    }

	    String providerAlias = provider.getExtendedIdentityStoreData().getAlias();
	    if (providerAlias != null
		    && ((isDomainNameSame(idsDataToAdd, providerAlias)) || (isDomainAliasSame(idsDataToAdd,
			    providerAlias)))) {
		return false;
	    }
	}

	return true;
    }

    private boolean isDomainNameSame(IIdentityStoreData storeDataToAdd, String existingDomainName) {
	assert (null != existingDomainName);
	// specified domain name needs to fully match the real domain name
	return StringUtils.equalsIgnoreCase(storeDataToAdd.getName(), existingDomainName);
    }

    private boolean isDomainAliasSame(IIdentityStoreData storeDataToAdd, String existingDomainAlias) {
	assert (null != existingDomainAlias);
	return StringUtils.equalsIgnoreCase(storeDataToAdd.getExtendedIdentityStoreData().getAlias(),
		existingDomainAlias);
    }

    @Override
    public void deleteProvider(String tenantName, String providerName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(providerName, "providerName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, false);

	    if (ServerUtils.isNullOrEmpty(identityProvidersContainerDn) == false) {
		String aliasName = null;

		IdentityProviderLdapObject identityProviderConfigObject = IdentityProviderLdapObject.getInstance();
		IdentityProviderAliasLdapObject identityProviderAliasConfigObject = IdentityProviderAliasLdapObject
			.getInstance();

		String identityProviderDn = identityProviderConfigObject.lookupObject(connection,
			identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, providerName);

		String aliasDn = null;

		if (ServerUtils.isNullOrEmpty(identityProviderDn) == true) {
		    IdentityProviderAlias obj = identityProviderAliasConfigObject.retrieveObject(connection,
			    identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, providerName);

		    if (obj != null) {
			aliasName = providerName;
			aliasDn = identityProviderAliasConfigObject.getDnFromObject(identityProvidersContainerDn, obj);
			identityProviderDn = obj.getProviderDn();
			providerName = ServerUtils.getStringValue(identityProviderConfigObject.getObjectProperty(
				connection, identityProviderDn, LdapScope.SCOPE_BASE, null,
				IdentityProviderLdapObject.PROPERTY_NAME));
			if (ServerUtils.isNullOrEmpty(providerName) == true) // not
									     // found
			{
			    identityProviderDn = null;
			}
		    }
		} else {
		    aliasName = ServerUtils.getStringValue(identityProviderConfigObject.getObjectProperty(connection,
			    identityProviderDn, LdapScope.SCOPE_BASE, null, IdentityProviderLdapObject.PROPERTY_ALIAS));

		    if (ServerUtils.isNullOrEmpty(aliasName) == false) {
			aliasDn = identityProviderAliasConfigObject.lookupObject(connection,
				identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, aliasName);
		    }
		}

		if (ServerUtils.isNullOrEmpty(aliasDn) == false) {
		    identityProviderAliasConfigObject.deleteObject(connection, aliasDn);
		}

		if (ServerUtils.isNullOrEmpty(identityProviderDn) == false) {
		    identityProviderConfigObject.deleteObject(connection, identityProviderDn);
		}

		// update default providers if needed
		String defaultProvider = ServerUtils.getStringValue(DirectoryConfigStore.getTenantProperty(connection,
			tenantsRootDn, TenantLdapObject.PROPERTY_DEFAULT_PROVIDER));

		if (ServerUtils.isNullOrEmpty(defaultProvider) == false) {
		    if ((defaultProvider.equalsIgnoreCase(providerName) == true)
			    || (defaultProvider.equalsIgnoreCase(aliasName) == true)) {
			// remove default provider setting
			DirectoryConfigStore.setTenantProperty(connection, tenantsRootDn,
				TenantLdapObject.PROPERTY_DEFAULT_PROVIDER, null);
		    }
		}
	    }
	}
    }

    @Override
    public IIdentityStoreData getProvider(String tenantName, String providerName, boolean getInternalInfo)
	    throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(providerName, "providerName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    Tenant tenant = this.getTenant(connection, tenantName);

	    String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, false);

	    IIdentityStoreData provider = null;

	    if (ServerUtils.isNullOrEmpty(identityProvidersContainerDn) == false) {
		IdentityProviderLdapObject identityProviderConfigObject = IdentityProviderLdapObject.getInstance();
		IdentityProviderAliasLdapObject identityProviderAliasConfigObject = IdentityProviderAliasLdapObject
			.getInstance();

		String identityProviderDn = identityProviderConfigObject.lookupObject(connection,
			identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, providerName);

		if (ServerUtils.isNullOrEmpty(identityProviderDn) == true) {
		    IdentityProviderAlias identityProviderAlias = identityProviderAliasConfigObject.retrieveObject(
			    connection, identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, providerName);

		    if (identityProviderAlias != null) {
			identityProviderDn = identityProviderAlias.getProviderDn();
			providerName = ServerUtils.getStringValue(identityProviderConfigObject.getObjectProperty(
				connection, identityProviderDn, LdapScope.SCOPE_BASE, null,
				IdentityProviderLdapObject.PROPERTY_NAME));
		    }

		    if (ServerUtils.isNullOrEmpty(providerName) == true) // not
									 // found
		    {
			identityProviderDn = null;
		    }
		}

		if (ServerUtils.isNullOrEmpty(identityProviderDn) == false) {
		    IIdentityStoreData provObj = identityProviderConfigObject.retrieveObject(connection,
			    identityProviderDn, LdapScope.SCOPE_BASE, null);
		    CryptoAESE cryptoAES = new CryptoAESE(tenant._tenantKey);
		    deCryptPassword(cryptoAES, provObj.getExtendedIdentityStoreData());
		    retrieveIdentityProviderAttributesMap(connection, identityProviderDn, provObj, getInternalInfo);
		    retrieveIdentityProviderSchemaMapping(connection, identityProviderDn, provObj, getInternalInfo);
		    provider = getIIdenttyStoreDataToReturn(provObj, getInternalInfo);
		}
	    }

	    return provider;
	}
    }

    @Override
    public void setProvider(String tenantName, IIdentityStoreData idsData) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(idsData, "idsData");
	ValidateUtil.validateNotEmpty(idsData.getName(), "idsData.getName()");

	switch (idsData.getDomainType()) {
	case EXTERNAL_DOMAIN:

	    // future - perform an update instead of add/remove
	    Collection<String> defaultProviders = getDefaultProviders(tenantName);
	    this.deleteProvider(tenantName, idsData.getName());
	    this.addProvider(tenantName, idsData);
	    if ((defaultProviders.size() == 1) && (defaultProviders.contains(idsData.getName()))) {
		setDefaultProviders(tenantName, defaultProviders);
	    }

	    break;

	case SYSTEM_DOMAIN:

	    if (!(idsData instanceof ServerIdentityStoreData)) {
		throw new IllegalArgumentException(String.format("Error : expected parameter %s. Found %s",
			ServerIdentityStoreData.class, idsData.getClass()));
	    }

	    updateSystemDomain(tenantName, (ServerIdentityStoreData) idsData);

	    break;

	default:

	    throw new IllegalArgumentException(String.format("Provider '%s' with Domain type '%s' cannot be updated.",
		    idsData.getName(), idsData.getDomainType().toString()));
	}
    }

    @Override
    public Collection<IIdentityStoreData> getProviders(String tenantName, EnumSet<DomainType> domainTypes,
	    boolean getInternalInfo) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(domainTypes, "domainTypes");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    Tenant tenant = this.getTenant(connection, tenantName);

	    Collection<IIdentityStoreData> providers = new ArrayList<IIdentityStoreData>();

	    if (domainTypes.size() > 0) {
		IdentityProviderLdapObject identityProviderConfig = IdentityProviderLdapObject.getInstance();

		String domainTypeFilter = null;
		if (domainTypes.size() < DomainType.values().length) {
		    String[] domainTypesStr = null;
		    domainTypesStr = new String[domainTypes.size()];
		    int i = 0;
		    for (DomainType type : domainTypes) {
			domainTypesStr[i] = type.toString();
			i++;
		    }

		    domainTypeFilter = identityProviderConfig.getInSetSearchFilter(
			    IdentityProviderLdapObject.PROPERTY_DOMAIN_TYPE, domainTypesStr);
		}

		final boolean retrieveInternalInfo = getInternalInfo;
		Collection<IIdentityStoreData> providersList = retrieveObjectsCollection(connection, tenantsRootDn,
			ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, domainTypeFilter, identityProviderConfig,
			new IObjectProcessedCallback<IIdentityStoreData>() {
			    @Override
			    public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
				    IIdentityStoreData object) {
				retrieveIdentityProviderAttributesMap(connection, objectDn, object,
					retrieveInternalInfo);
				retrieveIdentityProviderSchemaMapping(connection, objectDn, object,
					retrieveInternalInfo);
			    }
			});

		if ((providersList != null) && (providersList.size() > 0)) {
		    if (getInternalInfo == false) {
			// convert to external info
			for (IIdentityStoreData provider : providersList) {
			    providers.add(getIIdenttyStoreDataToReturn(provider, getInternalInfo));
			}
		    } else {
			providers = providersList;
		    }
		}
	    }

	    if (providers != null) {
		CryptoAESE cryptoAES = new CryptoAESE(tenant._tenantKey);
		for (IIdentityStoreData provider : providers) {
		    deCryptPassword(cryptoAES, provider.getExtendedIdentityStoreData());
		}
	    }
	    return providers;
	}
    }

    @Override
    public Collection<String> getDefaultProviders(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	Collection<String> result = new ArrayList<String>();

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_DEFAULT_PROVIDER);
	String defaultProvider = ServerUtils.getStringValue(value);

	if (ServerUtils.isNullOrEmpty(defaultProvider) == false) {
	    result.add(defaultProvider);
	}

	return result;
    }

    @Override
    public void setDefaultProviders(String tenantName, Collection<String> defaultProviders) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	if ((defaultProviders != null) && (defaultProviders.size() > 1)) {
	    throw new DuplicateProviderException("Only a single default provider is supported.");
	}

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String defaultProvider = null;
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, false);

	    IdentityProviderLdapObject identityProviderConfigObject = IdentityProviderLdapObject.getInstance();
	    IdentityProviderAliasLdapObject identityProviderAliasConfigObject = IdentityProviderAliasLdapObject
		    .getInstance();

	    if ((defaultProviders != null) && (defaultProviders.size() > 0)) {
		defaultProvider = defaultProviders.iterator().next();

		if (ServerUtils.isNullOrEmpty(defaultProvider) == false) {
		    String providerDn = null;
		    if (ServerUtils.isNullOrEmpty(identityProvidersContainerDn) == false) {
			providerDn = identityProviderConfigObject.lookupObject(connection,
				identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, defaultProvider);

			if (ServerUtils.isNullOrEmpty(providerDn)) {
			    providerDn = identityProviderAliasConfigObject.lookupObject(connection,
				    identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, defaultProvider);
			}
		    }

		    if (ServerUtils.isNullOrEmpty(providerDn)) {
			throw new NoSuchIdpException(String.format("Identity Provider '%s' does not exist.",
				defaultProvider));
		    }
		}
	    }

	    LdapValue[] value = ServerUtils.getLdapValue(defaultProvider);

	    DirectoryConfigStore.setTenantProperty(connection, tenantsRootDn,
		    TenantLdapObject.PROPERTY_DEFAULT_PROVIDER, value);
	}
    }

    @Override
    public void setEntityID(String tenantName, String entityID) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = ServerUtils.getLdapValue(entityID);

	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_ENTITY_ID, value);
    }

    @Override
    public String getEntityID(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	LdapValue[] value = this.getTenantProperty(tenantName, TenantLdapObject.PROPERTY_ENTITY_ID);

	return ServerUtils.getStringValue(value);
    }

    @Override
    public boolean registerUpnSuffixForDomain(String tenantName, String domainName, String upnSuffix) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(domainName, "domainName");

	boolean added = false;
	PooledLdapConnection pooledConnection = borrowConnection();
	ILdapConnectionEx connection = pooledConnection.getConnection();

	String tenantRootDn = lookupTenantsRootDn(connection, tenantName);
	assert (tenantRootDn != null);// tenant existence is validated by caller

	String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantRootDn,
		ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, false);
	if (identityProvidersContainerDn == null) {// Idp not found -- if
						   // container does not exist
	    String msg = String.format("IdentityProviders container does not exist for tenant [%s]",
		    identityProvidersContainerDn);
	    logger.error(msg);
	    throw new NoSuchIdpException(msg);
	}
	String identityProviderDn = IdentityProviderLdapObject.getInstance().lookupObject(connection,
		identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, domainName);
	if (identityProviderDn == null) {
	    String msg = String.format("Idp [%s] does not exist under [%s]", domainName, identityProvidersContainerDn);
	    logger.error(msg);
	    throw new NoSuchIdpException(msg);
	}

	try {
	    LdapMod mod[] = { new LdapMod(LdapModOperation.ADD, IdentityProviderLdapObject.PROPERTY_UPN_SUFFIXES,
		    LdapValue.fromString(ValidateUtil.getCanonicalUpnSuffix(upnSuffix))) };
	    connection.modifyObject(identityProviderDn, mod);
	    added = true;
	} catch (AttributeOrValueExistsLdapException e) {
	    logger.info(String.format("upnSuffix [%s] is already registered for provider [%s]", upnSuffix,
		    identityProviderDn));
	    added = false;
	} finally {
	    pooledConnection.close();
	}
	return added;
    }

    @Override
    public boolean unregisterUpnSuffixForDomain(String tenantName, String domainName, String upnSuffix)
	    throws Exception {

	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(domainName, "domainName");

	boolean removed = false;
	PooledLdapConnection pooledConnection = borrowConnection();
	ILdapConnectionEx connection = pooledConnection.getConnection();

	String tenantRootDn = lookupTenantsRootDn(connection, tenantName);
	assert (tenantRootDn != null);// tenant existence is validated by caller

	String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantRootDn,
		ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, false);
	if (identityProvidersContainerDn == null) {// Idp not found -- if
						   // container does not exist
	    String msg = String.format("IdentityProviders container does not exist for tenant [%s]",
		    identityProvidersContainerDn);
	    logger.error(msg);
	    throw new NoSuchIdpException(msg);
	}
	String identityProviderDn = IdentityProviderLdapObject.getInstance().lookupObject(connection,
		identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, domainName);
	if (identityProviderDn == null) {
	    String msg = String.format("Idp [%s] does not exist under [%s]", domainName, identityProvidersContainerDn);
	    logger.error(msg);
	    throw new NoSuchIdpException(msg);
	}

	try {
	    LdapMod[] mod = { new LdapMod(LdapModOperation.DELETE, IdentityProviderLdapObject.PROPERTY_UPN_SUFFIXES,
		    ValidateUtil.getCanonicalUpnSuffix(upnSuffix)) };
	    connection.modifyObject(identityProviderDn, mod);
	    removed = true;
	} catch (NoSuchAttributeLdapException e) {
	    logger.info(String.format("upnSuffix [%s] is not found from provider [%s]", upnSuffix, identityProviderDn));
	    removed = false;
	} finally {
	    pooledConnection.close();
	}
	return removed;
    }


    @Override
    public ClientCertPolicy getClientCertPolicy(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantRootDn = this.ensureTenantExists(connection, tenantName);
	    // get ClientCertPolicy
	    String filter = String.format("(cn=%s)", ClientCertPolicyLdapObject.ClientCertificatePolicyDefaultName);
	    Collection<TenantClientCertPolicy> certPolicies = retrieveObjectsCollection(connection, tenantRootDn,
		    ContainerLdapObject.CONTAINER_CLIENT_CERT_POLICIES, filter,
		    ClientCertPolicyLdapObject.getInstance(), new IObjectProcessedCallback<TenantClientCertPolicy>() {
			@Override
            public void processObjectSaved(
                    ILdapConnectionEx connection, String certPolicyDn,
                    TenantClientCertPolicy policy) {
                policy.getClientCertPolicy().set_siteOCSPMap(retrieveSiteOCSPMap(connection, certPolicyDn));
            }
		    });
	    ClientCertPolicy certPolicy = null;
	    if (certPolicies != null && certPolicies.size() > 0) {
		// retrieve the certPolicy
		TenantClientCertPolicy tenantClientCertPolicy = certPolicies.iterator().next();
		certPolicy = tenantClientCertPolicy.getClientCertPolicy();

		// retrieve the trustedCAs
		String clientCertPolicyContainerDn = String.format("cn=%s,%s",
			ContainerLdapObject.CONTAINER_CLIENT_CERT_POLICIES, tenantRootDn);
		Collection<TenantTrustedCertificateChain> chains = retrieveObjectsCollection(
			connection,
			ClientCertPolicyLdapObject.getInstance().getDnFromObject(clientCertPolicyContainerDn,
				tenantClientCertPolicy),
			ClientCertPolicyLdapObject.CONTAINER_CLIENT_CERT_TRUSTED_CA_CERTIFICATES,
			TenantTrustedCertChainLdapObject.getInstance(), null);
		for (TenantTrustedCertificateChain chain : chains) {
		    if (chain.getTenantTrustedCertChainName().equals(
			    ClientCertPolicyLdapObject.CLIENT_CERT_TRUSTED_CA_CERTIFICATES_DEFAULT_NAME)
			    && chain.getCertificateChain().size() > 0) {
			// set trusted CAs to certPolicy
			certPolicy.setTrustedCAs(chain.getCertificateChain().toArray(
				new Certificate[chain.getCertificateChain().size()]));
		    }
		}
	    } else {
		certPolicy = new ClientCertPolicy();
	    }
	    return certPolicy;
	}
    }

    private static HashMap<String, AlternativeOCSPList> retrieveSiteOCSPMap(ILdapConnectionEx connection, String certPolicyDn) {
        ValidateUtil.validateNotEmpty(certPolicyDn, "certPolicyDn");

        AlternativeOCSPListLdapObject ocspListLdapObj = AlternativeOCSPListLdapObject.getInstance();

        Collection<AlternativeOCSPList> altOCSPListColection = retrieveObjectsCollection(connection, certPolicyDn,
                ContainerLdapObject.CONTAINER_CLIENT_CERT_OCSP_LISTS,
                ocspListLdapObj,
                new IObjectProcessedCallback<AlternativeOCSPList>() {
                    @Override
                    public void processObjectSaved(
                            ILdapConnectionEx connection, String altOcspListDn,
                            AlternativeOCSPList altOCSPList) {
                        //retrieve OCSP responders for AlternativeOCSPList object.
                        altOCSPList.set_ocspList(retrieveAlternativeOCSPsForSite(connection, altOcspListDn));
                    }

                });

        //Create HashMap to be returned.
        HashMap<String, AlternativeOCSPList> retMap = new HashMap<String, AlternativeOCSPList>();
        if (altOCSPListColection!=null && !altOCSPListColection.isEmpty()) {
            for (AlternativeOCSPList altOcspList : altOCSPListColection) {
                retMap.put(altOcspList.get_siteID(), altOcspList);
            }
        }
        return retMap;
    }
    /**
     * retrieve ordered AlternativeOCSP list
     * @param connection
     * @param altOcspListDn
     * @return
     */
    protected static List<AlternativeOCSP> retrieveAlternativeOCSPsForSite(ILdapConnectionEx connection, String altOcspListDn) {
        Collection<TenantAlternativeOCSP> tOcspCollection = retrieveObjectsCollection(connection, altOcspListDn,
                ContainerLdapObject.CONTAINER_OCSPs,
                TenantAlternativeOCSPLdapObject.getInstance(), null);

        ArrayList<AlternativeOCSP> ocspArray = new ArrayList<AlternativeOCSP> (tOcspCollection.size() );
        for(TenantAlternativeOCSP tAlternativeOCSP : tOcspCollection) {
            ocspArray.add(tAlternativeOCSP.getAlternativeOCSP());
        }

        //verify the order by cn
        int index=0;
        for(TenantAlternativeOCSP tAlternativeOCSP : tOcspCollection) {
            int cn = Integer.parseInt(tAlternativeOCSP.getCn());
            if (cn < tOcspCollection.size() ) {
                if (cn != index) {
                    ocspArray.set(cn,tAlternativeOCSP.getAlternativeOCSP());
                }
            } else {
                throw new IllegalArgumentException("Unexpected TenantAlternativeOCSP object has cn value out of index range of the collection.");
            }
            index++;
        }

        return ocspArray;
    }

    @Override
    public void setClientCertPolicy(String tenantName, ClientCertPolicy policy)
            throws Exception {
        ValidateUtil.validateNotEmpty(tenantName, "tenantName");

        try (PooledLdapConnection pooledConnection = borrowConnection()) {
            ILdapConnectionEx connection = pooledConnection.getConnection();
            String tenantRootDn = this.ensureTenantExists(connection, tenantName);
            String authnPoliciesContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantRootDn,
                    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_CLIENT_CERT_POLICIES, true);
            String filter = String.format("(cn=%s)", ClientCertPolicyLdapObject.ClientCertificatePolicyDefaultName);
            Collection<TenantClientCertPolicy> certPolicies = retrieveObjectsCollection(connection, tenantRootDn,
                    ContainerLdapObject.CONTAINER_CLIENT_CERT_POLICIES, filter,
                    ClientCertPolicyLdapObject.getInstance(), new IObjectProcessedCallback<TenantClientCertPolicy>() {
                        @Override
                        public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
                                TenantClientCertPolicy certPolicy) {
                        }
                    });
            // Set ClientCertificatePolicy
            String certPolicyDn = ClientCertPolicyLdapObject.getInstance().getDnFromCn(authnPoliciesContainerDn,
                    ClientCertPolicyLdapObject.ClientCertificatePolicyDefaultName);
            if (policy != null) {
                if (certPolicies != null && certPolicies.size() > 0) {
                    ClientCertPolicyLdapObject.getInstance().updateObject(connection, certPolicyDn,
                            new TenantClientCertPolicy(tenantName, policy));
                } else {
                    ClientCertPolicyLdapObject.getInstance().createObject(connection, certPolicyDn,
                            new TenantClientCertPolicy(tenantName, policy));
                }
                // Ensure trustedCA container exist
                String trustedCAContainerDn = DirectoryConfigStore.ensureObjectExists(connection, certPolicyDn,
                        ContainerLdapObject.getInstance(),
                        ClientCertPolicyLdapObject.CONTAINER_CLIENT_CERT_TRUSTED_CA_CERTIFICATES, true);
                // Retrieve the existing trustedCAs
                Set<String> trustedCADns = TenantTrustedCertChainLdapObject.getInstance().lookupObjects(connection,
                        trustedCAContainerDn, LdapScope.SCOPE_ONE_LEVEL,
                        ClientCertPolicyLdapObject.CLIENT_CERT_TRUSTED_CA_CERTIFICATES_DEFAULT_NAME,
                        TenantTrustedCertChainLdapObject.OBJECT_CLASS);
                String trustedCAsDn = String.format("cn=%s,%s",
                        ClientCertPolicyLdapObject.CLIENT_CERT_TRUSTED_CA_CERTIFICATES_DEFAULT_NAME,
                        trustedCAContainerDn);
                // Set the trustedCAs
                if (policy.getTrustedCAs() != null && policy.getTrustedCAs().length > 0) {
                    Certificate[] certs = policy.getTrustedCAs();
                    TenantTrustedCertificateChain trustedCAs = new TenantTrustedCertificateChain(
                            ClientCertPolicyLdapObject.CLIENT_CERT_TRUSTED_CA_CERTIFICATES_DEFAULT_NAME,
                            new ArrayList<Certificate>(Arrays.asList(certs)));

                    if (trustedCADns != null && trustedCADns.size() > 0) {
                        // Update existing trusted CAs
                        TenantTrustedCertChainLdapObject.getInstance().updateObject(connection, trustedCAsDn,
                                trustedCAs);
                    } else {
                        // Create trusted CAs
                        TenantTrustedCertChainLdapObject.getInstance().createObject(connection, trustedCAsDn,
                                trustedCAs);
                    }
                } else if (trustedCADns != null && trustedCADns.size() > 0) {
                    // Delete current trusted CAs
                    TenantTrustedCertChainLdapObject.getInstance().deleteObject(connection, trustedCAsDn);
                }

                // set OCSP list
                setOCSPLists(certPolicyDn, connection,
                        policy.get_siteOCSPList());

            } else if (certPolicies != null && certPolicies.size() > 0) {
                // Delete the current certPolicy
                ClientCertPolicyLdapObject.getInstance().deleteObject(connection, certPolicyDn);
            }
        }
    }


    /**
     * Save a hashmap <site,AlternativeOCSPList>
     * if the provided instance is not empty, this function does update and add entries.
     *
     * Note: we ignore null siteOcspMap (i.e. do not clean the corresponding directory objects) for API backward compatibility.
     *
     * @param certPolicyDn
     * @param connection
     * @param siteOcspMap
     */
    private void setOCSPLists(String certPolicyDn, ILdapConnectionEx connection, HashMap<String, AlternativeOCSPList> siteOcspMap) {
        ValidateUtil.validateNotEmpty(certPolicyDn, "certPolicyDn");

        ContainerLdapObject instContainer =
                ContainerLdapObject.getInstance();
        String ocspListContainerDn =
                DirectoryConfigStore
                .ensureObjectExists(
                        connection,
                        certPolicyDn,
                        instContainer,
                        ContainerLdapObject.CONTAINER_CLIENT_CERT_OCSP_LISTS,
                        true);

        if (siteOcspMap != null && siteOcspMap.size() > 0) {

            AlternativeOCSPListLdapObject altOcspListLdap =
                    AlternativeOCSPListLdapObject.getInstance();
            Collection<AlternativeOCSPList> existingOcspLists = retrieveObjectsCollection(
                    connection, certPolicyDn,
                    ContainerLdapObject.CONTAINER_CLIENT_CERT_OCSP_LISTS, null,
                    altOcspListLdap,
                    new IObjectProcessedCallback<AlternativeOCSPList>() {
                        @Override
                        public void processObjectSaved(
                                ILdapConnectionEx connection, String objectDn,
                                AlternativeOCSPList instInfo) {
                        }
                    });

            if (siteOcspMap.size() != existingOcspLists.size()) {
                AlternativeOCSPListLdapObject.getInstance().deleteObject(
                        connection, ocspListContainerDn);

                //recreate empty DN
                ocspListContainerDn =
                        DirectoryConfigStore
                        .ensureObjectExists(
                                connection,
                                certPolicyDn,
                                instContainer,
                                ContainerLdapObject.CONTAINER_CLIENT_CERT_OCSP_LISTS,
                                true);
                existingOcspLists = null;
            }
            //Note: this does not guarantee remove old site config if site ID is changed. But this is okay. CLI is capable to clean all sites
            for (String siteID : siteOcspMap.keySet()) {
                AlternativeOCSPList ocspList = siteOcspMap.get(siteID);

                updateOCSPList( ocspListContainerDn, connection,existingOcspLists, ocspList);
            }
        } else {
            if (siteOcspMap != null) {
                AlternativeOCSPListLdapObject.getInstance().deleteObject(
                    connection, ocspListContainerDn);
            }
            //else null siteOcspMap. Ignore for API backward compatibility.
        }

    }



    /**
     * Add/update a list of alternative OCSP responders for a site
     * @param ocspListContainerDn
     * @param connection
     * @param ocspLists
     * @param ocspList
     */
    private void updateOCSPList(String ocspListContainerDn, ILdapConnectionEx connection,
            Collection<AlternativeOCSPList> existingOcspLists,
            AlternativeOCSPList ocspList) {

        AlternativeOCSPListLdapObject ocspListObj = AlternativeOCSPListLdapObject.getInstance();

        String listDn = ocspListObj.getDnFromCn(
                ocspListContainerDn,
                ocspList.get_siteID());

        if (existingOcspLists == null || !existingOcspLists.contains(ocspList)) {
              //go ahead update
            if (ocspList != null ) {
                if (isOCSPListSiteExist(existingOcspLists, ocspList.get_siteID()) ) {
                    AlternativeOCSPListLdapObject.getInstance().deleteObject(
                            connection, listDn);
                }
                ocspListObj.createObject(connection, listDn, ocspList);

                //save AlternativeOCSP objects
                ContainerLdapObject ocspContainer =
                        ContainerLdapObject.getInstance();
                String ocspContainerDn =
                        DirectoryConfigStore
                        .ensureObjectExists(
                                connection,
                                listDn,
                                ocspContainer,
                                ContainerLdapObject.CONTAINER_OCSPs,
                                true);

                List<AlternativeOCSP> altOCSPs = ocspList.get_ocspList();

                int ind = 0;
                for (AlternativeOCSP altOCSP : altOCSPs)
                {
                    String indStr = String.valueOf(ind++);
                    String ocspDn = ocspListObj.getDnFromCn(
                            ocspContainerDn,
                             indStr);
                    TenantAlternativeOCSP tenantAltOCSP= new TenantAlternativeOCSP(indStr, altOCSP);
                    addOCSP( ocspDn, connection, tenantAltOCSP);
                }
            }
        }
        // else exact same object exist
    }

    private void addOCSP(String ocspDn, ILdapConnectionEx connection, TenantAlternativeOCSP altOCSP) {
        TenantAlternativeOCSPLdapObject ocspLdapObj = TenantAlternativeOCSPLdapObject.getInstance();
        ocspLdapObj.createObject(connection, ocspDn, altOCSP);

    }

    /**
     * Check if a site was already created for the given site-id.
     * @param existingOcspLists
     * @param siteID
     * @return
     */
    private boolean isOCSPListSiteExist(Collection<AlternativeOCSPList> existingOcspLists, String siteID) {
        boolean retVal = false;

        ValidateUtil.validateNotNull(siteID, "siteID");
        if (existingOcspLists == null || existingOcspLists.isEmpty()) {
            retVal = false;
        } else {
            for (AlternativeOCSPList list : existingOcspLists) {
                if (list.get_siteID().equals(siteID)) {
                    retVal = true;
                }
            }
        }
        return retVal;

    }

    @Override
    public void setAuthnTypes(String tenantName, boolean password, boolean windows, boolean certificate,
	    boolean rsaSecureID) throws Exception {
	// Set AuthnTypes
	HashSet<Integer> authnTypes = new HashSet<Integer>();
	if (password)
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_PASSWORD);
	if (windows)
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_WINDOWS);
	if (certificate)
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_TLS_CERTIFICATE);
	if (rsaSecureID)
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_RSA_SECUREID);

	if (authnTypes.size() == 0) // This is to distinguish the case that
				    // none of the AuthnTypes is set and
				    // the case of migrating from old schema
				    // where AuthnTypes is not there.
	    authnTypes.add(DirectoryConfigStore.FLAG_AUTHN_TYPE_ALLOW_NONE);
	int[] authnTypesArray = ArrayUtils.toPrimitive(authnTypes.toArray(new Integer[authnTypes.size()]));
	this.setTenantProperty(tenantName, TenantLdapObject.PROPERTY_AUTHN_TYPES,
		ServerUtils.getLdapValue(authnTypesArray));
    }

    @Override
    public RSAAgentConfig getRSAAgentConfig(String tenantName) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	RSAAgentConfig rsaConfig = null;
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    Collection<RSAAgentConfig> configs = retrieveObjectsCollection(connection, tenantsRootDn,
		    ContainerLdapObject.CONTAINER_RSA_CONFIGURATIONS, null, RSAAgentConfigLdapObject.getInstance(),
		    new IObjectProcessedCallback<RSAAgentConfig>() {
			@Override
			public void processObjectSaved(ILdapConnectionEx connection, String rsaConfigDn,
				RSAAgentConfig rsaConfig) {
			    rsaConfig.set_instMap(retrieveRSAAMInstancesMap(connection, rsaConfigDn));
			    rsaConfig.set_idsUserIDAttributeMaps(retrieveRSAConfigIdsUserIDAttributeMaps(connection,
				    rsaConfigDn));
			}
		    });
	    if (configs == null || configs.size() == 0) {
		rsaConfig = null;
	    } else {
		rsaConfig = configs.iterator().next();
	    }
	}
	return rsaConfig;
    }

    private static HashMap<String, RSAAMInstanceInfo> retrieveRSAAMInstancesMap(ILdapConnectionEx connection,
	    String rsaAgentConfigDn) {
	ValidateUtil.validateNotEmpty(rsaAgentConfigDn, "rsaAgentConfigDn");

	Collection<RSAAMInstanceInfo> instColection = retrieveObjectsCollection(connection, rsaAgentConfigDn,
		ContainerLdapObject.CONTAINER_RSA_INSTANCES, RSAInstanceLdapObject.getInstance(), null);
	HashMap<String, RSAAMInstanceInfo> retMap = new HashMap<String, RSAAMInstanceInfo>();
	if (instColection != null && !instColection.isEmpty()) {
	    for (RSAAMInstanceInfo inst : instColection) {
		retMap.put(inst.get_siteID(), inst);
	    }
	}
	return retMap;
    }

    private static HashMap<String, String> retrieveRSAConfigIdsUserIDAttributeMaps(ILdapConnectionEx connection,
	    String rsaAgentConfigDn) {
	ValidateUtil.validateNotEmpty(rsaAgentConfigDn, "rsaAgentConfigDn");
	HashMap<String, String> attrMaps = null;

	Collection<AttributeMapping> mapCollection = retrieveObjectsCollection(connection, rsaAgentConfigDn,
		ContainerLdapObject.CONTAINER_RSA_IDS_USERID_ATTRIBUTE_MAPS, AttributeMappingLdapObject.getInstance(),
		null);
	if (mapCollection != null && !mapCollection.isEmpty()) {
	    attrMaps = new HashMap<String, String>();
	}
	for (AttributeMapping pair : mapCollection) {
	    attrMaps.put(pair.getAttributeFrom(), pair.getAttributeTo());
	}
	return attrMaps;
    }

    @Override
    public void setRsaAgentConfig(String tenantName, RSAAgentConfig rsaConfig) throws Exception {

	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	if (rsaConfig == null) {
	    return;
	}

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    ContainerLdapObject rsaConfigsContainer = ContainerLdapObject.getInstance();
	    String rsaConfigsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    rsaConfigsContainer, ContainerLdapObject.CONTAINER_RSA_CONFIGURATIONS, true);

	    Collection<RSAAgentConfig> rsaConfigs = retrieveObjectsCollection(connection, tenantsRootDn,
		    ContainerLdapObject.CONTAINER_RSA_CONFIGURATIONS, null, RSAAgentConfigLdapObject.getInstance(),
		    new IObjectProcessedCallback<RSAAgentConfig>() {
			@Override
			public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
				RSAAgentConfig rsaConfig) {
			}
		    });
	    // Set rsaConfigurations
	    RSAAgentConfigLdapObject rsaConfigLdap = RSAAgentConfigLdapObject.getInstance();

	    String rsaConfigDn = rsaConfigLdap.getDnFromCn(rsaConfigsContainerDn,
		    RSAAgentConfigLdapObject.PROPERTY_DEFAULT_NAME);

	    if (rsaConfig != null) {
		if (rsaConfigs.size() == 1) {
		    RSAAgentConfigLdapObject.getInstance().updateObject(connection, rsaConfigDn, rsaConfig);
		} else {
		    rsaConfigLdap.createObject(connection, rsaConfigDn, rsaConfig);
		}

		setRsaInstances(rsaConfigDn, connection, rsaConfig.get_instMap());
		// First delete old IDSUserIDAttribute map. Then add the new
		// one.
		deleteRSAConfigUserIDAttributeMap(tenantName, connection, rsaConfigDn, rsaConfig);

		DirectoryConfigStore.saveRSAConfigUserIDAttributeMap(connection, rsaConfigDn,
			rsaConfig.get_idsUserIDAttributeMap());
	    }
	}
    }

    private void deleteRSAConfigUserIDAttributeMap(String tenantName, ILdapConnectionEx connection, String rsaConfigDn,
	    RSAAgentConfig config) {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotEmpty(rsaConfigDn, "rsaConfigDn");
	try {
	    AttributeMappingLdapObject attrMapsLdapObject = AttributeMappingLdapObject.getInstance();

	    if (ServerUtils.isNullOrEmpty(rsaConfigDn) == false) {
		String idsUserIDAttributeMapsDn = attrMapsLdapObject.lookupObject(connection, rsaConfigDn,
			LdapScope.SCOPE_ONE_LEVEL, ContainerLdapObject.CONTAINER_RSA_IDS_USERID_ATTRIBUTE_MAPS);

		if (ServerUtils.isNullOrEmpty(idsUserIDAttributeMapsDn) == false) {
		    attrMapsLdapObject.deleteObject(connection, idsUserIDAttributeMapsDn);
		}
	    }
	} catch (Exception e) {
	    logger.error("Unable to delete old RSAAgentIDSAtributeMaps");
	}

    }

    private static void saveRSAConfigUserIDAttributeMap(ILdapConnectionEx connection, String rsaConfigDn,
	    HashMap<String, String> idsUserIDAttributeMap) {
	Collection<AttributeMapping> attributeMappingArray = new ArrayList<AttributeMapping>();

	if (idsUserIDAttributeMap != null) {
	    int i = 0;

	    Set<String> keySet = idsUserIDAttributeMap.keySet();
	    for (String key : keySet) {
		attributeMappingArray.add(new AttributeMapping(key, idsUserIDAttributeMap.get(key), i++));
	    }
	}

	saveObjectsCollection(connection, rsaConfigDn, ContainerLdapObject.CONTAINER_RSA_IDS_USERID_ATTRIBUTE_MAPS,
		AttributeMappingLdapObject.getInstance(), attributeMappingArray, null);

    }

    /**
     * if the provided instance is not empty, this function does update and add
     * entries but not remove existing entries. It clears the container if the
     * updating map is empty.
     *
     *
     * @param rsaConfigDn
     *            non-null
     * @param connection
     * @param instMap
     *            could be null or empty
     */
    private void setRsaInstances(String rsaConfigDn, ILdapConnectionEx connection,
	    HashMap<String, RSAAMInstanceInfo> instMap) {
	ValidateUtil.validateNotEmpty(rsaConfigDn, "rsaConfigDn");

	ContainerLdapObject instContainer = ContainerLdapObject.getInstance();
	String instContainerDn = DirectoryConfigStore.ensureObjectExists(connection, rsaConfigDn, instContainer,
		ContainerLdapObject.CONTAINER_RSA_INSTANCES, true);

	if (instMap != null && instMap.size() > 0) {

	    RSAInstanceLdapObject rsaInstanceLdap = RSAInstanceLdapObject.getInstance();
	    Collection<RSAAMInstanceInfo> instances = retrieveObjectsCollection(connection, rsaConfigDn,
		    ContainerLdapObject.CONTAINER_RSA_INSTANCES, null, rsaInstanceLdap,
		    new IObjectProcessedCallback<RSAAMInstanceInfo>() {
			@Override
			public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
				RSAAMInstanceInfo instInfo) {
			}
		    });

	    for (String siteID : instMap.keySet()) {
		RSAAMInstanceInfo inst = instMap.get(siteID);
		// Set RSAAMInstanceInfo

		addRSAInstanceInfo(instContainerDn, connection, instances, inst);
	    }
	} else {
	    // Delete the container since it is not set in the new data

	    RSAAgentConfigLdapObject.getInstance().deleteObject(connection, instContainerDn);
	}

    }

    @Override
    public void deleteRSAInstanceInfo(String tenantName, String siteID) throws Exception {

	ValidateUtil.validateNotNull(siteID, "siteID");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    ContainerLdapObject instContainer = ContainerLdapObject.getInstance();
	    String rsaConfigsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    instContainer, ContainerLdapObject.CONTAINER_RSA_CONFIGURATIONS, true);

	    String rsaConfigDn = instContainer.getDnFromCn(rsaConfigsContainerDn,
		    RSAAgentConfigLdapObject.PROPERTY_DEFAULT_NAME);

	    String instContainerDn = DirectoryConfigStore.ensureObjectExists(connection, rsaConfigDn, instContainer,
		    ContainerLdapObject.CONTAINER_RSA_INSTANCES, true);
	    // test
	    Set<String> rsaInstanceDns = instContainer.lookupObjects(connection, instContainerDn,
		    LdapScope.SCOPE_ONE_LEVEL, siteID, RSAInstanceLdapObject.OBJECT_CLASS);

	    for (String instDn : rsaInstanceDns) {
		instContainer.deleteObject(connection, instDn);
	    }
	}
    }

    private void addRSAInstanceInfo(String instContainerDn, ILdapConnectionEx connection,
	    Collection<RSAAMInstanceInfo> instances, RSAAMInstanceInfo inst) {

	RSAInstanceLdapObject rsaInstanceLdap = RSAInstanceLdapObject.getInstance();
	String instDn = rsaInstanceLdap.getDnFromCn(instContainerDn, inst.get_siteID());

	if (inst != null) {
	    if (isInstAlreadyCreated(instances, inst.get_siteID())) {
		RSAInstanceLdapObject.getInstance().updateObject(connection, instDn, inst);
	    } else {
		rsaInstanceLdap.createObject(connection, instDn, inst);
	    }
	}

    }

    /**
     * Check if RSAAMInstanceInfo is already defined for a site.
     *
     * @param instances
     *            set of RSAAMInstanceInfo defined for the tenant
     * @param siteID
     *            the siteID to search for
     * @return true if the set contains the given site
     */
    private boolean isInstAlreadyCreated(Collection<RSAAMInstanceInfo> instances, String siteID) {
	boolean retVal = false;

	ValidateUtil.validateNotNull(siteID, "siteID");
	if (instances == null || instances.isEmpty()) {
	    retVal = false;
	} else {
	    for (RSAAMInstanceInfo instInfo : instances) {
		if (instInfo.get_siteID().equals(siteID)) {
		    retVal = true;
		}
	    }
	}
	return retVal;
    }

    @Override
    public void addRSAInstanceInfo(String tenantName, RSAAMInstanceInfo instInfo) throws Exception {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");
	ValidateUtil.validateNotNull(instInfo, "instInfo");

	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.ensureTenantExists(connection, tenantName);

	    ContainerLdapObject ldapContainer = ContainerLdapObject.getInstance();
	    String rsaConfigsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		    ldapContainer, ContainerLdapObject.CONTAINER_RSA_CONFIGURATIONS, true);

	    Collection<RSAAgentConfig> rsaConfigs = retrieveObjectsCollection(connection, tenantsRootDn,
		    ContainerLdapObject.CONTAINER_RSA_CONFIGURATIONS, null, RSAAgentConfigLdapObject.getInstance(),
		    new IObjectProcessedCallback<RSAAgentConfig>() {
			@Override
			public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
				RSAAgentConfig rsaConfig) {
			}
		    });

	    // intialize a default RSAAgentCofig if needed
	    if (rsaConfigs == null || rsaConfigs.isEmpty()) {
		RSAAgentConfig agentConfig = new RSAAgentConfig();
		setRsaAgentConfig(tenantName, agentConfig);
	    }

	    RSAAgentConfigLdapObject rsaConfigLdap = RSAAgentConfigLdapObject.getInstance();

	    String rsaConfigDn = rsaConfigLdap.getDnFromCn(rsaConfigsContainerDn,
		    RSAAgentConfigLdapObject.PROPERTY_DEFAULT_NAME);

	    String instContainerDn = DirectoryConfigStore.ensureObjectExists(connection, rsaConfigDn, ldapContainer,
		    ContainerLdapObject.CONTAINER_RSA_INSTANCES, true);

	    RSAInstanceLdapObject rsaInstanceLdap = RSAInstanceLdapObject.getInstance();
	    Collection<RSAAMInstanceInfo> instances = retrieveObjectsCollection(connection, rsaConfigDn,
		    ContainerLdapObject.CONTAINER_RSA_INSTANCES, null, rsaInstanceLdap,
		    new IObjectProcessedCallback<RSAAMInstanceInfo>() {
			@Override
			public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
				RSAAMInstanceInfo instInfo) {
			}
		    });

	    addRSAInstanceInfo(instContainerDn, connection, instances, instInfo);
	} catch (Exception e) {
	    logger.error(String.format("Fail to add RSAAMInstance for site %s", instInfo.get_siteID()), e);
	}
    }

    private Tenant getTenant(ILdapConnectionEx connection, String name) throws Exception {
	Tenant tenant = null;

	String tenantsRootDn = this.lookupTenantsRootDn(connection, name);

	if (ServerUtils.isNullOrEmpty(tenantsRootDn) == false) {
	    TenantLdapObject tenantConfigObject = TenantLdapObject.getInstance();
	    tenant = tenantConfigObject.retrieveObject(connection, tenantsRootDn, LdapScope.SCOPE_BASE, null);
	}

	return tenant;
    }

    private String getDefaultTenant(ILdapConnectionEx connection) {
	String defaultTenant = null;
	String rootSystemConfigDn = this.getRootSystemConfigDn(connection, false);

	if (ServerUtils.isNullOrEmpty(rootSystemConfigDn) == false) {
	    TenantsContainerLdapObject tenantsContainer = TenantsContainerLdapObject.getInstance();
	    defaultTenant = ServerUtils.getStringValue(tenantsContainer.getObjectProperty(connection,
		    rootSystemConfigDn, LdapScope.SCOPE_ONE_LEVEL,
		    tenantsContainer.getCn(TenantsContainerLdapObject.CONTAINER_TENANTS),
		    TenantsContainerLdapObject.PROPERTY_DEFAULT_TENANT));
	}
	return defaultTenant;
    }

    private void setDefaultTenant(ILdapConnectionEx connection, String defaultTenant) {
	String rootSystemConfigDn = this.getRootSystemConfigDn(connection, false);
	TenantsContainerLdapObject tenantsContainer = TenantsContainerLdapObject.getInstance();
	tenantsContainer.setObjectPropertyValue(
		connection,
		tenantsContainer.getDnFromCn(rootSystemConfigDn,
			tenantsContainer.getCn(TenantsContainerLdapObject.CONTAINER_TENANTS)),
		TenantsContainerLdapObject.PROPERTY_DEFAULT_TENANT, ServerUtils.getLdapValue(defaultTenant));
    }

    private String getSystemTenant(ILdapConnectionEx connection) {
	String defaultTenant = null;
	String rootSystemConfigDn = this.getRootSystemConfigDn(connection, false);

	if (ServerUtils.isNullOrEmpty(rootSystemConfigDn) == false) {
	    TenantsContainerLdapObject tenantsContainer = TenantsContainerLdapObject.getInstance();
	    defaultTenant = ServerUtils.getStringValue(tenantsContainer.getObjectProperty(connection,
		    rootSystemConfigDn, LdapScope.SCOPE_ONE_LEVEL,
		    tenantsContainer.getCn(TenantsContainerLdapObject.CONTAINER_TENANTS),
		    TenantsContainerLdapObject.PROPERTY_SYSTEM_TENANT));
	}
	return defaultTenant;
    }

    private void setSystemTenant(ILdapConnectionEx connection, String systemTenant) {
	String rootSystemConfigDn = this.getRootSystemConfigDn(connection, false);
	TenantsContainerLdapObject tenantsContainer = TenantsContainerLdapObject.getInstance();
	tenantsContainer.setObjectPropertyValue(
		connection,
		tenantsContainer.getDnFromCn(rootSystemConfigDn,
			tenantsContainer.getCn(TenantsContainerLdapObject.CONTAINER_TENANTS)),
		TenantsContainerLdapObject.PROPERTY_SYSTEM_TENANT, ServerUtils.getLdapValue(systemTenant));
    }

    private static Collection<RelyingParty> getRelyingParties(DirectoryConfigStore directoryConfigStore,
	    ILdapConnectionEx connection, String tenantName, String propertyFilter) {
	ValidateUtil.validateNotEmpty(tenantName, "tenantName");

	String tenantsRootDn = directoryConfigStore.ensureTenantExists(connection, tenantName);

	Collection<RelyingParty> relyingParties = retrieveObjectsCollection(connection, tenantsRootDn,
		ContainerLdapObject.CONTAINER_RELYING_PARTIES, propertyFilter, RelyingPartyLdapObject.getInstance(),
		new IObjectProcessedCallback<RelyingParty>() {
		    @Override
		    public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
			    RelyingParty relyingParty) {
			relyingParty.setAssertionConsumerServices(retrieveRelyingPartyAssertionConsumerServices(
				connection, objectDn));
			relyingParty.setAttributeConsumerServices(retrieveRelyingPartyAttributeConsumerServices(
				connection, objectDn));
			relyingParty.setSingleLogoutServices(retrieveRelyingPartySingleLogoutServices(connection,
				objectDn));
			relyingParty.setSignatureAlgorithms(retrieveRelyingPartySignatureAlgorithms(connection,
				objectDn));
		    }
		});

	return relyingParties;
    }

    private IIdentityStoreData getLocalOsDomainIdentityStoreData(IIdentityStoreData idsData) {
	ServerIdentityStoreData serverIdentityStoreData = new ServerIdentityStoreData(idsData.getDomainType(),
		idsData.getName());
	serverIdentityStoreData.setProviderType(IdentityStoreType.IDENTITY_STORE_TYPE_LOCAL_OS);
	HashMap<String, String> map = new HashMap<String, String>();
	map.put(IdmServerConfig.ATTRIBUTE_GROUPS, LocalOsIdentityProvider.GROUPS_ATTRIBUTE);
	map.put(IdmServerConfig.ATTRIBUTE_FIRST_NAME, LocalOsIdentityProvider.FIRST_NAME_ATTRIBUTE);
	map.put(IdmServerConfig.ATTRIBUTE_LAST_NAME, LocalOsIdentityProvider.LAST_NAME_ATTRIBUTE);
	map.put(IdmServerConfig.ATTRIBUTE_SUBJECT_TYPE, LocalOsIdentityProvider.SUBJECT_TYPE_ATTRIBUTE);
	map.put(IdmServerConfig.ATTRIBUTE_USER_PRINCIPAL_NAME, LocalOsIdentityProvider.USER_PRINCIPAL_NAME_ATTRIBUTE);
	map.put(IdmServerConfig.ATTRIBUTE_EMAIL, LocalOsIdentityProvider.EMAIL_ATTRIBUTE);

	serverIdentityStoreData.setAttributeMap(map);
	if (idsData.getExtendedIdentityStoreData() != null) {
	    if (ServerUtils.isNullOrEmpty(idsData.getExtendedIdentityStoreData().getAlias()) == false) {
		serverIdentityStoreData.setAlias(idsData.getExtendedIdentityStoreData().getAlias());
	    }
	}
	return serverIdentityStoreData;
    }

    private void updateSystemDomain(String tenantName, ServerIdentityStoreData configData) throws Exception {
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    Tenant tenant = this.getTenant(connection, tenantName);

	    String identityManagerDn = this.getRootSystemConfigDn(connection, true);

	    String tenantsContainerDn = DirectoryConfigStore.ensureObjectExists(connection, identityManagerDn,
		    TenantsContainerLdapObject.getInstance(), TenantsContainerLdapObject.CONTAINER_TENANTS, false);

	    ValidateUtil.validateNotEmpty(tenantsContainerDn, "Tenant container DN");

	    TenantLdapObject tenantObject = TenantLdapObject.getInstance();

	    String tenantDn = tenantObject.getDnFromObject(tenantsContainerDn, tenant);

	    IdentityProviderLdapObject identityProviderConfigObject = IdentityProviderLdapObject.getInstance();

	    String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantDn,
		    ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, false);

	    ValidateUtil.validateNotEmpty(identityProvidersContainerDn, "Identity provider container DN");

	    String identityProviderDn = identityProviderConfigObject.getDnFromObject(identityProvidersContainerDn,
		    configData);

	    enCryptPassword(tenant._tenantKey, configData.getExtendedIdentityStoreData());
	    identityProviderConfigObject.updateObject(connection, identityProviderDn, configData);
	}
    }

    private static void SaveIdentityProviderConfig(DirectoryConfigStore directoryConfigStore,
	    ILdapConnectionEx connection, String tenantsRootDn, IIdentityStoreData idsData) throws Exception {
	IdentityProviderLdapObject identityProviderConfigObject = IdentityProviderLdapObject.getInstance();
	IdentityProviderAliasLdapObject identityProviderAliasConfigObject = IdentityProviderAliasLdapObject
		.getInstance();

	String identityProvidersContainerDn = DirectoryConfigStore.ensureObjectExists(connection, tenantsRootDn,
		ContainerLdapObject.getInstance(), ContainerLdapObject.CONTAINER_IDENTITY_PROVIDERS, true);

	IIdentityStoreDataEx storeDataEx = idsData.getExtendedIdentityStoreData();
	String aliasName = null;
	if ((storeDataEx != null) && (ServerUtils.isNullOrEmpty(storeDataEx.getAlias()) == false)) {
	    aliasName = storeDataEx.getAlias();
	}

	if (ServerUtils.isNullOrEmpty(aliasName) == false) {
	    // validate alias will not clash with anything
	    String existingObject = null;
	    try {
		existingObject = identityProviderConfigObject.lookupObject(connection, identityProvidersContainerDn,
			LdapScope.SCOPE_ONE_LEVEL, aliasName);
		if (ServerUtils.isNullOrEmpty(existingObject) == false) {
		    throw new DuplicateProviderException(null, aliasName);
		}
		existingObject = identityProviderAliasConfigObject.lookupObject(connection,
			identityProvidersContainerDn, LdapScope.SCOPE_ONE_LEVEL, aliasName);
		if (ServerUtils.isNullOrEmpty(existingObject) == false) {
		    throw new DuplicateProviderException(String.format(
			    "Identity Store Provider '%s' is already in use.", aliasName));
		}
	    } catch (NoSuchObjectLdapException ex) {
		// Do nothing continue;
	    }
	}

	String identityProviderDn = identityProviderConfigObject.getDnFromObject(identityProvidersContainerDn, idsData);
	identityProviderConfigObject.createObject(connection, identityProviderDn, idsData);

	DirectoryConfigStore.saveIdentityProviderAttributesMap(connection, identityProviderDn,
		(storeDataEx != null) ? storeDataEx.getAttributeMap() : null);

	DirectoryConfigStore.saveIdentityProviderSchemaMapping(connection, identityProviderDn,
		(storeDataEx != null) ? storeDataEx.getIdentityStoreSchemaMapping() : null);

	if (ServerUtils.isNullOrEmpty(aliasName) == false && !aliasName.equalsIgnoreCase(idsData.getName())) {
	    IdentityProviderAlias alias = new IdentityProviderAlias(identityProviderDn, aliasName);
	    // create alias
	    identityProviderAliasConfigObject.createObject(connection,
		    identityProviderAliasConfigObject.getDnFromObject(identityProvidersContainerDn, alias), alias);
	}
    }

    private static void saveIdentityProviderAttributesMap(ILdapConnectionEx connection, String identityProviderDn,
	    Map<String, String> attributesMap) {
	Collection<AttributeMapping> mappings = Collections.emptyList();

	if ((attributesMap != null) && (attributesMap.size() > 0)) {
	    mappings = new ArrayList<AttributeMapping>(attributesMap.size());

	    int i = 0;
	    for (Map.Entry<String, String> pair : attributesMap.entrySet()) {
		mappings.add(new AttributeMapping(pair.getKey(), pair.getValue(), i++));
	    }
	}

	saveObjectsCollection(connection, identityProviderDn, ContainerLdapObject.CONTAINER_ATRIBUTE_MAP,
		AttributeMappingLdapObject.getInstance(), mappings, null);
    }

    private static Map<String, String> retrieveIdentityProviderAttributesMap(ILdapConnectionEx connection,
	    String identityProviderDn) {
	Collection<AttributeMapping> mappings = retrieveObjectsCollection(connection, identityProviderDn,
		ContainerLdapObject.CONTAINER_ATRIBUTE_MAP, AttributeMappingLdapObject.getInstance(), null);

	Map<String, String> map = Collections.emptyMap();

	if ((mappings != null) && (mappings.size() > 0)) {
	    map = new HashMap<String, String>(mappings.size());

	    for (AttributeMapping attrMapping : mappings) {
		map.put(attrMapping.getAttributeFrom(), attrMapping.getAttributeTo());
	    }
	}

	return map;
    }

    private static void saveIdentityProviderSchemaMapping(ILdapConnectionEx connection, String identityProviderDn,
	    IdentityStoreSchemaMapping schemaMapping) {
	Collection<IdentityStoreObjectMapping> objectMappings = (schemaMapping != null) ? schemaMapping
		.getObjectMappings() : null;
	Collection<AttributeMapping> attrsMapping = Collections.emptyList();

	if ((objectMappings != null) && (objectMappings.size() > 0)) {
	    attrsMapping = new ArrayList<AttributeMapping>(objectMappings.size());

	    int i = 0;
	    for (IdentityStoreObjectMapping objectMapping : objectMappings) {
		String storeObjectClass = objectMapping.getObjectClass();
		if (ServerUtils.isNullOrEmpty(storeObjectClass) == false) {
		    attrsMapping.add(new AttributeMapping(objectMapping.getObjectId(), storeObjectClass, i++));
		}
		for (IdentityStoreAttributeMapping attributeMapping : objectMapping.getAttributeMappings()) {
		    attrsMapping.add(new AttributeMapping(objectMapping.getObjectId() + ":"
			    + attributeMapping.getAttributeId(), attributeMapping.getAttributeName(), i++));
		}
	    }
	}

	saveObjectsCollection(connection, identityProviderDn,
		ContainerLdapObject.CONTAINER_IDENTITY_STORE_SCHEMA_MAPPING, AttributeMappingLdapObject.getInstance(),
		attrsMapping, null);
    }

    private static IdentityStoreSchemaMapping retrieveIdentityStoreSchemaMapping(ILdapConnectionEx connection,
	    String identityProviderDn) {
	Collection<AttributeMapping> mappings = retrieveObjectsCollection(connection, identityProviderDn,
		ContainerLdapObject.CONTAINER_IDENTITY_STORE_SCHEMA_MAPPING, AttributeMappingLdapObject.getInstance(),
		null);

	IdentityStoreSchemaMapping schemaMapping = null;

	if ((mappings != null) && (mappings.size() > 0)) {
	    IdentityStoreSchemaMapping.Builder schemaMappingBuilder = new IdentityStoreSchemaMapping.Builder();

	    Map<String, IdentityStoreObjectMapping.Builder> objectMappingBuilderMap = new HashMap<String, IdentityStoreObjectMapping.Builder>();

	    for (AttributeMapping attrMapping : mappings) {
		if (attrMapping != null) {
		    String attrFrom = attrMapping.getAttributeFrom();
		    if (ServerUtils.isNullOrEmpty(attrFrom) != true) {
			String[] parts = attrFrom.split(":");
			IdentityStoreObjectMapping.Builder objectMappingBuilder = objectMappingBuilderMap.get(parts[0]);

			if (objectMappingBuilder == null) {
			    objectMappingBuilder = new IdentityStoreObjectMapping.Builder(parts[0]);
			    objectMappingBuilderMap.put(parts[0], objectMappingBuilder);
			}
			if (parts.length == 1) {
			    objectMappingBuilder.setObjectClass(attrMapping.getAttributeTo());
			} else {
			    IdentityStoreAttributeMapping attr = new IdentityStoreAttributeMapping(parts[1],
				    attrMapping.getAttributeTo());
			    objectMappingBuilder.addAttributeMapping(attr);
			}
		    }
		}
	    }

	    for (IdentityStoreObjectMapping.Builder objectMappingBuilder : objectMappingBuilderMap.values()) {
		schemaMappingBuilder.addObjectMappings(objectMappingBuilder.buildObjectMapping());
	    }

	    schemaMapping = schemaMappingBuilder.buildSchemaMapping();
	}

	return schemaMapping;
    }

    private static IIdentityStoreData getIIdenttyStoreDataToReturn(IIdentityStoreData provider, boolean getInternalInfo) {
	IIdentityStoreData iIdentityStoreData = null;
	if (provider instanceof ServerIdentityStoreData) {
	    ServerIdentityStoreData storeData = (ServerIdentityStoreData) provider;
	    if (getInternalInfo == false) {
		iIdentityStoreData = storeData.getExternalIdentityStoreData();
	    } else {
		iIdentityStoreData = storeData;
	    }
	}
	return iIdentityStoreData;
    }

    private static void retrieveIdentityProviderAttributesMap(ILdapConnectionEx connection, String identityProviderDn,
	    IIdentityStoreData provider, boolean getInternalInfo) {
	if (provider instanceof ServerIdentityStoreData) {
	    ServerIdentityStoreData storeData = (ServerIdentityStoreData) provider;
	    if ((storeData.getDomainType() == DomainType.EXTERNAL_DOMAIN) || (getInternalInfo == true)) {
		Map<String, String> map = retrieveIdentityProviderAttributesMap(connection, identityProviderDn);

		if (map != null) {
		    storeData.setAttributeMap(map);
		}
	    }
	}
    }

    private static void retrieveIdentityProviderSchemaMapping(ILdapConnectionEx connection, String identityProviderDn,
	    IIdentityStoreData provider, boolean getInternalInfo) {
	if (provider instanceof ServerIdentityStoreData) {
	    ServerIdentityStoreData storeData = (ServerIdentityStoreData) provider;
	    if ((storeData.getDomainType() == DomainType.EXTERNAL_DOMAIN) || (getInternalInfo == true)) {
		IdentityStoreSchemaMapping schemaMapping = retrieveIdentityStoreSchemaMapping(connection,
			identityProviderDn);
		if (schemaMapping != null) {
		    storeData.setSchemaMapping(schemaMapping);
		}
	    }
	}
    }

    private static void saveRelyingPartyAssertionConsumerServices(ILdapConnectionEx connection, String relyingPartyDn,
	    Collection<AssertionConsumerService> services) {
	saveObjectsCollection(connection, relyingPartyDn, ContainerLdapObject.CONTAINER_ASSERTION_COMSUMER_SERVICES,
		AssertionConsumerServiceLdapObject.getInstance(), services, null);
    }

    private static Collection<AssertionConsumerService> retrieveRelyingPartyAssertionConsumerServices(
	    ILdapConnectionEx connection, String relyingPartyDn) {
	return retrieveObjectsCollection(connection, relyingPartyDn,
		ContainerLdapObject.CONTAINER_ASSERTION_COMSUMER_SERVICES,
		AssertionConsumerServiceLdapObject.getInstance(), null);
    }

    private static void saveRelyingPartyAttributeConsumerServices(ILdapConnectionEx connection, String relyingPartyDn,
	    Collection<AttributeConsumerService> services) {
	saveObjectsCollection(connection, relyingPartyDn, ContainerLdapObject.CONTAINER_ATTRIBUTE_COMSUMER_SERVICES,
		AttributeConsumerServiceLdapObject.getInstance(), services,
		new IObjectProcessedCallback<AttributeConsumerService>() {
		    @Override
		    public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
			    AttributeConsumerService service) {
			saveAttributes(connection, objectDn, service.getAttributes());
		    }
		});
    }

    private static Collection<AttributeConsumerService> retrieveRelyingPartyAttributeConsumerServices(
	    ILdapConnectionEx connection, String relyingPartyDn) {
	return retrieveObjectsCollection(connection, relyingPartyDn,
		ContainerLdapObject.CONTAINER_ATTRIBUTE_COMSUMER_SERVICES,
		AttributeConsumerServiceLdapObject.getInstance(),
		new IObjectProcessedCallback<AttributeConsumerService>() {
		    @Override
		    public void processObjectSaved(ILdapConnectionEx connection, String objectDn,
			    AttributeConsumerService service) {
			service.setAttributes(retrieveAttributes(connection, objectDn));
		    }
		});
    }

    private static void saveRelyingPartySingleLogoutServices(ILdapConnectionEx connection, String relyingPartyDn,
	    Collection<ServiceEndpoint> services) {
	saveObjectsCollection(connection, relyingPartyDn, ContainerLdapObject.CONTAINER_SINGLE_LOGOUT_SERVICES,
		SingleLogoutServiceLdapObject.getInstance(), services, null);
    }

    private static Collection<ServiceEndpoint> retrieveRelyingPartySingleLogoutServices(ILdapConnectionEx connection,
	    String relyingPartyDn) {
	return retrieveObjectsCollection(connection, relyingPartyDn,
		ContainerLdapObject.CONTAINER_SINGLE_LOGOUT_SERVICES, SingleLogoutServiceLdapObject.getInstance(), null);
    }

    private static void saveRelyingPartySignatureAlgorithms(ILdapConnectionEx connection, String relyingPartyDn,
	    Collection<SignatureAlgorithm> algorithms) {
	saveIndexWrappedObjectsCollecton(connection, relyingPartyDn,
		ContainerLdapObject.CONTAINER_SIGNATURE_ALGORITHMS_SERVICES,
		SignatureAlgorithmsLdapObject.getInstance(), algorithms, null);
    }

    private static Collection<SignatureAlgorithm> retrieveRelyingPartySignatureAlgorithms(ILdapConnectionEx connection,
	    String relyingPartyDn) {
	return retrieveIndexWrappedObjectsCollection(connection, relyingPartyDn,
		ContainerLdapObject.CONTAINER_SIGNATURE_ALGORITHMS_SERVICES,
		SignatureAlgorithmsLdapObject.getInstance(), null, false);
    }

    private static void saveAttributes(ILdapConnectionEx connection, String parentDn, Collection<Attribute> attributes) {
	saveIndexWrappedObjectsCollecton(connection, parentDn, ContainerLdapObject.CONTAINER_ATTRIBUTES,
		TenantAttributeLdapObject.getInstance(), attributes, null);
    }

    private static Collection<Attribute> retrieveAttributes(ILdapConnectionEx connection, String parentDn) {
	return retrieveIndexWrappedObjectsCollection(connection, parentDn, ContainerLdapObject.CONTAINER_ATTRIBUTES,
		TenantAttributeLdapObject.getInstance(), null, false);
    }

    private interface IObjectProcessedCallback<T> {
	void processObjectSaved(ILdapConnectionEx connection, String objectDn, T object);
    }

    private static <T> void saveIndexWrappedObjectsCollecton(ILdapConnectionEx connection, String parentDn,
	    String containerCn, ILdapObject<IndexedObjectWrapper<T>> iLdapObject, Collection<T> objects,
	    IObjectProcessedCallback<T> callback) {
	ContainerLdapObject containerConfigObject = ContainerLdapObject.getInstance();

	String containerDn = containerConfigObject.lookupObject(connection, parentDn, LdapScope.SCOPE_ONE_LEVEL,
		containerCn);

	if (ServerUtils.isNullOrEmpty(containerDn) == false) {
	    // remove all existing objects
	    containerConfigObject.deleteObject(connection, containerDn);
	}

	if ((objects != null) && (objects.size() > 0)) {
	    containerDn = containerConfigObject.getDnFromObject(parentDn, containerCn);
	    containerConfigObject.createObject(connection, containerDn, containerCn);

	    int i = 0;
	    IndexedObjectWrapper<T> theObject = null;
	    for (T object : objects) {
		theObject = new IndexedObjectWrapper<T>(object, i);
		String objectsDn = iLdapObject.getDnFromObject(containerDn, theObject);
		iLdapObject.createObject(connection, objectsDn, theObject);

		if (callback != null) {
		    callback.processObjectSaved(connection, objectsDn, object);
		}
		i++;
	    }
	}
    }

    private static <T> void saveObjectsCollection(ILdapConnectionEx connection, String parentDn, String containerCn,
	    ILdapObject<T> iLdapObject, Collection<T> objects, IObjectProcessedCallback<T> callback) {
	ContainerLdapObject containerConfigObject = ContainerLdapObject.getInstance();

	String containerDn = containerConfigObject.lookupObject(connection, parentDn, LdapScope.SCOPE_ONE_LEVEL,
		containerCn);

	if (ServerUtils.isNullOrEmpty(containerDn) == false) {
	    // remove all existing objects
	    containerConfigObject.deleteObject(connection, containerDn);
	}

	if ((objects != null) && (objects.size() > 0)) {
	    containerDn = containerConfigObject.getDnFromObject(parentDn, containerCn);
	    containerConfigObject.createObject(connection, containerDn, containerCn);

	    for (T object : objects) {
		String objectsDn = iLdapObject.getDnFromObject(containerDn, object);
		iLdapObject.createObject(connection, objectsDn, object);

		if (callback != null) {
		    callback.processObjectSaved(connection, objectsDn, object);
		}
	    }
	}
    }

    private static <T> Collection<T> retrieveIndexWrappedObjectsCollection(ILdapConnectionEx connection,
	    String parentDn, String containerCn, ILdapObject<IndexedObjectWrapper<T>> iLdapObject,
	    IObjectProcessedCallback<T> callback, boolean needsSort) {
	return retrieveIndexWrappedObjectsCollection(connection, parentDn, containerCn, null, iLdapObject, callback,
		needsSort);
    }

    private static <T> Collection<T> retrieveIndexWrappedObjectsCollection(ILdapConnectionEx connection,
	    String parentDn, String containerCn, String additionalFilter,
	    ILdapObject<IndexedObjectWrapper<T>> iLdapObject, IObjectProcessedCallback<T> callback, boolean needsSort) {
	ContainerLdapObject containerConfig = ContainerLdapObject.getInstance();
	String containerDn = containerConfig.lookupObject(connection, parentDn, LdapScope.SCOPE_ONE_LEVEL, containerCn);

	Collection<T> objects = new ArrayList<T>();

	if (ServerUtils.isNullOrEmpty(containerDn) == false) {
	    List<IndexedObjectWrapper<T>> objectsCollection = iLdapObject.searchObjects(connection, containerDn,
		    LdapScope.SCOPE_ONE_LEVEL, additionalFilter);

	    if ((objectsCollection != null) && (objectsCollection.size() > 0)) {
		if (needsSort == true) {
		    Collections.sort(objectsCollection, new Comparator<IndexedObjectWrapper<T>>() {
			@Override
			public int compare(IndexedObjectWrapper<T> o1, IndexedObjectWrapper<T> o2) {
			    return o1.getIndex() - o2.getIndex();
			}
		    });
		}

		for (IndexedObjectWrapper<T> object : objectsCollection) {
		    T candidate = object.getWrappedObject();

		    if (callback != null) {
			callback.processObjectSaved(connection, iLdapObject.getDnFromObject(containerDn, object),
				candidate);
		    }

		    objects.add(candidate);
		}
	    }
	}

	return objects;
    }

    private static <T> Collection<T> retrieveObjectsCollection(ILdapConnectionEx connection, String parentDn,
	    String containerCn, ILdapObject<T> iLdapObject, IObjectProcessedCallback<T> callback) {
	return retrieveObjectsCollection(connection, parentDn, containerCn, null, iLdapObject, callback);
    }

    private static <T> Collection<T> retrieveObjectsCollection(ILdapConnectionEx connection, String parentDn,
	    String containerCn, String additionalFilter, ILdapObject<T> iLdapObject,
	    IObjectProcessedCallback<T> callback) {
	ContainerLdapObject containerConfig = ContainerLdapObject.getInstance();
	String containerDn = containerConfig.lookupObject(connection, parentDn, LdapScope.SCOPE_ONE_LEVEL, containerCn);

	Collection<T> objects = new ArrayList<T>();

	if (ServerUtils.isNullOrEmpty(containerDn) == false) {
	    List<T> objectsCollection = iLdapObject.searchObjects(connection, containerDn, LdapScope.SCOPE_ONE_LEVEL,
		    additionalFilter);

	    if ((objectsCollection != null) && (objectsCollection.size() > 0)) {
		for (T object : objectsCollection) {
		    if (callback != null) {
			callback.processObjectSaved(connection, iLdapObject.getDnFromObject(containerDn, object),
				object);
		    }

		    objects.add(object);
		}
	    }
	}

	return objects;
    }

    private void createSystemDomainIdentityProviderForTenant(ILdapConnectionEx connection, String tenantName,
	    String tenantsRootDn, String adminAccountName, char[] adminPwd) throws Exception {
	IdmServerConfig settings = IdmServerConfig.getInstance();

	String systemDomainName = settings.getTenantsSystemDomainName(tenantName);

	ServerIdentityStoreData serverIdentityStoreData = new ServerIdentityStoreData(DomainType.SYSTEM_DOMAIN,
		systemDomainName);

	Tenant tenant = this.getTenant(connection, tenantName);

	serverIdentityStoreData.setProviderType(settings.getSystemDomainIdentityStoreType());
	serverIdentityStoreData.setAuthenticationType(settings.getSystemDomainAuthenticationType());
	serverIdentityStoreData.setSearchTimeoutSeconds(settings.getSystemDomainSearchTimeout());
	serverIdentityStoreData.setUserBaseDn(ServerUtils.getDomainDN(systemDomainName));
	serverIdentityStoreData.setGroupBaseDn(ServerUtils.getDomainDN(systemDomainName));
	serverIdentityStoreData.setUserName(settings.getTenantAdminUserName(tenantName, adminAccountName));
	serverIdentityStoreData.setPassword(String.valueOf(adminPwd));
	serverIdentityStoreData.setConnectionStrings(ServerUtils.getConnectionStringFromUris(settings
		.getSystemDomainConnectionInfo()));

	serverIdentityStoreData.setAttributeMap(settings.getSystemDomainAttributesMap());

	// if it is a service provider system domain
	if (tenantName.equalsIgnoreCase(settings.getDirectoryConfigStoreDomain())) {
	    if (settings.isServiceProviderSystemDomainInBackCompatMode() == true) {
		serverIdentityStoreData.setAlias(settings.getServiceProviderSystemDomianAlias());
		serverIdentityStoreData.setUpnSuffixes(Collections.<String> singletonList(settings
			.getServiceProviderSystemDomianAlias()));
	    }
	}

	try {
	    enCryptPassword(tenant._tenantKey, serverIdentityStoreData.getExtendedIdentityStoreData());
	    DirectoryConfigStore.SaveIdentityProviderConfig(this, connection, tenantsRootDn, serverIdentityStoreData);
	} catch (AlreadyExistsLdapException ex) {
	    throw new DuplicateProviderException(serverIdentityStoreData.getName(), (String) null);
	}
    }

    private LdapValue[] getTenantProperty(String tenantName, String propertyName) throws Exception {
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.lookupTenantsRootDn(connection, tenantName);
	    if (ServerUtils.isNullOrEmpty(tenantsRootDn)) {
		throw new RuntimeException(String.format("Tenant '%s' does not exist.", tenantName));
	    }

	    return DirectoryConfigStore.getTenantProperty(connection, tenantsRootDn, propertyName);
	}
    }

    private static LdapValue[] getTenantProperty(ILdapConnectionEx connection, String tenantsRootDn, String propertyName)
	    throws Exception {
	return TenantLdapObject.getInstance().getObjectProperty(connection, tenantsRootDn, LdapScope.SCOPE_BASE, null,
		propertyName);
    }

    private void setTenantProperty(String tenantName, String propertyName, LdapValue[] value) throws Exception {
	try (PooledLdapConnection pooledConnection = borrowConnection()) {
	    ILdapConnectionEx connection = pooledConnection.getConnection();
	    String tenantsRootDn = this.lookupTenantsRootDn(connection, tenantName);
	    if (ServerUtils.isNullOrEmpty(tenantsRootDn)) {
		throw new NoSuchTenantException(String.format("Tenant '%s' does not exist.", tenantName));
	    }
	    logger.info("tenantsRootDn: " + tenantsRootDn + ", property name: " + propertyName + ", value: " + value);

	    setTenantProperty(connection, tenantsRootDn, propertyName, value);
	}
    }

    private static void setTenantProperty(ILdapConnectionEx connection, String tenantsRootDn, String propertyName,
	    LdapValue[] value) throws Exception {
	TenantLdapObject.getInstance().setObjectPropertyValue(connection, tenantsRootDn, propertyName, value);
    }

    // returns tenant's root dn if exists...
    private String ensureTenantExists(ILdapConnectionEx connection, String tenantName) {
	String tenantsRootDn = this.lookupTenantsRootDn(connection, tenantName);

	if (ServerUtils.isNullOrEmpty(tenantsRootDn)) {
	    throw new IllegalArgumentException(String.format("Tenant '%s' does not exist.", tenantName));
	}

	return tenantsRootDn;
    }

    private String getSystemTenantCredsDn() {
	String lduGuid = Directory.GetLocalLduGuid();
	ValidateUtil.validateNotEmpty(lduGuid, "LduGuid");

	return String.format("CN=%s,CN=Ldus,CN=ComponentManager,%s", lduGuid, this.getRootDn());
    }

    private String getSystemTenantLduDn() {
	return String.format("CN=Ldus,CN=ComponentManager,%s", this.getRootDn());
    }

    private String getTenantCredentialCn(int index) {
	return String.format("%s-%d", this.tenantCredNamePrefix, index);
    }

    private String getTenantCredentialDn(int index, String tenantRootDn) {
	return String.format("CN=%s,%s", getTenantCredentialCn(index), tenantRootDn);
    }

    private String getTenantTrustedCertChainCn(int index) {
	return String.format("%s-%d", this.tenantTrustedCertChainPrefix, index);
    }

    private String lookupTenantsRootDn(ILdapConnectionEx connection, String tenantName) {
	String tenantsRootDn = null;
	String rootSystemConfigDn = this.getRootSystemConfigDn(connection, false);
	if (ServerUtils.isNullOrEmpty(rootSystemConfigDn) == false) {
	    TenantsContainerLdapObject tenantsConfigObject = TenantsContainerLdapObject.getInstance();

	    String tenantsContainerDn = tenantsConfigObject.lookupObject(connection, rootSystemConfigDn,
		    LdapScope.SCOPE_ONE_LEVEL, tenantsConfigObject.getCn(TenantsContainerLdapObject.CONTAINER_TENANTS));
	    if (ServerUtils.isNullOrEmpty(tenantsContainerDn) == false) {
		TenantLdapObject tenantConfig = TenantLdapObject.getInstance();
		tenantsRootDn = tenantConfig.lookupObject(connection, tenantsContainerDn, LdapScope.SCOPE_ONE_LEVEL,
			tenantName);
	    }
	}

	return tenantsRootDn;
    }

    private Collection<String> lookupAllTenantsDn(ILdapConnectionEx connection) {
	String rootSystemConfigDn = this.getRootSystemConfigDn(connection, false);
	Collection<String> result = new ArrayList<String>();

	if (ServerUtils.isNullOrEmpty(rootSystemConfigDn) == false) {
	    TenantsContainerLdapObject tenantsConfigObject = TenantsContainerLdapObject.getInstance();

	    // get tenant container dn:
	    // "cn=Tenants,cn=IdentityManager,cn=services,dc=vsphere,dc=local"
	    String tenantsContainerDn = tenantsConfigObject.lookupObject(connection, rootSystemConfigDn,
		    LdapScope.SCOPE_ONE_LEVEL, tenantsConfigObject.getCn(TenantsContainerLdapObject.CONTAINER_TENANTS));

	    if (ServerUtils.isNullOrEmpty(tenantsContainerDn) == false) {
		List<Tenant> allTenants = TenantLdapObject.getInstance().searchObjects(connection, tenantsContainerDn,
			LdapScope.SCOPE_ONE_LEVEL);

		if ((allTenants != null) && (allTenants.size() > 0)) {
		    for (Tenant tenant : allTenants) {
			result.add(tenant.getName());
		    }
		}
	    }
	}

	return result;
    }

    private PooledLdapConnection borrowConnection() throws Exception {
        Exception latestEx = null;
        IdmServerConfig config = IdmServerConfig.getInstance();
        for (URI connectionString : this._uris) {
            PooledLdapConnectionIdentity.Builder builder = new PooledLdapConnectionIdentity.Builder(
                    connectionString.toString(), config.getDirectoryConfigStoreAuthType());
            PooledLdapConnectionIdentity pooledLdapConnectionIdentity = builder
                    .setTenantName(config.getDirectoryConfigStoreDomain())
                    .setUsername(config.getDirectoryConfigStoreUserName())
                    .setPassword(config.getDirectoryConfigStorePassword())
                    .setUseGCPort(false)
                    .build();

            try {
                LdapConnectionPool ldapConnectionPool = LdapConnectionPool.getInstance();
                ILdapConnectionEx conn = ldapConnectionPool.borrowConnection(pooledLdapConnectionIdentity);
                return new PooledLdapConnection(conn, pooledLdapConnectionIdentity, ldapConnectionPool);
            } catch (Exception e) {
                logger.error(e);
                latestEx = e;
            }
        }
        throw latestEx;
    }

    private String getRootSystemConfigDn(ILdapConnectionEx connection, boolean createIfNotExists) {
	// services->IdentityManager
	ContainerLdapObject containerConfigObject = ContainerLdapObject.getInstance();

	String identityManagerDn = null;
	String servicesDn = DirectoryConfigStore.ensureObjectExists(connection, this.getRootDn(),
		containerConfigObject, ContainerLdapObject.CONTAINER_SERVICES, createIfNotExists);

	if (ServerUtils.isNullOrEmpty(servicesDn) == false) {
	    identityManagerDn = DirectoryConfigStore.ensureObjectExists(connection, servicesDn, containerConfigObject,
		    ContainerLdapObject.CONTAINER_IDENTITY_MANAGER, createIfNotExists);
	}
	return identityManagerDn;
    }

    private static <T> String ensureObjectExists(ILdapConnectionEx connection, String parentDn,
	    ILdapObject<T> iLdapObject, T object, boolean createIfNotExists) {
	String objectDn = iLdapObject.lookupObject(connection, parentDn, LdapScope.SCOPE_ONE_LEVEL,
		iLdapObject.getCn(object));

	if ((ServerUtils.isNullOrEmpty(objectDn) == true) && (createIfNotExists == true)) {
	    objectDn = iLdapObject.getDnFromObject(parentDn, object);
	    iLdapObject.createObject(connection, objectDn, object);
	}

	return objectDn;
    }

    private String getRootDn() {
	return this._configRootDn;
    }

    private void enCryptPassword(String tenantKey, IIdentityStoreDataEx idsDataEx) throws Exception {
	if (idsDataEx != null) {
	    CryptoAESE cryptoAES = new CryptoAESE(tenantKey);
	    String secret = idsDataEx.getPassword();
	    if (secret != null) {
		idsDataEx.setPassword(new BASE64Encoder().encode(cryptoAES.encrypt(secret)));
	    }
	}
    }

    private void deCryptPassword(CryptoAESE cryptoAES, IIdentityStoreDataEx idsDataEx) throws Exception {
	if (idsDataEx != null) {
	    String secret = idsDataEx.getPassword();
	    if (secret != null) {
		idsDataEx.setPassword(cryptoAES.decrypt(new BASE64Decoder().decodeBuffer(secret)));
	    }
	}
    }

}
