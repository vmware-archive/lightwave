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
import java.util.Collection;
import java.util.EnumSet;
import java.util.List;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * @author Yehia Zayour
 */
public class IdmClient {
    private final CasIdmClient casIdmClient;

    // for MockIdmClient
    protected IdmClient() {
        this.casIdmClient = null;
    }

    public IdmClient(CasIdmClient casIdmClient) {
        Validate.notNull(casIdmClient, "casIdmClient");
        this.casIdmClient = casIdmClient;
    }

    public PrincipalId authenticate(String tenantName, String username, String password) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notEmpty(username, "username");
        Validate.notEmpty(password, "password");
        return this.casIdmClient.authenticate(tenantName, username, password);
    }

    public PrincipalId authenticate(String tenantName, X509Certificate[] tlsCertChain) throws Exception {
        throw new UnsupportedOperationException();
    }

    public GSSResult authenticate(String tenantName, String contextId, byte[] gssTicket) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notEmpty(contextId, "contextId");
        Validate.notNull(gssTicket, "gssTicket");
        return this.casIdmClient.authenticate(tenantName, contextId, gssTicket);
    }

    public boolean isActive(String tenantName, PrincipalId id) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notNull(id, "id");
        return this.casIdmClient.isActive(tenantName, id);
    }

    public PersonUser findPersonUser(String tenantName, PrincipalId id) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notNull(id, "id");
        return this.casIdmClient.findPersonUser(tenantName, id);
    }

    public SolutionUser findSolutionUserByCertDn(String tenantName, String subjectDN) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notEmpty(subjectDN, "subjectDN");
        return this.casIdmClient.findSolutionUserByCertDn(tenantName, subjectDN);
    }

    public Collection<AttributeValuePair> getAttributeValues(String tenantName, PrincipalId id, Collection<Attribute> attributes) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notNull(id, "id");
        Validate.notEmpty(attributes, "attributes");
        return this.casIdmClient.getAttributeValues(tenantName, id, attributes);
    }

    public Collection<IIdentityStoreData> getProviders(String tenantName, EnumSet<DomainType> domains) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notEmpty(domains, "domains");
        return this.casIdmClient.getProviders(tenantName, domains);
    }

    public Tenant getTenant(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getTenant(tenantName);
    }

    public String getDefaultTenant() throws Exception {
        return this.casIdmClient.getDefaultTenant();
    }

    public PrivateKey getTenantPrivateKey(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getTenantPrivateKey(tenantName);
    }

    public List<Certificate> getTenantCertificate(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getTenantCertificate(tenantName);
    }

    public String getOIDCEntityID(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getOIDCEntityID(tenantName);
    }

    public OIDCClient getOIDCClient(String tenantName, String clientId) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notEmpty(clientId, "clientId");
        return this.casIdmClient.getOIDCClient(tenantName, clientId);
    }

    public String getBrandName(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getBrandName(tenantName);
    }

    public String getLogonBannerTitle(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getLogonBannerTitle(tenantName);
    }

    public String getLogonBannerContent(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getLogonBannerContent(tenantName);
    }

    public boolean getLogonBannerCheckboxFlag(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getLogonBannerCheckboxFlag(tenantName);
    }

    public long getMaximumBearerTokenLifetime(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getMaximumBearerTokenLifetime(tenantName);
    }

    public long getMaximumHoKTokenLifetime(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getMaximumHoKTokenLifetime(tenantName);
    }

    public long getMaximumBearerRefreshTokenLifetime(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getMaximumBearerRefreshTokenLifetime(tenantName);
    }

    public long getMaximumHoKRefreshTokenLifetime(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getMaximumHoKRefreshTokenLifetime(tenantName);
    }

    public long getClockTolerance(String tenantName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        return this.casIdmClient.getClockTolerance(tenantName);
    }

    public boolean isMemberOfSystemGroup(String tenantName, PrincipalId id, String groupName) throws Exception {
        Validate.notEmpty(tenantName, "tenantName");
        Validate.notNull(id, "id");
        Validate.notEmpty(groupName, "groupName");
        return this.casIdmClient.isMemberOfSystemGroup(tenantName, id, groupName);
    }

    public String getServerSPN() throws Exception {
        return this.casIdmClient.getServerSPN();
    }

    public String getSsoMachineHostName() throws Exception {
        return this.casIdmClient.getSsoMachineHostName();
    }
}
