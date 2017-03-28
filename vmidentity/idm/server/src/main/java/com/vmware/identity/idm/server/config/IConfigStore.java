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

package com.vmware.identity.idm.server.config;

import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.util.Collection;
import java.util.EnumSet;
import java.util.List;

import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.ClientCertPolicy;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.server.config.directory.TenantAttributes;

/**
 * Created by IntelliJ IDEA.
 * User: snambakam
 * Date: 12/23/11
 * Time: 1:53 PM
 * To change this template use File | Settings | File Templates.
 */
public interface IConfigStore
{
    /**
     * Default delegation count
     */
    public static int  DEFAULT_DELEGATION_COUNT    = 10;
    /**
     * Default renew count
     */
    public static int  DEFAULT_RENEW_COUNT         = 10;
    /**
     * Default clock tolerance (+ or - 5 minutes)
     */
    public static long DEFAULT_CLOCK_TOLERANCE     = 10l * 60 * 1000; // 10min
    /**
     * Default maximum lifetime for bearer tokens (1 hour)
     */
    public static long DEFAULT_MAX_BEARER_LIFETIME = 1l * 60 * 60 * 1000; // 1h
    /**
     * Default maximum lifetime for HoK tokens (30 days)
     */
    public static long DEFAULT_MAX_HOK_LIFETIME    = 30l * 24 * 60 * 60 * 1000; // 30d
    /**
     * Default maximum lifetime for bearer refresh tokens (8 hours)
     */
    public static long DEFAULT_MAX_BEARER_REFRESH_TOKEN_LIFETIME = 8l * 60 * 60 * 1000; // 8h
    /**
     * Default maximum lifetime for HoK refresh tokens (30 days)
     */
    public static long DEFAULT_MAX_HOK_REFRESH_TOKEN_LIFETIME    = 30l * 24 * 60 * 60 * 1000; // 30d

    /*
     *  Tenant
     */
    public void addTenant(Tenant tenant, String adminAccountName, char[] adminPwd) throws Exception;

    public void deleteTenant(String name) throws Exception;

    public Tenant getTenant(String name) throws Exception;

    public Collection<String> getAllTenants() throws Exception;

    public void setTenant(Tenant tenant) throws Exception;

    public String getDefaultTenant() throws Exception;

    public String getSystemTenant() throws Exception;

    public void setDefaultTenant(String name) throws Exception;

    public void setSystemTenant(String name) throws Exception;

    public
    Collection<List<Certificate>>
    getTenantCertChains(String tenantName) throws Exception;

    public
    List<Certificate>
    getTenantCertificate(String tenantName) throws Exception;

    public
    void
    setTenantCredentials(
            String                  tenantName,
            Collection<Certificate> tenantCertificates,
            PrivateKey              tenantPrivateKey
            ) throws Exception;

    public
    void
    setTenantTrustedCertificateChain(
            String                  tenantName,
            Collection<Certificate> tenantCertificates
            ) throws Exception;

    public
    void
    addCertificateForSystemTenant(String tenantName, Certificate idmCert, CertificateType cerType) throws Exception;

    public
    void
    addCertificateForNonSystemTenant(String tenantName, Certificate idmCert, CertificateType cerType) throws Exception;

    public
    Collection<Certificate>
    getAllCertificatesForSystemTenant(String tenantName, CertificateType certType) throws Exception;

    public
    Collection<Certificate>
    getAllCertificatesForNonSystemTenant(String tenantName, CertificateType certType) throws Exception;

    public
    Collection<Certificate>
    getTrustedCertificatesForTenant(String tenantName) throws Exception;

    public
    Collection<Certificate>
    getStsIssuersCertificatesForTenant(String tenantName) throws Exception;

    public void
    deleteCertificateForSystemTenant(String tenantName, String fingerprint, CertificateType certType) throws Exception;

    public void
    deleteCertificateForNonSystemTenant(String tenantName, String fingerprint, CertificateType certType) throws Exception;

    public
    Collection<Attribute>
    getTenantAttributes(String tenantName) throws Exception;

    public
    void
    setTenantAttributes(
            String                tenantName,
            Collection<Attribute> attributes
            ) throws Exception;

    public PrivateKey getTenantPrivateKey(String tenantName) throws Exception;

    public TenantAttributes getTokenPolicyExt(String tenantName) throws Exception;

    public long getClockTolerance(String tenantName) throws Exception;

    public void setClockTolerance(String tenantName, long milliseconds)
            throws Exception;

    public int getDelegationCount(String tenantName) throws Exception;

    public void setDelegationCount(String tenantName, int delegationCount)
            throws Exception;

    public void setTenantIDPSelectionEnabled(String tenantName, boolean enableIDPSelection) throws Exception;

    public void setAlias(String tenantName, String alias) throws Exception;

    public int getRenewCount(String tenantName) throws Exception;

    public void setRenewCount(String tenantName, int renewCount)
            throws Exception;

    public long getMaximumBearerTokenLifetime(String tenantName)
            throws Exception;

    public void setMaximumBearerTokenLifetime(String tenantName,
            long maxLifetime) throws Exception;

    public long getMaximumHoKTokenLifetime(String tenantName) throws Exception;

    public void setMaximumHoKTokenLifetime(String tenantName, long maxLifetime)
            throws Exception;

