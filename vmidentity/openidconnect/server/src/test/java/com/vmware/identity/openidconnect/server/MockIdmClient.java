/*
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
 */

package com.vmware.identity.openidconnect.server;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IdentityStoreData;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.NoSuchResourceServerException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * @author Yehia Zayour
 */
public class MockIdmClient extends CasIdmClient {
    private final String tenantName;
    private final PrivateKey tenantPrivateKey;
    private final Certificate tenantCertificate;
    private final AuthnPolicy authnPolicy;
    private final String issuer;

    private final String clientId;
    private final String redirectUri;
    private final String logoutUri;
    private final String postLogoutRedirectUri;
    private final String clientCertSubjectDN;
    private final Certificate clientCertificate;
    private final String tokenEndpointAuthMethod;
    private final OIDCClient additionalClient;

    private final String username;
    private final String password;
    private final String securIdPasscode;
    private final String securIdSessionId;
    private final boolean securIdNewPinRequired;
    private final String gssContextId;
    private final byte[] gssServerLeg;
    private final boolean personUserEnabled;

    private final String solutionUsername;
    private final String solutionUserCertSubjectDN;
    private final boolean solutionUserEnabled;

    private final long maxBearerTokenLifetime;
    private final long maxHoKTokenLifetime;
    private final long maxBearerRefreshTokenLifetime;
    private final long maxHoKRefreshTokenLifetime;
    private final long clockTolerance;

    private final Set<String> systemGroupMembership;
    private final Set<String> groupMembership;
    private final Map<String, ResourceServer> resourceServerMap;
    private final Map<String, OIDCClient> clientMap;

    private MockIdmClient(Builder builder) {
        this.tenantName              = builder.tenantName;
        this.tenantPrivateKey        = builder.tenantPrivateKey;
        this.tenantCertificate       = builder.tenantCertificate;
        this.authnPolicy             = builder.authnPolicy;
        this.issuer                  = builder.issuer;

        this.clientId                = builder.clientId;
        this.redirectUri             = builder.redirectUri;
        this.logoutUri               = builder.logoutUri;
        this.postLogoutRedirectUri   = builder.postLogoutRedirectUri;
        this.clientCertSubjectDN     = builder.clientCertSubjectDN;
        this.clientCertificate       = builder.clientCertificate;
        this.tokenEndpointAuthMethod = builder.tokenEndpointAuthMethod;
        this.additionalClient        = builder.additionalClient;

        this.username                = builder.username;
        this.password                = builder.password;
        this.securIdPasscode        = builder.securIdPasscode;
        this.securIdSessionId       = builder.securIdSessionId;
        this.securIdNewPinRequired  = builder.securIdNewPinRequired;
        this.gssContextId            = builder.gssContextId;
        this.gssServerLeg            = builder.gssServerLeg;
        this.personUserEnabled       = builder.personUserEnabled;

        this.solutionUsername               = builder.solutionUsername;
        this.solutionUserCertSubjectDN      = builder.solutionUserCertSubjectDN;
        this.solutionUserEnabled            = builder.solutionUserEnabled;

        this.maxBearerTokenLifetime         = builder.maxBearerTokenLifetime;
        this.maxHoKTokenLifetime            = builder.maxHoKTokenLifetime;
        this.maxBearerRefreshTokenLifetime  = builder.maxBearerRefreshTokenLifetime;
        this.maxHoKRefreshTokenLifetime     = builder.maxHoKRefreshTokenLifetime;
        this.clockTolerance                 = builder.clockTolerance;

        this.systemGroupMembership          = builder.systemGroupMembership;
        this.groupMembership                = builder.groupMembership;
        this.resourceServerMap              = builder.resourceServerMap;

        this.clientMap = new HashMap<String, OIDCClient>();

        if (this.clientId != null) {
            OIDCClient client = new OIDCClient.Builder(this.clientId).
                    redirectUris(Arrays.asList(this.redirectUri)).
                    postLogoutRedirectUris(Arrays.asList(this.postLogoutRedirectUri)).
                    logoutUri(this.logoutUri).
                    certSubjectDN(this.clientCertSubjectDN).
                    tokenEndpointAuthMethod(this.tokenEndpointAuthMethod).
                    tokenEndpointAuthSigningAlg("RS256").build();
            this.clientMap.put(this.clientId, client);
        }
        if (this.additionalClient != null) {
            this.clientMap.put(this.additionalClient.getClientId(), this.additionalClient);
        }
    }