    public long getMaximumBearerRefreshTokenLifetime(String tenantName)
            throws Exception;

    public void setMaximumBearerRefreshTokenLifetime(String tenantName,
            long maxLifetime) throws Exception;

    public long getMaximumHoKRefreshTokenLifetime(String tenantName) throws Exception;

    public void setMaximumHoKRefreshTokenLifetime(String tenantName, long maxLifetime)
            throws Exception;

    public String getSignatureAlgorithm(String tenantName) throws Exception;

    public String getBrandName(String tenantName) throws Exception;

    public String getLogonBannerContent(String tenantName) throws Exception;

    public String getLogonBannerTitle(String tenantname) throws Exception;

    public boolean getLogonBannerCheckboxFlag(String tenantname) throws Exception;

    public void setSignatureAlgorithm(String tenantName,
            String signatureAlgorithm) throws Exception;

    public PasswordExpiration getPasswordExpirationConfiguration(
            String tenantName) throws Exception;

    public void updatePasswordExpirationConfiguration(String tenantName,
            PasswordExpiration config) throws Exception;

    /*
     *  RelyingParty
     */
    public
    void
    addRelyingParty(String tenantName, RelyingParty rp) throws Exception;

    public
    void
    deleteRelyingParty(String tenantName, String rpName) throws Exception;

    public
    RelyingParty
    getRelyingParty(String tenantName, String rpName) throws Exception;

    public
    RelyingParty
    getRelyingPartyByUrl(String tenantName, String url) throws Exception;

    public
    void
    setRelyingParty(String tenantName, RelyingParty rp) throws Exception;

    public
    Collection<RelyingParty>
    getRelyingParties(String tenantName) throws Exception;

    /*
     *  OIDC Client
     */
    public void addOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception;

    public void deleteOIDCClient(String tenantName, String clientID) throws Exception;

    public OIDCClient getOIDCClient(String tenantName, String clientID) throws Exception;

    public void setOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception;

    public Collection<OIDCClient> getOIDCClients(String tenantName) throws Exception;

    /*
     *  Resource Server
     */
    public void addResourceServer(String tenantName, ResourceServer resourceServer) throws Exception;

    public void deleteResourceServer(String tenantName, String resourceServerName) throws Exception;

    public ResourceServer getResourceServer(String tenantName, String resourceServerName) throws Exception;

    public void setResourceServer(String tenantName, ResourceServer resourceServer) throws Exception;

    public Collection<ResourceServer> getResourceServers(String tenantName) throws Exception;

    /*
     *  IdentityProvider
     */
    public
    void
    addProvider(
            String             tenantName,
            IIdentityStoreData idsData ) throws Exception;

    public
    void
    deleteProvider(
            String tenantName,
            String providerName) throws Exception;

    public
    IIdentityStoreData
    getProvider(
            String  tenantName,
            String  providerName,
            boolean getInternalInfo
            ) throws Exception;

    public
    void
    setProvider(
            String             tenantName,
            IIdentityStoreData idsData ) throws Exception;

    public
    Collection<IIdentityStoreData>
    getProviders(
            String              tenantName,
            EnumSet<DomainType> domainTypes,
            boolean             getInternalInfo
            ) throws Exception;

    public
    Collection<String> getDefaultProviders(String tenantName) throws Exception;

    public
    void
    setDefaultProviders(
            String             tenantName,
            Collection<String> defaultProviders
            ) throws Exception;

    public
    void setEntityID(String tenantName, String entityID) throws Exception;

    public String getEntityID(String tenantName) throws Exception;

    /**
     * External IDP
     */
    public void registerExternalIdpConfig(String tenantName, IDPConfig idpConfig) throws Exception;

    public void removeExternalIdpConfig(String tenantName, String configEntityId) throws Exception;

    public Collection<IDPConfig> getExternalIDPConfigs(String tenantName) throws Exception;

    public void setBrandName(String tenantName, String brandName) throws Exception;

    public void setLogonBannerContent(String tenantName, String logonBannerContent) throws Exception;

    public void setAuthnTypesForProvider(String tenantName, String providerName, boolean password, boolean windows, boolean certificate,boolean rsaSecureID) throws Exception;

    public void setLogonBannerTitle(String tenantName, String logonBannerTitle) throws Exception;

    public void setLogonBannerCheckboxFlag(String tenantName, boolean logonBannerEnableCheckbox) throws Exception;

    public boolean registerUpnSuffixForDomain(String tenantName, String domainName,
         String upnSuffix) throws Exception;

   public boolean unregisterUpnSuffixForDomain(String tenantName,
         String domainName, String upnSuffix) throws Exception;

   public ClientCertPolicy getClientCertPolicy(String tenantName) throws Exception;

   public void setClientCertPolicy(String tenantName, ClientCertPolicy policy)
            throws Exception;

   public void setAuthnTypes(String tenantName, boolean password, boolean windows, boolean certificate, boolean rsaSecureID)
           throws Exception;

   public void setRsaAgentConfig(String tenantName, RSAAgentConfig rsaConfig) throws Exception;

   public RSAAgentConfig getRSAAgentConfig(String tenantName) throws Exception;

   public void addRSAInstanceInfo(String tenantName, RSAAMInstanceInfo instInfo) throws Exception;

   public void deleteRSAInstanceInfo(String tenantName, String siteID) throws Exception;
}