    @Override
    public PrincipalId authenticate(String tenantName, String username, String password) throws Exception {
        validateTenant(tenantName);
        Validate.notEmpty(username, "username");
        Validate.notEmpty(password, "password");

        boolean match =
                Objects.equals(username, this.username) &&
                Objects.equals(password, this.password);
        if (!match) {
            throw new IDMLoginException("invalid credentials");
        }

        return new PrincipalId(this.username, this.tenantName);
    }

    @Override
    public PrincipalId authenticate(
            String tenantName,
            X509Certificate[] tlsCertChain,
            String hint) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notNull(tlsCertChain, "tlsCertChain");

        boolean match =
                tlsCertChain.length == 1 &&
                tlsCertChain[0].equals(this.clientCertificate);
        if (!match) {
            throw new IDMLoginException("invalid credentials");
        }

        return new PrincipalId(this.username, this.tenantName);
    }

    @Override
    public GSSResult authenticate(String tenantName, String contextId, byte[] gssTicket) throws Exception {
        validateTenant(tenantName);
        Validate.notEmpty(contextId, "contextId");
        Validate.notNull(gssTicket, "gssTicket");

        boolean match = Objects.equals(contextId, this.gssContextId);
        if (!match) {
            throw new IDMLoginException("invalid credentials");
        }

        return (this.gssServerLeg != null) ?
                new GSSResult(this.gssContextId, this.gssServerLeg) :
                new GSSResult(this.gssContextId, new PrincipalId(this.username, this.tenantName));
    }

    @Override
    public RSAAMResult authenticateRsaSecurId(
            String tenantName,
            String sessionId,
            String principal,
            String passcode) throws Exception {
        validateTenant(tenantName);
        // nullable sessionId
        Validate.notEmpty(principal, "principal");
        Validate.notEmpty(passcode, "passcode");

        boolean match =
                Objects.equals(principal, this.username) &&
                Objects.equals(passcode, this.securIdPasscode);
        if (!match) {
            throw new IDMLoginException("invalid credentials");
        }

        if (this.securIdNewPinRequired) {
            throw new IDMSecureIDNewPinException("new pin required");
        }

        return (this.securIdSessionId != null) ?
                new RSAAMResult(this.securIdSessionId) :
                new RSAAMResult(new PrincipalId(this.username, this.tenantName));
    }

    @Override
    public boolean isActive(String tenantName, PrincipalId id) throws Exception {
        validateTenant(tenantName);
        Validate.notNull(id, "id");

        boolean result;
        if (id.getName().equals(this.username)) {
            result = this.personUserEnabled;
        } else if (id.getName().equals(this.solutionUsername)) {
            result = this.solutionUserEnabled;
        } else {
            throw new InvalidPrincipalException("invalid principal", id.getUPN());
        }
        return result;
    }

    @Override
    public PersonUser findPersonUser(String tenantName, PrincipalId id) throws Exception {
        validateTenant(tenantName);
        Validate.notNull(id, "id");

        PersonUser result = null;
        if (id.getName().equals(this.username)) {
            PersonDetail detail = new PersonDetail.Builder().build();
            boolean disabled = !this.personUserEnabled;
            boolean locked = false;
            result = new PersonUser(id, detail, disabled, locked);
        }
        return result;
    }

    @Override
    public SolutionUser findSolutionUserByCertDn(String tenantName, String subjectDN) throws Exception {
        validateTenant(tenantName);
        Validate.notEmpty(subjectDN, "subjectDN");

        SolutionUser result = null;
        if (subjectDN.equals(this.solutionUserCertSubjectDN) || subjectDN.equals(this.clientCertSubjectDN)) {
            PrincipalId id = new PrincipalId(this.solutionUsername, this.tenantName);
            SolutionDetail detail = new SolutionDetail((X509Certificate) this.clientCertificate);
            boolean disabled = !this.solutionUserEnabled;
            result = new SolutionUser(id, detail, disabled);
        }
        return result;
    }

    @Override
    public Collection<AttributeValuePair> getAttributeValues(
            String tenantName,
            PrincipalId id,
            Collection<Attribute> attributes) throws Exception {
        validateTenant(tenantName);
        Validate.notNull(id, "id");
        Validate.notEmpty(attributes, "attributes");

        AttributeValuePair pair = new AttributeValuePair();
        pair.setAttrDefinition(attributes.iterator().next());
        pair.getValues().addAll(this.groupMembership);
        return Arrays.asList(pair);
    }

    @Override
    public Collection<IIdentityStoreData> getProviders(
            String tenantName,
            EnumSet<DomainType> domains) throws Exception {
        validateTenant(tenantName);
        Validate.notEmpty(domains, "domains");
        return Collections.singletonList((IIdentityStoreData) IdentityStoreData.CreateSystemIdentityStoreData(this.tenantName));
    }

    @Override
    public Tenant getTenant(String tenantName) throws Exception {
        validateTenant(tenantName);
        return new Tenant(this.tenantName);
    }

    @Override
    public String getDefaultTenant() throws Exception {
        return this.tenantName;
    }

    @Override
    public String getSystemTenant() throws Exception {
        return this.tenantName;
    }

    @Override
    public PrivateKey getTenantPrivateKey(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.tenantPrivateKey;
    }

    @Override
    public List<Certificate> getTenantCertificate(String tenantName) throws Exception {
        validateTenant(tenantName);
        return Arrays.asList(this.tenantCertificate);
    }

    @Override
    public AuthnPolicy getAuthnPolicy(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.authnPolicy;
    }

    @Override
    public String getOIDCEntityID(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.issuer;
    }

    @Override
    public OIDCClient getOIDCClient(String tenantName, String clientId) throws Exception {
        validateTenant(tenantName);
        Validate.notEmpty(clientId, "clientId");

        OIDCClient client = this.clientMap.get(clientId);
        if (client == null) {
            throw new NoSuchOIDCClientException("client not found");
        }
        return client;
    }

    @Override
    public ResourceServer getResourceServer(String tenantName, String resourceServerName) throws Exception {
        validateTenant(tenantName);
        Validate.notEmpty(resourceServerName, "resourceServerName");

        ResourceServer resourceServer = this.resourceServerMap.get(resourceServerName);
        if (resourceServer == null) {
            throw new NoSuchResourceServerException("resource server not found");
        }
        return resourceServer;
    }

    @Override
    public String getBrandName(String tenantName) throws Exception {
        validateTenant(tenantName);
        return null;
    }

    @Override
    public String getLogonBannerTitle(String tenantName) throws Exception {
        validateTenant(tenantName);
        return null;
    }

    @Override
    public String getLogonBannerContent(String tenantName) throws Exception {
        validateTenant(tenantName);
        return null;
    }

    @Override
    public boolean getLogonBannerCheckboxFlag(String tenantName) throws Exception {
        validateTenant(tenantName);
        return true;
    }

    @Override
    public long getMaximumBearerTokenLifetime(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.maxBearerTokenLifetime;
    }

    @Override
    public long getMaximumHoKTokenLifetime(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.maxHoKTokenLifetime;
    }

    @Override
    public long getMaximumBearerRefreshTokenLifetime(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.maxBearerRefreshTokenLifetime;
    }

    @Override
    public long getMaximumHoKRefreshTokenLifetime(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.maxHoKRefreshTokenLifetime;
    }

    @Override
    public long getClockTolerance(String tenantName) throws Exception {
        validateTenant(tenantName);
        return this.clockTolerance;
    }

    @Override
    public boolean isMemberOfSystemGroup(String tenantName, PrincipalId id, String groupName) throws Exception {
        validateTenant(tenantName);
        Validate.notNull(id, "id");
        Validate.notEmpty(groupName, "groupName");
        return this.systemGroupMembership.contains(groupName);
    }

    @Override
    public String getServerSPN() {
        return null;
    }

    @Override
    public String getSsoMachineHostName() {
        return null;
    }

    private void validateTenant(String tenantName) throws NoSuchTenantException {
        Validate.notEmpty(tenantName, "tenantName");
        if (!tenantName.equals(this.tenantName)) {
            throw new NoSuchTenantException("tenant not found");
        }
    }

    public static class Builder {
        private String tenantName;
        private PrivateKey tenantPrivateKey;
        private Certificate tenantCertificate;
        private AuthnPolicy authnPolicy;
        private String issuer;

        private String clientId;
        private String redirectUri;
        private String logoutUri;
        private String postLogoutRedirectUri;
        private String clientCertSubjectDN;
        private Certificate clientCertificate;
        private String tokenEndpointAuthMethod;
        private OIDCClient additionalClient;

        private String username;
        private String password;
        private String securIdPasscode;
        private String securIdSessionId;
        private boolean securIdNewPinRequired;
        private String gssContextId;
        private byte[] gssServerLeg;
        private boolean personUserEnabled;

        private String solutionUsername;
        private String solutionUserCertSubjectDN;
        private boolean solutionUserEnabled;

        private long maxBearerTokenLifetime;
        private long maxHoKTokenLifetime;
        private long maxBearerRefreshTokenLifetime;
        private long maxHoKRefreshTokenLifetime;
        private long clockTolerance;

        private Set<String> systemGroupMembership;
        private Set<String> groupMembership;
        private Map<String, ResourceServer> resourceServerMap;

        public Builder() {
        }

        public Builder tenantName(String tenantName) {
            this.tenantName = tenantName;
            return this;
        }

        public Builder tenantPrivateKey(PrivateKey tenantPrivateKey) {
            this.tenantPrivateKey = tenantPrivateKey;
            return this;
        }

        public Builder tenantCertificate(Certificate tenantCertificate) {
            this.tenantCertificate = tenantCertificate;
            return this;
        }

        public Builder authnPolicy(AuthnPolicy authnPolicy) {
            this.authnPolicy = authnPolicy;
            return this;
        }

        public Builder issuer(String issuer) {
            this.issuer = issuer;
            return this;
        }

        public Builder clientId(String clientId) {
            this.clientId = clientId;
            return this;
        }

        public Builder redirectUri(String redirectUri) {
            this.redirectUri = redirectUri;
            return this;
        }

        public Builder logoutUri(String logoutUri) {
            this.logoutUri = logoutUri;
            return this;
        }

        public Builder postLogoutRedirectUri(String postLogoutRedirectUri) {
            this.postLogoutRedirectUri = postLogoutRedirectUri;
            return this;
        }

        public Builder clientCertSubjectDN(String clientCertSubjectDN) {
            this.clientCertSubjectDN = clientCertSubjectDN;
            return this;
        }

        public Builder clientCertificate(Certificate clientCertificate) {
            this.clientCertificate = clientCertificate;
            return this;
        }

        public Builder tokenEndpointAuthMethod(String tokenEndpointAuthMethod) {
            Validate.notEmpty(tokenEndpointAuthMethod);
            this.tokenEndpointAuthMethod = tokenEndpointAuthMethod;
            return this;
        }

        public Builder additionalClient(OIDCClient additionalClient) {
            this.additionalClient = additionalClient;
            return this;
        }

        public Builder username(String username) {
            this.username = username;
            return this;
        }

        public Builder password(String password) {
            this.password = password;
            return this;
        }

        public Builder securIdPasscode(String securIdPasscode) {
            this.securIdPasscode = securIdPasscode;
            return this;
        }

        public Builder securIdSessionId(String securIdSessionId) {
            this.securIdSessionId = securIdSessionId;
            return this;
        }

        public Builder securIdNewPinRequired(boolean securIdNewPinRequired) {
            this.securIdNewPinRequired = securIdNewPinRequired;
            return this;
        }

        public Builder gssContextId(String gssContextId) {
            this.gssContextId = gssContextId;
            return this;
        }

        public Builder gssServerLeg(byte[] gssServerLeg) {
            this.gssServerLeg = gssServerLeg;
            return this;
        }

        public Builder personUserEnabled(boolean personUserEnabled) {
            this.personUserEnabled = personUserEnabled;
            return this;
        }

        public Builder solutionUsername(String solutionUsername) {
            this.solutionUsername = solutionUsername;
            return this;
        }

        public Builder solutionUserCertSubjectDN(String solutionUserCertSubjectDN) {
            this.solutionUserCertSubjectDN = solutionUserCertSubjectDN;
            return this;
        }

        public Builder solutionUserEnabled(boolean solutionUserEnabled) {
            this.solutionUserEnabled = solutionUserEnabled;
            return this;
        }

        public Builder maxBearerTokenLifetime(long maxBearerTokenLifetime) {
            this.maxBearerTokenLifetime = maxBearerTokenLifetime;
            return this;
        }

        public Builder maxHoKTokenLifetime(long maxHoKTokenLifetime) {
            this.maxHoKTokenLifetime = maxHoKTokenLifetime;
            return this;
        }

        public Builder maxBearerRefreshTokenLifetime(long maxBearerRefreshTokenLifetime) {
            this.maxBearerRefreshTokenLifetime = maxBearerRefreshTokenLifetime;
            return this;
        }

        public Builder maxHoKRefreshTokenLifetime(long maxHoKRefreshTokenLifetime) {
            this.maxHoKRefreshTokenLifetime = maxHoKRefreshTokenLifetime;
            return this;
        }

        public Builder clockTolerance(long clockTolerance) {
            this.clockTolerance = clockTolerance;
            return this;
        }

        public Builder systemGroupMembership(Set<String> systemGroupMembership) {
            Validate.notNull(systemGroupMembership, "systemGroupMembership");
            this.systemGroupMembership = systemGroupMembership;
            return this;
        }

        public Builder groupMembership(Set<String> groupMembership) {
            Validate.notNull(groupMembership, "groupMembership");
            this.groupMembership = groupMembership;
            return this;
        }

        public Builder resourceServerMap(Map<String, ResourceServer> resourceServerMap) {
            Validate.notNull(resourceServerMap, "resourceServerMap");
            this.resourceServerMap = resourceServerMap;
            return this;
        }

        public MockIdmClient build() {
            return new MockIdmClient(this);
        }
    }
}
