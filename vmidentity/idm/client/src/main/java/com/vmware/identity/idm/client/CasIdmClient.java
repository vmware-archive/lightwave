/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package com.vmware.identity.idm.client;

import java.io.IOException;
import java.net.MalformedURLException;
import java.rmi.NotBoundException;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.XMLConstants;
import javax.xml.transform.Source;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.w3c.dom.Document;
import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ADIDSAlreadyExistException;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.Attribute;
import com.vmware.identity.idm.AttributeValuePair;
import com.vmware.identity.idm.AuthenticationType;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.CertificateInUseException;
import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.DomainManagerException;
import com.vmware.identity.idm.DomainTrustsInfo;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.DuplicateCertificateException;
import com.vmware.identity.idm.DuplicateProviderException;
import com.vmware.identity.idm.DuplicateTenantException;
import com.vmware.identity.idm.ExternalIDPCertChainInvalidTrustedPathException;
import com.vmware.identity.idm.ExternalIDPExtraneousCertsInCertChainException;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.GroupDetail;
import com.vmware.identity.idm.HostNotJoinedRequiredDomainException;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.IDMLoginException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityManager;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.IIdmServiceContext;
import com.vmware.identity.idm.IdmADDomainAccessDeniedException;
import com.vmware.identity.idm.IdmADDomainException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.InvalidPrincipalException;
import com.vmware.identity.idm.InvalidProviderException;
import com.vmware.identity.idm.LockoutPolicy;
import com.vmware.identity.idm.MemberAlreadyExistException;
import com.vmware.identity.idm.NoSuchCertificateException;
import com.vmware.identity.idm.NoSuchExternalIdpConfigException;
import com.vmware.identity.idm.NoSuchIdpException;
import com.vmware.identity.idm.NoSuchOIDCClientException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.OIDCClient;
import com.vmware.identity.idm.PasswordExpiration;
import com.vmware.identity.idm.PasswordExpiredException;
import com.vmware.identity.idm.PasswordPolicy;
import com.vmware.identity.idm.PasswordPolicyViolationException;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PersonUser;
import com.vmware.identity.idm.Principal;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.ResourceServer;
import com.vmware.identity.idm.SearchCriteria;
import com.vmware.identity.idm.SearchResult;
import com.vmware.identity.idm.SecurityDomain;
import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.SsoHealthStatsData;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.UserAccountLockedException;
import com.vmware.identity.idm.ValidateUtil;
import com.vmware.identity.idm.VmHostData;
import com.vmware.identity.idm.server.IdentityManager;
import com.vmware.identity.performanceSupport.IIdmAuthStat;
import com.vmware.identity.performanceSupport.IIdmAuthStatus;

/**
 * User: krishnag Date: 12/8/11 Time: 5:37 PM
 */
public class CasIdmClient
{

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(CasIdmClient.class);

    private static final String SAML_SCHEMA_FILE = "/saml-schema-metadata-2.0.xsd";
    private final IServiceContextProvider _serviceContextProvider;

    private Schema schema;
    private IIdentityManager identityManager;

    // a constructor temporarily added for unit test.
    protected CasIdmClient()
    {
        _serviceContextProvider = null;
    }

    /**
     * Constructs an instance of the IDM Client
     *
     * @param hostName IDM Server host location
     */
    public CasIdmClient(String hostName)
    {
        this(hostName, null);
    }

    /**
     * Constructs an instance of the IDM Client
     *
     * @param hostName IDM Server host location
     * @param serviceContextProvider Optional IServiceContextProvider (see IServiceContextProvider).
     */
    public CasIdmClient(String hostName, IServiceContextProvider serviceContextProvider)
    {
        this._serviceContextProvider = serviceContextProvider;
        try {
            identityManager = IdentityManager.getIdmInstance();
        } catch (IDMException e){
            log.error(String.format("Failed to initialize CasIdmClient: %s", e.getMessage()));
            throw new IllegalStateException("Failed to initialize CasIdmClient", e);
        }
     }

    /**
     * Adds the specified tenant with administrator credentials to the IDM configuration
     *
     * @param  tenant       Tenant to be added.
     * @param  adminAccountName name of the account with administrator privilege, cannot be null or empty
     * @param  adminPwd         cannot be null or zero length array
     * @throws DuplicateTenantException  - if tenant already exist.
     * @throws InvalidArgumentException    -- if the tenant name or
     *                                 other required parameter is null or empty
     */
    public void addTenant(Tenant tenant, String adminAccountName, char[] adminPwd) throws Exception
    {
        getService().addTenant(tenant, adminAccountName, adminPwd, this.getServiceContext());
    }

    /**
     * Retrieves the tenant information as referenced by the tenant name
     *
     * @param name Name of the tenant.
     *                   Required, non-null, non-empty, case-insensitive.
     *                   This allies to all API use tenant name.
     * @return     Tenant instance associated with the specified name.
     *             value will not be null.
     * @throws IDMException
     * @throws NoSuchTenantException    - tenant does not exist
     * @throws Exception
     */
    public Tenant getTenant(String name) throws Exception
    {
        return getService().getTenant(name, this.getServiceContext());
    }

    /**
     * Retrieves the system tenant name
     *
     * @param none
     * @return System tenant name - should not be null.
     * @throws Exception
     * @throws IDMException         - if fail to find system tenant.
     */
    public String getSystemTenant() throws Exception
    {
        return getService().getSystemTenant(this.getServiceContext());
    }

    /**
     * Retrieves a list of all existing tenant names.
     *
     * @return a collection of all tenant names. It expect at least one (system) tenant
     *             in the list.
     * @throws Exception
     * @throws IDMException          - if no tenant is found
     */
    public Collection<String> getAllTenants() throws Exception
    {
        return getService().getAllTenants(this.getServiceContext());
    }

    /**
     * Updates the tenant configuration with the specified information.
     * This action only allies to existing tenant.
     * @param tenant      Tenant information.
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist, user should call
     *                                      addTenant first for new tenant.
     * @throws InvalidArgumentException    -- if the tenant name or
     *                                 other required parameter is null or empty
     */
    public void setTenant(Tenant tenant) throws Exception
    {
        getService().setTenant(tenant, this.getServiceContext());
    }

    /**
     * Retrieves the currently configured signature algorithm for the tenant.
     * This string is used on login page.
     *
     * @param tenantName
     * @return brand name. Could be null value.
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name is null or empty
     */
    public String getBrandName(String tenantName) throws Exception
    {
        return getService().getBrandName(tenantName, this.getServiceContext());
    }

    /**
     * Sets the optional brand name for the tenant. This string is used on login
     * page. If the attribute is not set or set to null/empty string, it will
     * use vCenter login page with vCenter logo If a non-empty value is set, the
     * brand name string is displayed on the login page.
     *
     * No validation beyond name format checking.
     *
     * @param tenantName
     * @param brandName
     *            Could be null value or empty value.
     * @return
     * @throws Exception
     *             - if unable to connect the IDM server
     * @throws NoSuchTenantException
     *             - if no such tenant exist
     * @throws InvalidArgumentException
     *             -- if the tenant name or signatureAlgorithm is null or empty
     */
    public void setBrandName(String tenantName, String brandName) throws Exception
    {
        getService().setBrandName(tenantName, brandName, this.getServiceContext());
    }

    public String getLogonBannerTitle(String tenantName) throws Exception
    {
        return getService().getLogonBannerTitle(tenantName, this.getServiceContext());
    }

    public void setLogonBannerTitle(String tenantName, String logonBannerTitle) throws Exception
    {
        getService().setLogonBannerTitle(tenantName, logonBannerTitle, this.getServiceContext());
    }

    public String getLogonBannerContent(String tenantName) throws Exception
    {
        return getService().getLogonBannerContent(tenantName, this.getServiceContext());
    }

    public void setLogonBannerContent(String tenantName, String logonBannerContent) throws Exception
    {
        getService().setLogonBannerContent(tenantName, logonBannerContent, this.getServiceContext());
    }

    public boolean getLogonBannerCheckboxFlag(String tenantName) throws Exception
    {
        return getService().getLogonBannerCheckboxFlag(tenantName, this.getServiceContext());
    }

    public void setLogonBannerCheckboxFlag(String tenantName, boolean enableLogonBannerCheckbox) throws Exception
    {
        getService().setLogonBannerCheckboxFlag(tenantName, enableLogonBannerCheckbox, this.getServiceContext());
    }

    public void disableLogonBanner(String tenantName) throws Exception {
        // set title and content to null to disable logon banner on websso
        setLogonBannerTitle(tenantName, null);
        setLogonBannerContent(tenantName, null);
    }

    /**
     * Operation for setting authentication policy of an identity provider
     *
     * @param providerName Name of identity provider on which authentication policy to be set
     * @param policy Authentication policy to set
     * @throws Exception
     */
    public void setAuthnPolicyForProvider(String tenantName, String providerName, AuthnPolicy policy) throws Exception {
        getService().setAuthnPolicyForProvider(tenantName, providerName, policy, this.getServiceContext());
    }

    /**
     * Retrieves the currently configured signature algorithm for the tenant.
     *
     * @param tenantName
     * @return algorithm name. Could be null value.
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name is null or empty
     */
    public String getTenantSignatureAlgorithm(String tenantName) throws Exception
    {
        return getService().getTenantSignatureAlgorithm(tenantName, this.getServiceContext());
    }

    /**
     * Sets the signature algorithm used for signing tokens from STS for that tenant.
     * No validation beyond name format checking.
     * @param tenantName
     * @return
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name or
     *                                 signatureAlgorithm is null or empty
     */
    public void setTenantSignatureAlgorithm(String tenantName, String signatureAlgorithm) throws Exception
    {
        getService().setTenantSignatureAlgorithm(tenantName, signatureAlgorithm, this.getServiceContext());
    }

    /**
     * Retrieves idp selection flag for tenant.
     *
     * @param tenantName name of tenant
     * @return true if idp selection is enabled
     * @throws Exception
     */
    public boolean isTenantIDPSelectionEnabled(String tenantName) throws Exception
    {
        return getService().isTenantIDPSelectionEnabled(tenantName, this.getServiceContext());
    }

    /**
     * Sets idp selection flag for tenant.
     *
     * @param tenantName name of tenant.
     * @param enableIDPSelection true if enabling idp selection.
     * @throws Exception
     */
    public void setTenantIDPSelectionEnabled(String tenantName, boolean enableIDPSelection) throws Exception
    {
        getService().setTenantIDPSelectionEnabled(tenantName, enableIDPSelection, this.getServiceContext());
    }

    /**
     * Adds a relying party to a Tenant's configuration. Ignore if it already exist.
     *
     * @param tenantName     Name of the tenant
     *                   Required, non-null, non-empty, case-insensitive.
     * @param rp             Relying Party information
     * @throws IDMException
     * @throws Exception

     */
    public
    void
    addRelyingParty(String tenantName, RelyingParty rp) throws Exception
    {
        getService().addRelyingParty(tenantName, rp, this.getServiceContext());
    }

    /**
     * Removes a relying party from the tenant's configuration
     *
     * @param tenantName  Name of tenant
     *                   Required, non-null, non-empty, case-insensitive.
     * @param rpName      Name of relying party.
     *                   Required, non-null, non-empty, case-insensitive.
     * @throws Exception
     * @throws NoSuchRelyingPartyException.   if the relying party can't be found.

     */
    public
    void
    deleteRelyingParty(String tenantName, String rpName)   throws Exception
    {
        getService().deleteRelyingParty(tenantName, rpName, this.getServiceContext());
    }

    /**
     * Retrieves a relying party from the tenant's configuration
     *
     * @param tenantName Name of tenant
     *                   Required, non-null, non-empty, case-insensitive.
     * @param rpName     Name of relying party
     *                   Required, non-null, non-empty, case-insensitive.
     * @return           Relying party information. Null if none were found.
     * @throws IDMException
     * @throws Exception

     */
    public
    RelyingParty
    getRelyingParty(String tenantName, String rpName) throws Exception
    {
        return getService().getRelyingParty(tenantName, rpName, this.getServiceContext());
    }

    /**
     * Retrieves a relying party from the tenant's configuration
     *
     * @param tenantName Name of tenant
     *                   Required, non-null, non-empty, case insensitive.
     * @param url        Relying party's URL
     * @return           Relying party information. Null if none were found
     * @throws IDMException
     * @throws Exception

     */
    public
    RelyingParty
    getRelyingPartyByUrl(String tenantName, String url) throws Exception
    {
        return getService().getRelyingPartyByUrl(tenantName, url, this.getServiceContext());
    }

    /**
     * Update relying party information
     *
     * @param tenantName Name of tenant. Required, non-null, non-empty, case insensitive.
     * @param rp         Relying party information to be updated
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchRElyingParty
     */
    public
    void setRelyingParty(String tenantName, RelyingParty rp) throws Exception
    {
        getService().setRelyingParty(tenantName, rp, this.getServiceContext());
    }

    /**
     * Retrieves the collection of relying parties configured for the tenant.
     *
     * @param tenantName Name of the tenant
     *                   Required, non-null, non-empty, case-insensitive.
     * @return           The collection of relying parties.
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws InvalidArgumentException    -- if the tenant name is illegal.
     */
    public
    Collection<RelyingParty>
    getRelyingParties(String tenantName) throws Exception
    {
        return getService().getRelyingParties(tenantName, this.getServiceContext());
    }

    /**
     * Adds a OIDC client to a Tenant's configuration. Ignore if it already
     * exist.
     *
     * @param tenantName
     *            Name of the tenant Required, non-null, non-empty,
     *            case-insensitive.
     * @param oidcClient
     *            OIDC client information
     * @throws IDMException
     * @throws Exception
     *             - if unable to connect the IDM server
     */
    public void addOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception {
        getService().addOIDCClient(tenantName, oidcClient, this.getServiceContext());
    }

    /**
     * Removes a OIDC client from the tenant's configuration
     *
     * @param tenantName
     *            Name of tenant Required, non-null, non-empty,
     *            case-insensitive.
     * @param clientId
     *            Name of OIDC client. Required, non-null, non-empty,
     *            case-insensitive.
     * @throws Exception
     * @throws NoSuchRelyingPartyException. if the OIDC client can't be found.
     *             - if unable to connect the IDM server
     */
    public void deleteOIDCClient(String tenantName, String clientId) throws Exception {
        getService().deleteOIDCClient(tenantName, clientId, this.getServiceContext());
    }

    /**
     * Retrieves a OIDC client from the tenant's configuration
     *
     * @param tenantName
     *            Name of tenant Required, non-null, non-empty,
     *            case-insensitive.
     * @param clientId
     *            Name of OIDC client Required, non-null, non-empty,
     *            case-insensitive.
     * @return OIDC client information. Null if none were found.
     * @throws IDMException
     * @throws Exception
     *             - if unable to connect the IDM server
     */
    public OIDCClient getOIDCClient(String tenantName, String clientId) throws Exception {
        return getService().getOIDCClient(tenantName, clientId, this.getServiceContext());
    }

    /**
     * Update OIDC client information
     *
     * @param tenantName
     *            Name of tenant. Required, non-null, non-empty, case
     *            insensitive.
     * @param oidcClient
     *            OIDC client information to be updated
     * @throws IDMException
     * @throws Exception
     *             - if unable to connect the IDM server
     * @throws NoSuchOIDCClientException
     */
    public void setOIDCClient(String tenantName, OIDCClient oidcClient) throws Exception {
        getService().setOIDCClient(tenantName, oidcClient, this.getServiceContext());
    }

    /**
     * Retrieves the collection of OIDC clients configured for the tenant.
     *
     * @param tenantName
     *            Name of the tenant Required, non-null, non-empty,
     *            case-insensitive.
     * @return The collection of OIDC clients.
     * @throws IDMException
     * @throws Exception
     *             - if unable to connect the IDM server
     * @throws NoSuchTenantException
     *             - if no such tenant exist
     * @throws InvalidArgumentException
     *             -- if the tenant name is illegal.
     */
    public Collection<OIDCClient> getOIDCClients(String tenantName) throws Exception {
        return getService().getOIDCClients(tenantName, this.getServiceContext());
    }

    public void addResourceServer(String tenantName, ResourceServer resourceServer) throws Exception {
        getService().addResourceServer(tenantName, resourceServer, this.getServiceContext());
    }

    public void deleteResourceServer(String tenantName, String resourceServerName) throws Exception {
        getService().deleteResourceServer(tenantName, resourceServerName, this.getServiceContext());
    }

    public ResourceServer getResourceServer(String tenantName, String resourceServerName) throws Exception {
        return getService().getResourceServer(tenantName, resourceServerName, this.getServiceContext());
    }

    public void setResourceServer(String tenantName, ResourceServer resourceServer) throws Exception {
        getService().setResourceServer(tenantName, resourceServer, this.getServiceContext());
    }

    public Collection<ResourceServer> getResourceServers(String tenantName) throws Exception {
        return getService().getResourceServers(tenantName, this.getServiceContext());
    }

    /**
     * Create the external IDP configuration for the tenant. If the
     * configuration already exists, it will be updated accordingly.
     *
     * @param tenantName
     *            Cannot be null or empty
     * @param idpConfig
     *            Cannot be null
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public void setExternalIdpConfig(String tenantName, IDPConfig idpConfig)
            throws Exception
    {
        getService().setExternalIdpForTenant(tenantName, idpConfig, this.getServiceContext());
    }

    /**
     * Remove the specified external IDP configuration from tenant
     *
     * @param tenantName
     *            Cannot be null or empty.
     * @param configEntityId
     *            Cannot not null or empty
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     * @throws NoSuchExternalIdpConfigException
     *             configuration with specified entity Id not found
     */
    public void removeExternalIdpConfig(String tenantName, String configEntityId)
            throws Exception
    {
        getService().removeExternalIdpForTenant(tenantName, configEntityId, this.getServiceContext());
    }

    /**
     * Remove the specified external IDP configuration from tenant
     *
     * @param tenantName
     *            Cannot be null or empty.
     * @param configEntityId
     *            Cannot not null or empty
     * @param removeJitUsers
     *            true if remove all Jit users created for the external IDP
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     * @throws NoSuchExternalIdpConfigException
     *             configuration with specified entity Id not found
     */
    public void removeExternalIdpConfig(String tenantName, String configEntityId, boolean removeJitUsers)
            throws Exception
    {
        getService().removeExternalIdpForTenant(tenantName, configEntityId, removeJitUsers, this.getServiceContext());
    }

    /**
     * Get all external IDP configurations for the specified tenant
     *
     * @param tenantName
     *            Cannot be null or empty
     * @return Collection of external IDP configuration for the tenant, empty if
     *         none are found
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public Collection<IDPConfig> getAllExternalIdpConfig(String tenantName)
            throws Exception
    {
        return getService().getAllExternalIdpsForTenant(tenantName, this.getServiceContext());
    }

    /**
     * Get trust anchors for all the IDP configurations of the specified tenant.
     *
     * @param tenantName cannot be null or empty
     * @return set of trust anchors for ExteralIDP.
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     */
    @Deprecated
    public Collection<X509Certificate> getAllExternalIdpConfigTrustAnchors(
            String tenantName) throws Exception
    {
        Collection<X509Certificate> trustAnchors = new HashSet<X509Certificate>();
        for (IDPConfig config : getService().getAllExternalIdpsForTenant(
                tenantName, this.getServiceContext()))
        {
            trustAnchors.add(config.getSigningCertificateChain().get(
                    config.getSigningCertificateChain().size() - 1));
        }
        return Collections.unmodifiableCollection(trustAnchors);
    }

    /**
     * Get issuers certificate for all the IDP configurations of the specified tenant.
     *
     * @param tenantName cannot be null or empty
     * @return set of issuers certificates for ExteralIDP.
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public Collection<X509Certificate> getAllExternalIdpIssuersCertificates(
            String tenantName) throws Exception
    {
        Collection<X509Certificate> trustAnchors = new HashSet<X509Certificate>();
        for (IDPConfig config : getService().getAllExternalIdpsForTenant(
                tenantName, this.getServiceContext()))
        {
            trustAnchors.add(config.getSigningCertificateChain().get(0));
        }
        return Collections.unmodifiableCollection(trustAnchors);
    }

    /**
     * Retrieve external IDP configuration per tenant name and entityId.
     *
     * @param tenantName
     *            Cannot be null or empty;
     * @param configEntityId
     *            Cannot be null or empty;
     * @return IdpConfiguration with the matching entityId, null if not found
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public IDPConfig getExternalIdpConfigForTenant(String tenantName,
            String configEntityId) throws Exception
    {
        return getService().getExternalIdpForTenant(tenantName, configEntityId, this.getServiceContext());
    }

    private boolean isJitEnabledForExternalIdp(String tenantName, String entityId) throws Exception {
        IDPConfig idpConfig = getService().getExternalIdpForTenant(tenantName, entityId,
               this.getServiceContext());
        if (idpConfig == null) {
            throw new NoSuchIdpException(String.format("Failed to retrieve Jit attribute. "
                    + "External IDP %s is not found.", entityId));
        }
        return idpConfig.getJitAttribute();
    }

    /**
     * Retrieve collection of external IDP configurations with matching URL
     *
     * @param tenantName
     *            Cannot be null or empty.
     * @param urlStr
     *            string used to match SSO / SLO locations. Cannot be null or
     *            empty.
     * @return List of idpConfiguration with the matching URL location, empty if
     *         not found. Note the result matches either the SSO service
     *         end-points or SLO service end-points
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             argument validation failed
     * @throws NoSuchTenantException
     *             tenant not found
     */
    public Collection<IDPConfig> getExternalIdpConfigForTenantByUrl(
            String tenantName, String urlStr) throws Exception
    {
        return getService().getExternalIdpForTenantByUrl(tenantName, urlStr, this.getServiceContext());
    }

   /**
    * Adds an identity provider to the tenant's configuration
    *
    * @param tenantName
    *           Name of tenant. Required, non-null, non-empty, case insensitive.
    * @param idp
    *           Identity Provider information, required.
    * @throws Exception
    *            overall -- when finally addProvider() fails;
    * @throws NoSuchTenantException
    *            tenant not exist.
    * @throws InvalidArgumentException
    *            when the sanity check of input failure, such as null or empty
    *            value was used;
    * @throws InvalidPrincipalException
    *            failed generic probe connectivity test;
    * @throws DuplicateProviderException
    *            provider already exist;
    * @throws HostNotJoinedRequiredDomainException
    *            Host is not joined to the required domain when trying to
    *            register a native AD provider.
    * @throws DomainManagerException
    *            DomainManager native API invocation error when trying to
    *            register a native AD provider.
    * @throws ADIDSAlreadyExistException
    *            There is already a native AD IDS or LDAP AD AD IDS registered
    *            when trying to register a native AD provider;
    *            Or, nativeAD already exist when adding ADLdap.
    * @throws InvalidProviderException
    *              Invalid configuration for provider, such as invalid users baseDN or
    *              invalid groups baseDN.
    */
    public void addProvider(String tenantName, IIdentityStoreData idp)
         throws Exception
    {
        getService().addProvider(tenantName, idp, this.getServiceContext());
    }

    /**
     * Retrieves an identity provider from the tenant's configuration
     *
     * @param tenantName   Name of tenant. Required, non-null, non-empty, case insensitive.
     * @param ProviderName Name of Identity Provider. Required.
     * @return             Identity Provider information, null if not found.
     * @throws Exception
     * @throws NoSuchTenantException
     *            tenant not exist.
     * @throws InvalidArgumentException - one or more input argument is invalid.
     */
    public
    IIdentityStoreData
    getProvider(String tenantName, String ProviderName) throws Exception
    {
        return getService().getProvider(tenantName, ProviderName, this.getServiceContext());
    }

    public
    IIdentityStoreData getProviderWithInternalInfo(String tenantName, String ProviderName) throws Exception
    {
        return getService().getProviderWithInternalInfo(tenantName, ProviderName, this.getServiceContext());
    }

    /**
     * Removes an identity provider from the tenant's configuration
     * No action if the provider does not exist.
     *
     * It invokes the overloaded function specifying the option to clear the tenant info cache in IDM.
     *
     * @param tenantName   Name of tenant. Required, non-null, non-empty, case insensitive.
     * @param providerName Name of Identity Provider.
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException
     * @throws InvalidArgumentException  - result of illegal input.
     */
    public
    void
    deleteProvider(String tenantName, String providerName) throws Exception
    {
        getService().deleteProvider(tenantName, providerName, this.getServiceContext());
    }

    /**
     * Updates the native AD provider information in the tenant's configuration
     *
     * @param tenantName
     *            Name of tenant. Required, non-null, non-empty, case
     *            insensitive.
     * @param idpd
     *            Identity provider information
     * @throws NoSuchTenantException
     *             tenant not exist.
     * @throws InvalidArgumentException
     *             when the sanity check of input failure, such as null or empty
     *             value was used;
     * @throws InvalidPrincipalException
     *             failed generic probe connectivity test;
     * @throws DuplicateProviderException
     *             provider of the same name already exist;
     * @throws DomainManagerException
     *             DomainManager native API invocation error when trying to
     *             update a native AD provider.
     */
    public void setNativeADProvider(String tenantName, IIdentityStoreData idpd)
            throws Exception
    {
        getService().setNativeADProvider(tenantName, idpd,
                this.getServiceContext());
    }

    /**
     * Updates the LDAP provider information in the tenant's configuration
     *
     * @param tenantName Name of tenant. Required, non-null, non-empty, case insensitive.
     * @param idpd       Identity provider information
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException
     * @throws InvalidArgumentException
     *            when the sanity check of input failure, such as null or empty
     *            value was used;
     * @throws IDMLoginException
     *            failed generic probe connectivity test;
     * @throws InvalidProviderException
     *              Invalid configuration for provider, such as invalid users baseDN or groups baseDN.
     */
    public
    void
    setProvider(String tenantName, IIdentityStoreData idpd) throws Exception
    {
        getService().setProvider(tenantName, idpd, this.getServiceContext());
    }

    /**
     * Retrieves all providers configured for a tenant, system, local, external.
     *
     * @param tenantName Name of tenant, non-null, non-empty, required.
     * @return           Collection of identity providers, Empty collect if no provider found
     * @throws Exception
     * @throws InvalidArgumentException
     * @throws NoSuchTenantException
     */
    public
    Collection<IIdentityStoreData>
    getProviders(String tenantName) throws Exception
    {
        return getService().getProviders(tenantName, this.getServiceContext());
    }

    /**
     * Retrieves all the providers matching the set of specified domain types
     * in a tenant's configuration
     *
     * @param tenantName  Name of tenant, non-null non-empty, required
     * @param domains    Set of domain types. Required.
     * @return           Collection of identity providers, Empty collect if no provider found
     * @throws Exception
     * @throws InvalidArgumentException
     * @throws NoSuchTenantException
     * @see    DomainType
     */
    public
    Collection<IIdentityStoreData>
    getProviders(
            String              tenantName,
            EnumSet<DomainType> domains
            ) throws Exception
            {
        return getService().getProviders(tenantName, domains, this.getServiceContext());
            }

    /**
     * Retrieves the security domains supported by a provider
     *
     * @param tenantName    Name of tenant, non-null non-empty, required
     * @param providerName  Name of identity provider
     * @return Collection of domains, Empty collection if no provider found
     * @throws Exception
     */
    public
    Collection<SecurityDomain>
    getSecurityDomains(
            String tenantName,
            String providerName
            ) throws Exception
    {
        return getService().getSecurityDomains(tenantName, providerName, this.getServiceContext());
    }

    /**
     * Checks the connectivity to an identity provider
     *
     * @param tenantName  Name of tenant, non-null non-empty, required
     * @param providerUri Location of identity provider. non-null non-empty, required
     * @param authType    Type of authentication. required.
     *                 currently supports AuthenticationType.PASSWORD only.
     * @param userName    Login identifier. non-null, required
     * @param pwd         Password    non-null non-empty, required
     * @param certificates Trusted certificates used for ldaps connection. The remote server's SSL certificate must be
     *           verifiable using the list of certificates (or the
     *           connection will fail).
     * @throws Exception
     * @throws InvalidPrincipalException username is empty
     * @throws IDMLoginException. If one or more of the input argument is illegal.
     *                                 Or URI syntax is incorrect.
     * @throws IDMException     if no connection
     * @see AuthenticationType
     */
    public
    void
    probeProviderConnectivity(
            String             tenantName,
            String             providerUri,
            AuthenticationType authType,
            String             userName,
            String             pwd,
            Collection<X509Certificate> certificates
            ) throws Exception
            {
        getService().probeProviderConnectivity(
                tenantName,
                providerUri,
                authType,
                userName,
                pwd, certificates, this.getServiceContext());
    }

    /**
     * Checks the connectivity to an identity provider
     *
     * @param tenant
     * @param idsData
     * @throws Exception
     */
    public
    void
    probeProviderConnectivity(String tenant, IIdentityStoreData idsData) throws Exception
    {
        getService().probeProviderConnectivity(tenant, idsData, this.getServiceContext());
    }

    /**
     * Checks the connectivity to an identity provider with certificate validation even when server validation is disabled
     *
     * @param tenantName  Name of tenant, non-null non-empty, required
     * @param providerUri Location of identity provider. non-null non-empty, required
     * @param authType    Type of authentication. required.
     *                 currently supports AuthenticationType.PASSWORD only.
     * @param userName    Login identifier. non-null, required
     * @param pwd         Password    non-null non-empty, required
     * @param certificates Trusted certificates used for ldaps connection. The remote server's SSL certificate must be
     *           verifiable using the list of certificates (or the
     *           connection will fail).
     * @throws Exception
     * @throws InvalidPrincipalException username is empty
     * @throws IDMLoginException. If one or more of the input argument is illegal.
     *                                 Or URI syntax is incorrect.
     * @throws IDMException     if no connection
     * @see AuthenticationType
     */
    public
    void
    probeProviderConnectivityWithCertValidation(
            String             tenantName,
            String             providerUri,
            AuthenticationType authType,
            String             userName,
            String             pwd,
            Collection<X509Certificate> certificates
            ) throws Exception
            {
        getService().probeProviderConnectivityWithCertValidation(
                tenantName,
                providerUri,
                authType,
                userName,
                pwd,
                certificates,
                this.getServiceContext());
    }

    /**
     * Retrieves all the default identity providers in the tenant's
     * configuration.
     *
     * At the current time, any tenant may have at most one default identity
     * provider.
     *
     * The default Identity provider's domain is used for authenticating
     * principals when the authentication request does not specify the
     * domain name.
     *
     * @param tenantName    Name of tenant
     * @return              Collection of default Identity providers.
     * @throws IDMException
     * @throws Exception
     */
    public
    Collection<String> getDefaultProviders(String tenantName) throws Exception
    {
        return getService().getDefaultProviders(tenantName, this.getServiceContext());
    }

    /**
     * Sets a collection of Identity Providers as being default for a tenant.
     *
     * At the current time, any tenant may have at most one default identity
     * provider.
     *
     * The default Identity provider's domain is used for authenticating
     * principals when the authentication request does not specify the
     * domain name.
     *
     * @param tenantName       Name of tenant
     * @param defaultProviders Collection of Identity providers
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    setDefaultProviders(
        String             tenantName,
        Collection<String> defaultProviders
        ) throws Exception
    {
        getService().setDefaultProviders(tenantName, defaultProviders, this.getServiceContext());
    }

    /**
     * Sets a certificate chain and private key for the tenant.
     *
     * This is the credential used to sign SAML tokens generated for the
     * tenant's clients.
     *
     * @param tenantName         Name of tenant
     * @param tenantCertificates Certificate chain
     * @param tenantPrivateKey   Private key
     * @throws IllegalArgumentException
     *         when either privateKey or certification chain fails in validation
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    setTenantCredentials(
        String                  tenantName,
        Collection<Certificate> tenantCertificates,
        PrivateKey              tenantPrivateKey
        ) throws Exception
    {
        getService().setTenantCredentials(
                tenantName,
                tenantCertificates,
                tenantPrivateKey, this.getServiceContext());
    }

    /**
     * Add a trusted certificate chain for the tenant.
     *
     * This added certificate chain is used to as part of the certChain set to
     * validate incoming tokens
     *
     * @param tenantName         Name of tenant
     * @param tenantCertificates Certificate chain
     * @throws IllegalArgumentException
     *         when either privateKey or certification chain fails in validation
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    setTenantTrustedCertificateChain(
        String                  tenantName,
        Collection<Certificate> tenantCertificates
    ) throws Exception
    {
        getService().setTenantTrustedCertificateChain(
                tenantName,
                tenantCertificates, this.getServiceContext());
    }

    /**
     * Retrieves the private key set for the tenant.
     *
     * This is the credential used to sign SAML tokens generated for the
     * tenant's clients.
     *
     * @param tenantName Name of tenant
     * @return           Private key
     * @throws IDMException
     * @throws Exception
     */
    public PrivateKey getTenantPrivateKey(String tenantName) throws Exception
    {
        return getService().getTenantPrivateKey(tenantName, this.getServiceContext());
    }

    /**
     * Removes the tenant from the IDM server's configuration
     *
     * @param name       Name of tenant
     * @throws IDMException
     * @throws Exception
     */
    public void deleteTenant(String name) throws Exception
    {
        getService().deleteTenant(name, this.getServiceContext());
    }

    /**
     * Retrieves the default tenant's name
     *
     * The feature of having a default tenant is used by clients that are not
     * aware of the multi-tenant nature of the IDM server.
     *
     * @return Name of the tenant set as default
     * @throws IDMException
     * @throws Exception
     */
    public String getDefaultTenant() throws Exception
    {
        return getService().getDefaultTenant(this.getServiceContext());
    }

    /**
     * Sets a default tenant.
     *
     * The feature of having a default tenant is used by clients that are not
     * aware of the multi-tenant nature of the IDM server.
     *
     * @param name  Name of tenant
     * @throws IDMException
     * @throws Exception
     */
    public void setDefaultTenant(String name) throws Exception
    {
        getService().setDefaultTenant(name, this.getServiceContext());
    }

    /**
     * Authenticates a principal using the password specified
     *
     * The principal is expected to be managed by one of the Identity providers
     * configured in the tenant.
     *
     * @param tenantName Name of tenant. non-null non-empty, required
     * @param principal  User principal to be authenticated. non-null non-empty, required
     * @param password   Password.  non-null non-empty, required
     * @return Normalized principal if successfully authenticated.
     * @throws IDMLoginException when authentication failed.
     * @throws PasswordExpiredException when authentication failed due to
     *         expired password.
     * @throws UserAccountLockedException when authentication failed due to
     *         locked user account.
     * @throws Exception
     */
    public
    PrincipalId
    authenticate(
        String tenantName,
        String principal,
        String password
        ) throws Exception
    {
        return getService().authenticate(tenantName, principal, password, this.getServiceContext());
    }

    /**
     * Authenticates using a ticket obtained through GSSAPI.
     *
     * @param tenantName Name of tenant
     * @param gssTicket  Ticket (byte array) obtained through GSSAPI
     * @return GSSAPI result obtained as a result of validating/accepting the
     *         ticket.
     * @throws IDMLoginException when authentication failed.
     * @throws Exception
     * @see GSSResult
     */
    public
    GSSResult
    authenticate(String tenantName, String contextId, byte[] gssTicket) throws Exception
    {
        return getService().authenticate(tenantName, contextId, gssTicket, this.getServiceContext());
    }

    /**
     * Authenticates using TLS Client certificate. The certifcate is from
     * TLS validated with trusted CA - by TC or other means. Thus the client machine
     * has the private key installed matching to the public key in the target certificate.
     *
     * This authenticate function will conduct the following
     * 1. CRL/OSCP checking
     * 2. OID validation based on tenant authentication policy
     * 3. Subject Mapped with an identity provider
     *
     * @param tenantName Name of tenant
     * @param tlsCertChain
     * @param hint         optional user name hint
     * @return Normalized principal if successfully authenticated.
     * @throws MalformedURLException
     * @throws IDMLoginException when authentication failed.
     * @throws UserAccountLockedException when authentication failed due to
     *         locked user account.
     * @throws Exception
     */
	public PrincipalId authenticate(String tenantName,
			X509Certificate[] tlsCertChain, String hint) throws Exception {
        return getService().authenticate(tenantName, tlsCertChain, hint, this.getServiceContext());

    }

    /**
     * Authenticates a principal using the password specified
     *
     * The principal is expected to be managed by one of the Identity providers
     * configured in the tenant.
     *
     * @param tenantName
     *            Name of tenant. non-null non-empty, required
     * @param principal
     *            User principal to be authenticated. non-null non-empty,
     *            required
     * @param sessionId
     *            index to secureID session. could be null for new request.
     * @param passcode
     *            Passcode. non-null non-empty, required
     * @param passcode2
     * @return RSAAMResult which includes normalized principal if successfully authenticated.
     * @throws IDMLoginException
     *             when authentication failed.
     * @throws IDMSecureIDNextPinException
     *             in next
     * @throws Exception
     */
    public RSAAMResult authenticateRsaSecurId(
        String tenantName,
        String sessionId,
        String principal,
        String passcode
        ) throws Exception
    {
        return getService().authenticateRsaSecurId(tenantName, sessionId,
                principal, passcode, this.getServiceContext());
    }

    /**
     * Retrieve default information/attribute definitions about a User or
     * Service Principal in the given tenant/domain.
     *
     * @param tenantName Name of tenant
     * @return Collection of supported attribute definitions
     * @throws IDMException
     * @throws Exception
     */
    public
    Collection<Attribute>
    getAttributeDefinitions(String tenantName) throws Exception
    {
        return getService().getAttributeDefinitions(tenantName, this.getServiceContext());
    }

    /**
     * Retrieve specific information/attributes about a User or Service
     * Principal
     *
     * @param tenantName Name of tenant
     * @param principal  User or Service Principal about which attribute
     *                   information is being queried.
     * @param attributes Name of user or service principal attributes whose
     *                   values are to be queried.
     * @return Collection of attribute/value pairs
     * @throws IDMException
     * @throws Exception
     */
    public
    Collection<AttributeValuePair>
    getAttributeValues(
        String                tenantName,
        PrincipalId           principal,
        Collection<Attribute> attributes
        ) throws Exception
    {
        return getService().getAttributeValues(tenantName, principal, attributes, this.getServiceContext());
    }

    /**
     * Query if the user or service principal is active (not disabled).
     *
     * @param tenantName  Name of tenant, required.non-null non-empty

     * @param principalId User or service principal
     * @return true if the user or service principal is active and false
     *         otherwise.
     * @throws InvalidPrincipalException
     * @throws Exception
     */
    public
    boolean
    isActive(String tenantName, PrincipalId principalId) throws Exception
    {
        return getService().IsActive(tenantName, principalId, this.getServiceContext());
    }

    /**
     * Retrieves the currently configured clock tolerance for the tenant
     *
     * The clock tolerance is the time difference between the client and server
     * systems.
     *
     * @param tenantName Name of tenant
     * @return value of current clock tolerance in milliseconds
     * @throws Exception
     * @throws NoSuchTenantException       - when target tenant does not exist.
     *           InvalidaArgumentException   - for illegal input value.
     */
    public long getClockTolerance(String tenantName) throws Exception
    {
        return getService().getClockTolerance(tenantName, this.getServiceContext());
    }

    /**
     * Sets the clock tolerance on a tenant configuration
     *
     * The clock tolerance is the time difference between the client and server
     * systems.
     *
     * @param tenantName   Name of tenant, non-null, non-empty.
     * @param milliseconds Clock tolerance in milli seconds. non-negative.
     * @throws Exception
     * @throws NoSuchTenantException       - when target tenant does not exist.
     *           InvalidaArgumentException   - for illegal input value.
     */
    public
    void
    setClockTolerance(String tenantName, long milliseconds) throws Exception
    {
        getService().setClockTolerance(tenantName, milliseconds, this.getServiceContext());
    }

    /**
     * Sets the entity identifier associated with the tenant
     *
     * @param tenantName Name of tenant
     * @param entityID   Entity identifier
     * @throws IDMException
     * @throws Exception
     */
    public
    void setEntityID(String tenantName, String entityID) throws Exception
    {
        getService().setEntityID(tenantName, entityID, this.getServiceContext());
    }

    /**
     * Retrieves the entity identifier associated with the tenant
     *
     * @param tenantName Name of tenant
     * @return Entity identifier associated with the tenant
     * @throws IDMException
     * @throws Exception
     */
    public String getEntityID(String tenantName) throws Exception
    {
        return getService().getEntityID(tenantName, this.getServiceContext());
    }

    /**
     * Retrieves the alias associated with the entityId of external identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @param entityId entity id of provider, cannot be null or emtpty.
     * @return alias, null if alias is not set
     * @throws Exception
     */
    public String getExternalIDPAlias(String tenantName, String entityId) throws Exception
    {
        return getService().getExternalIDPAlias(tenantName, entityId, this.getServiceContext());
    }

    /**
     * Sets the alias associated with the entityId of external identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @param entityId entity id of provider, cannot be null or empty.
     * @param alias alias to be set, can be null or empty.
     * @throws Exception
     */
    public void setExternalIDPAlias(String tenantName, String entityId, String alias) throws Exception
    {
        getService().setExternalIDPAlias(tenantName, entityId, alias, this.getServiceContext());
    }

    /**
     * Retrieves the alias associated with the entityId of local identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @return alias, null if alias is not set
     * @throws Exception
     */
    public String getLocalIDPAlias(String tenantName) throws Exception
    {
        return getService().getLocalIDPAlias(tenantName, this.getServiceContext());
    }

    /**
     * Sets the alias associated with the entityId of local identity provider.
     *
     * @param tenantName name of tenant, cannot be null or empty.
     * @param alias alias to be set, can be null or empty.
     * @throws Exception
     */
    public void setLocalIDPAlias(String tenantName,  String alias) throws Exception
    {
        getService().setLocalIDPAlias(tenantName, alias, this.getServiceContext());
    }

    /**
     * Retrieves the OIDC Entity ID associated with the tenant
     *
     * @param tenantName Name of tenant
     * @return OIDC Entity ID String
     * @throws IDMException
     * @throws Exception
     */
    public String getOIDCEntityID(String tenantName) throws Exception
    {
        return getService().getOIDCEntityID(tenantName, this.getServiceContext());
    }

    /**
     * Retrieves the configuration setting in the tenant pertaining to the
     * maximum number of times a SAML token may be delegated.
     *
     * @param tenantName Name of tenant
     * @return a positive integer
     * @throws IDMException
     * @throws Exception
     */
    public int getDelegationCount(String tenantName) throws Exception
    {
        return getService().getDelegationCount(tenantName, this.getServiceContext());
    }

    /**
     * Sets the maximum number of times a SAML token generated by the tenant
     * may be delegated.
     *
     * @param tenantName      Name of tenant, required, non-null, non-empty.
     * @param delegationCount A positive integer, required, non-negative
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public
    void
    setDelegationCount(String tenantName, int delegationCount) throws Exception
    {
        getService().setDelegationCount(tenantName, delegationCount, this.getServiceContext());
    }

    /**
     * Retrieves the configuration setting in the tenant pertaining to the
     * maximum number of times a SAML token may be renewed.
     *
     * @param tenantName Name of tenant, required, non-null, non-empty
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public int getRenewCount(String tenantName) throws Exception
    {
        return getService().getRenewCount(tenantName, this.getServiceContext());
    }

    /**
     * Sets the maximum number of times a SAML token generated by the tenant may
     * be renewed.
     *
     * @param tenantName Name of tenant, required, non-null, non-empty
     * @param renewCount Maximum number of time. Positive integer
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public
    void setRenewCount(String tenantName, int renewCount) throws Exception
    {
        getService().setRenewCount(tenantName, renewCount, this.getServiceContext());
    }

    public SsoHealthStatsData getSsoStatistics(String tenantName)
            throws Exception {
        return getService().getSsoStatistics(tenantName, this.getServiceContext());
    }

    /**
     * Increments the generated tokens count for a tenant.
     *
     * @param tenant
     * @throws Exception
     */
    public void incrementGeneratedTokens(String tenant) throws Exception {
        getService().incrementGeneratedTokens(tenant, this.getServiceContext());
    }

    /**
     * Increments the renewed tokens count for a tenant.
     * @param tenant
     * @throws Exception
     */
    public void incrementRenewedTokens(String tenant) throws Exception {
        getService().incrementRenewedTokens(tenant, this.getServiceContext());
    }

    /**
     * Retrieves the configuration setting about the maximum lifetime in milliseconds
     * of a SAML bearer token generated for the tenant
     *
     * @param tenantName Name of tenant, required.non-null non-empty.
     * @return maximum bearer token lifetime in milliseconds
     * @throws IDMException
     * @throws Exception
     */
    public long getMaximumBearerTokenLifetime(String tenantName) throws Exception
    {
        return getService().getMaximumBearerTokenLifetime(tenantName, this.getServiceContext());
    }

    /**
     * Sets the maximum lifetime of a SAML bearer token generated for the tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param maxLifetime Maximum lifetime in milliseconds, required, non-negative.
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public
    void
    setMaximumBearerTokenLifetime(
        String tenantName,
        long   maxLifetime
        ) throws Exception
    {
        getService().setMaximumBearerTokenLifetime(tenantName, maxLifetime, this.getServiceContext());
    }

    /**
     * Retrieves the maximum lifetime of a SAML bearer token generated for the
     * tenant.
     *
     * @param tenantName Name of tenant, required.non-null non-empty.
     * @return maximum HOK token lifetime in milliseconds
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public long getMaximumHoKTokenLifetime(String tenantName) throws Exception
    {
        return getService().getMaximumHoKTokenLifetime(tenantName, this.getServiceContext());
    }

    /**
     * Sets the maximum lifetime of a SAML holder-of-key token generated for the
     * tenant.
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param maxLifetime Maximum lifetime in milliseconds, non negative.
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public
    void
    setMaximumHoKTokenLifetime(
        String tenantName,
        long   maxLifetime
        ) throws Exception
    {
        getService().setMaximumHoKTokenLifetime(tenantName, maxLifetime, this.getServiceContext());
    }

    public long getMaximumBearerRefreshTokenLifetime(String tenantName) throws Exception
    {
        return getService().getMaximumBearerRefreshTokenLifetime(tenantName, this.getServiceContext());
    }

    public void setMaximumBearerRefreshTokenLifetime(String tenantName, long maxLifetime) throws Exception
    {
        getService().setMaximumBearerRefreshTokenLifetime(tenantName, maxLifetime, this.getServiceContext());
    }

    public long getMaximumHoKRefreshTokenLifetime(String tenantName) throws Exception
    {
        return getService().getMaximumHoKRefreshTokenLifetime(tenantName, this.getServiceContext());
    }

    public void setMaximumHoKRefreshTokenLifetime(String tenantName, long maxLifetime) throws Exception
    {
        getService().setMaximumHoKRefreshTokenLifetime(tenantName, maxLifetime, this.getServiceContext());
    }

    /**
     * Retrieves the password policy for the tenant
     *
     * @param tenantName Name of tenant
     * @return Password policy configuration
     * @throws IDMException
     * @throws Exception
     */
    public
    PasswordExpiration
    getPasswordExpirationConfiguration(String tenantName) throws Exception
    {
        return getService().getPasswordExpirationConfiguration(tenantName, this.getServiceContext());
    }

    /**
     * Updates the password policy for the tenant
     *
     * @param tenantName Name of tenant
     * @param config     Password policy configuration
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    updatePasswordExpirationConfiguration(
        String             tenantName,
        PasswordExpiration config
        ) throws Exception
    {
        getService().updatePasswordExpirationConfiguration(tenantName, config, this.getServiceContext());
    }

    // Certificate Management

    /**
     * Retrieves the certificate chain used by the tenant to sign SAML tokens.
     *
     * @param tenantName Name of tenant
     * @return           Collection of certificates in chain
     * @throws IDMException
     * @throws Exception
     */
    public
    List<Certificate> getTenantCertificate(String tenantName) throws Exception
    {
        return getService().getTenantCertificate(tenantName, this.getServiceContext());
    }

    /**
     * Retrieves all certificate chains which are or have been used by the
     * tenant to sign SAML tokens.
     *
     * @param tenantName Name of tenant
     * @return           Collection of certificates in chain
     * @throws IDMException
     * @throws Exception
     */
    public
    Collection<List<Certificate>> getTenantCertificates(String tenantName) throws Exception
    {
        return getService().getTenantCertificates(tenantName, this.getServiceContext());
    }

    /**
     * Add a certificate the certificate store of a tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param idmCert Certificate to add
     * @param certificateType type of the certificate, valid type are 'LDAP' and 'STS'
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input (for instance, invalid certificate type)
     * @throws DuplicateCertificateException - if the particular certificate with certType already exists
     */
    @Deprecated
    public
    void
    addCertificate(
        String          tenantName,
        Certificate     idmCert,
        CertificateType certificateType
        ) throws Exception
    {
        getService().addCertificate(tenantName, idmCert, certificateType, this.getServiceContext());
    }

    /**
     * Retrieve all certificates of certain type for a tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param certificateType type of the certificate, valid type are 'LDAP' and 'STS'
     * @return A complete collection of certificates of 'certificateType' for tenant 'tenantName'
     *         In case no such certificate, an empty collection is returned
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input (for instance, invalid certificate type)
     */
    public
    Collection<Certificate>
    getAllCertificates(String tenantName, CertificateType certificateType) throws Exception
    {
        return getService().getAllCertificates(tenantName, certificateType, this.getServiceContext());
    }

    /**
     * Retrieve all root STS certificates for a tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @return A complete collection of root STS certificates for tenant 'tenantName'
     *         In case no such certificate, an empty collection is returned
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input (for instance, invalid certificate type)
     */
    public
    Collection<Certificate>
    getTrustedCertificates(String tenantName) throws Exception
    {
        return getService().getTrustedCertificates(tenantName, this.getServiceContext());
    }

    /**
     * Retrieve all leaf STS certificates for a tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @return A complete collection of leaf STS certificates for tenant 'tenantName'
     *         In case no such certificate, an empty collection is returned
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input (for instance, invalid certificate type)
     */
    public
    Collection<Certificate>
    getStsIssuersCertificates(String tenantName) throws Exception
    {
        return getService().getStsIssuersCertificates(tenantName, this.getServiceContext());
    }

    /**
     * Delete a certificates of certain type in a tenant's certificate store
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param fingerprint fingerprint of the certificate to be deleted
     * @param certificateType type of the certificate, valid type are 'LDAP' and 'STS'
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input (for instance, invalid certificate type)
     * @throws NoSuchCertificateException - in case the certificate of certificateType does not exist
     * @throws CertificateInUseException  - in case the certificate is the root certificate of an active signer Identity
     */
    public
    void
    deleteCertificate(String tenantName, String fingerprint, CertificateType certificateType) throws Exception
    {
        getService().deleteCertificate(tenantName, fingerprint, certificateType, this.getServiceContext());
    }

    // Principal Management

    /**
     * Searches and retrieves a service principal
     *
     * @param tenantName Name of tenant
     * @param username   Name of the service principal
     *                   This can be in one of the following formats.
     *                   1. UPN format (account@fully-qualified-domain-name)
     *                      Example : joe@abc.com
     *                   2. Alias\\account
     *                      Example : ABC\\joe
     * @return Service principal if found.  Otherwise returns null.
     * @throws IDMException
     * @throws Exception
     */
    public
    SolutionUser
    findSolutionUser(String tenantName, String username) throws Exception
    {
        return getService().findSolutionUser(tenantName, username, this.getServiceContext());
    }

    /**
     * Searches and retrieves a service principal matched by the specified
     * subject distinguished name
     *
     * @param tenantName Name of tenant
     * @param subjectDN  Distinguished name of subject specified in the
     *                   certificate.
     * @return Service principal if found. Otherwise, returns null.
     * @throws IDMException
     * @throws Exception
     */
    public
    SolutionUser
    findSolutionUserByCertDn(
        String tenantName,
        String subjectDN
        ) throws Exception
    {
        return getService().findSolutionUserByCertDn(tenantName, subjectDN, this.getServiceContext());
    }

    /**
     * Adds a service principal to the tenant's system domain under current ldu
     *
     * @param tenantName Name of tenant
     * @param userName   Name of service principal
     * @param detail     Detailed information about the service principal
     *                   detail information must contain userCertificate
     *                   userCertificate must have a valid non-empty subjectDN
     *                   or 'InvalidArugmentException' shall be thrown
     * @return Principal id of the service principal
     * @throws IDMException
     * @throws Exception
     */
    public
    PrincipalId
    addSolutionUser(
        String         tenantName,
        String         userName,
        SolutionDetail detail
        ) throws Exception
    {
        return getService().addSolutionUser(tenantName, userName, detail, this.getServiceContext());
    }

    /**
     * Adds an external service principal to the tenant's system domain
     *
     * @param tenantName Name of tenant
     * @param userName   Name of service principal
     * @param detail     Detailed information about the service principal
     *                   detail information must contain userCertificate
     *                   userCertificate must have a valid non-empty subjectDN
     *                   or 'InvalidArugmentException' shall be thrown
     * @return Principal id of the service principal
     * @throws IDMException
     * @throws Exception
     */
    public
    PrincipalId
    addExternalSolutionUser(
            String         tenantName,
            String         userName,
            SolutionDetail detail
            ) throws Exception
            {
        return getService().addSolutionUser(tenantName, userName, detail, this.getServiceContext());
            }

    /**
     * Finds a regular user defined in one of the tenant's identity providers
     *
     * @param tenantName Name of tenant
     * @param id         User to be found
     * @return the regular user information
     * @throws IDMException
     * @throws Exception
     */
    public
    PersonUser
    findPersonUser(String tenantName, PrincipalId id) throws Exception
    {
        return getService().findPersonUser(tenantName, id, this.getServiceContext());
    }

    /**
     * Finds a regular user defined in one of the tenant's identity providers
     *
     * @param tenantName      Name of tenant, required, non-null, non-empty.
     * @param objectSid  User ObjectSid, required, non-null.
     * @return the regular user information
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public
    PersonUser
    findPersonUserByObjectId(String tenantName, String UserObjectId) throws Exception
    {
        return getService().findPersonUserByObjectId(tenantName, UserObjectId, this.getServiceContext());
    }

    /**
     * Finds a security group defined in one of the tenant's identity providers
     *
     * @param tenantName Name of tenant
     * @param id         Group to be found
     * @return Group object of the security group. could be null if no such group
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     *         IDMEception              - any other exceptions could be returned.
     */
    public
    Group
    findGroup(String tenantName, PrincipalId groupId) throws Exception
    {
        return getService().findGroup(tenantName, groupId, this.getServiceContext());
    }

    /**
     * Finds a security group defined in one of the tenant's identity providers
     *
     * @param tenantName Name of tenant
     * @param id         Group to be found
     * @return Group object of the security group. could be null if no such group
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException   - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     *         IDMEception              - any other exceptions could be returned.
     */
    public
    Group
    findGroupByObjectId(String tenantName, String GroupObjectId) throws Exception
    {
        return getService().findGroupByObjectId(tenantName, GroupObjectId, this.getServiceContext());
    }

    /**
     * Finds regular users in one of the identity providers configured for the
     * tenant.
     *
     * The search criteria defines the identity domain to be searched.
     *
     * A user account is chosen for the search results if the search string is
     * part of either the account name, first name, last name, or description
     * of the account.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of the tenant
     * @param criteria   Search criteria, non negative
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of regular users found. Empty set when no user found.
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     *         IDMEception              - any other exceptions could be returned.
     */
    public
    Set<PersonUser>
    findPersonUsers(String tenantName, SearchCriteria criteria, int limit) throws Exception
    {
        return getService().findPersonUsers(tenantName, criteria, limit, this.getServiceContext());
    }

    /**
     * Finds regular users in one of the identity providers configured for the
     * tenant.
     *
     * The search criteria defines the identity domain to be searched.
     *
     * A user account is chosen for the search results if the search string is
     * part of the account name, i.e. for AD, samAccountName is used
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of the tenant
     * @param criteria   Search criteria, non negative, search is limited to only accountNames, i.e. in AD, samAccountName is used
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of regular users found. Empty set when no user found.
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     *         IDMEception              - any other exceptions could be returned.
     */
    public
    Set<PersonUser>
    findPersonUsersByName(String tenantName, SearchCriteria criteria, int limit) throws Exception
    {
        return getService().findPersonUsersByName(tenantName, criteria, limit, this.getServiceContext());
    }

    /**
     * Finds service principals matching the search criteria, specified in the
     * tenant's system domain.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName   Name of tenant
     * @param searchString Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of service principals found
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<SolutionUser>
    findSolutionUsers(String tenantName, String searchString, int limit) throws Exception
    {
        return getService().findSolutionUsers(tenantName, searchString, limit, this.getServiceContext());
    }

    /**
     * Finds security groups in the tenant matching the search criteria.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of tenant
     * @param criteria   Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of groups found.
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<Group>
    findGroups(String tenantName, SearchCriteria criteria, int limit) throws Exception
    {
        return getService().findGroups(tenantName, criteria, limit, this.getServiceContext());
    }

    /**
     * Finds security groups in the tenant matching the search criteria.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of tenant
     * @param criteria   Search criteria, search is limited to only accountName, i.e. in AD, samAccountName is used
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of groups found.
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<Group>
    findGroupsByName(String tenantName, SearchCriteria criteria, int limit) throws Exception
    {
        return getService().findGroupsByName(tenantName, criteria, limit, this.getServiceContext());
    }

    /**
     * Finds regular users, security groups and service principals in the
     * tenant.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of tenant
     * @param criteria   Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of security principals found
     * @throws IDMException
     * @throws Exception
     */
    public
    SearchResult
    find(String tenantName, SearchCriteria criteria, int limit) throws Exception
    {
        return getService().find(tenantName, criteria, limit, this.getServiceContext());
    }

    /**
     * Finds regular users, security groups and service principals in the
     * tenant.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName Name of tenant
     * @param criteria   Search criteria. Search is only limited to accountName, i.e. for AD, samAccountName is used
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of security principals found
     * @throws IDMException
     * @throws Exception
     */
    public
    SearchResult
    findByName(String tenantName, SearchCriteria criteria, int limit) throws Exception
    {
        return getService().findByName(tenantName, criteria, limit, this.getServiceContext());
    }

    /**
     * Finds regular users in a particular group of the tenant
     *
     * Utilizes 'contains'-style queries
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName   Name of tenant
     * @param groupId      Name of group
     * @param searchString Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of regular users found in the group
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<PersonUser>
    findPersonUsersInGroup(
            String      tenantName,
            PrincipalId groupId,
            String      searchString,
            int         limit
            ) throws Exception
            {
        return getService().findPersonUsersInGroup(
                tenantName,
                groupId,
                searchString,
                limit, this.getServiceContext());
    }

    /**
     * Finds regular users in a particular group of the tenant
     *
     * Utilizes 'starts with'-style queries
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName   Name of tenant
     * @param groupId      Name of group
     * @param searchString Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of regular users found in the group
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<PersonUser>
    findPersonUsersByNameInGroup(
            String      tenantName,
            PrincipalId groupId,
            String      searchString,
            int         limit
            ) throws Exception
            {
        return getService().findPersonUsersByNameInGroup(
                tenantName,
                groupId,
                searchString,
                limit, this.getServiceContext());
    }

    /**
     * Finds service principals in a particular group of the tenant
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName    Name of tenant
     * @param groupName     Name of security group
     * @param searchString  Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of service principals found.
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<SolutionUser>
    findSolutionUsersInGroup(
        String tenantName,
        String groupName,
        String searchString,
        int    limit
        ) throws Exception
    {
        return getService().findSolutionUsersInGroup(
                tenantName,
                groupName,
                searchString,
                limit, this.getServiceContext());
    }

    /**
     * Finds the set of groups that are immediately part of the specified group
     * in the tenant
     *
     * Utilizes 'contains'-style queries
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName    Name of tenant
     * @param groupId       Name of group to search in
     * @param searchString  Search criteria for candidate groups
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of security groups found
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<Group>
    findGroupsInGroup(
        String      tenantName,
        PrincipalId groupId,
        String      searchString,
        int         limit
        ) throws Exception
    {
        return getService().findGroupsInGroup(
                tenantName,
                groupId,
                searchString,
                limit, this.getServiceContext());
    }

    /**
     * Finds the set of groups that are immediately part of the specified group
     * in the tenant
     *
     * Utilizes 'starts with'-style queries.
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName    Name of tenant
     * @param groupId       Name of group to search in
     * @param searchString  Search criteria for candidate groups
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of security groups found
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<Group>
    findGroupsByNameInGroup(
        String      tenantName,
        PrincipalId groupId,
        String      searchString,
        int         limit
        ) throws Exception
    {
        return getService().findGroupsByNameInGroup(
                tenantName,
                groupId,
                searchString,
                limit, this.getServiceContext());
    }

    /**
     * Finds the set of groups that contain the specified security principal in
     * the tenant.
     *
     * The principal whose immediate parents are desired, may be a user or group
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param principalId Security principal id, required, non-null.
     * @return Set of immediate parent groups found.
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws NoSuchIdpException   - system tenant is not set up.
     * @throws InvalidArgumentException    -- if the tenant name is null or empty
     * @throws InvalidPrincipalException    - principal ID is invalid.
     * @throws IDMException         - wrapping exception for any other exceptions
     *                              from down the stack.
     */
    public
    Set<Group>
    findDirectParentGroups(
        String      tenantName,
        PrincipalId principalId
        ) throws Exception
    {
        return getService().findDirectParentGroups(tenantName, principalId, this.getServiceContext());
    }

    /**
     * Checks whether principal is member of system group. Principals include end
     * users and groups in any back-end identity source (incl. solution users).
     *
     * @param tenantName name of the tenant to search a group for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param principalId
     *           principal identifier, required
     * @param groupName
     *           system group name, required
     * @return
     *         if that group does not exist or the principal is not its member
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws NoSuchIdpException   - system tenant is not set up.
     * @throws InvalidArgumentException    -- if the tenant name is null or empty
     * @throws InvalidPrincipalException    - principal ID is invalid.
     * @throws IDMException         - wrapping exception for any other exceptions
     *                              from down the stack.
     */
    public boolean isMemberOfSystemGroup(
        String tenantName, PrincipalId principalId, String groupName) throws Exception
    {
        return getService().isMemberOfSystemGroup(tenantName, principalId, groupName, this.getServiceContext());
    }

    /**
     * Finds the set of groups the specified security principal is part of.
     * The resultant set of groups resolves nesting and returns parent groups.
     *
     * @param tenantName Name of tenant
     * @param userId     User principal whose group membership is desired
     * @return Full set of groups the user is a member of.
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<Group>
    findNestedParentGroups(
        String      tenantName,
        PrincipalId userId
        ) throws Exception
    {
        return getService().findNestedParentGroups(tenantName, userId, this.getServiceContext());
    }

    /**
     * Finds all the regular users that are locked in the tenant's system domain
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName   Name of tenant
     * @param searchString Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of locked regular users found
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<PersonUser>
    findLockedUsers(String tenantName, String searchString, int limit) throws Exception
    {
        return getService().findLockedUsers(tenantName, searchString, limit, this.getServiceContext());
    }

    /**
     * Finds all the regular users that are disabled in the tenant's system
     * domain
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName   Name of tenant
     * @param searchString Search criteria
     * @param limit a positive integer for the maximum number of items to return unless negative (no return limit is set)
     * @return Set of disabled regular users found
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<PersonUser>
    findDisabledPersonUsers(
        String tenantName,
        String searchString,
        int    limit
        ) throws Exception
    {
        return getService().findDisabledPersonUsers(tenantName, searchString, limit, this.getServiceContext());
    }

    /**
     * Finds all the service principals that are disabled in the tenant's system
     * domain
     *
     * Regular expressions are not allowed in the search string at this time.
     *
     * @param tenantName   Name of tenant
     * @param searchString Search criteria
     * @return Set of disabled service principals found
     * @throws IDMException
     * @throws Exception
     */
    public
    Set<SolutionUser>
    findDisabledSolutionUsers(
        String tenantName,
        String searchString
        ) throws Exception
    {
        return getService().findDisabledSolutionUsers(tenantName, searchString, this.getServiceContext());
    }

    /**
     * Adds a regular user to the tenant's system domain
     *
     * @param tenantName Name of tenant. required, non-null, non-empty
     * @param userName   Name of regular user. required, non-null,
     * @param detail     Detailed information about the user. required, non-null,
     * @param password   User's password
     * @return Principal id of the regular user after it has been created.
     * @throws Exception
     * @throws NoSuchTenantException    - when tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws InvalidPrincipalException    - empty username or user already exist
     */

    public
    PrincipalId
    addPersonUser(
        String       tenantName,
        String       userName,
        PersonDetail detail,
        char[]       password
        ) throws Exception
    {
        return getService().addUser(tenantName, userName, detail, password, this.getServiceContext());
    }

    /**
     * Adds a regular user to the tenant's system domain
     *
     * @param tenantName Name of tenant. required, non-null, non-empty
     * @param userName   Name of regular user. required, non-null,
     * @param detail     Detailed information about the user. required, non-null,
     * @param hashedPassword   User's hashed password. required, non-null,
     * @param hashingAlgorithm  The algorithm used to generate the password hash, this is mandatory,
     *        The valid values are defined in enum 'HashingAlgorithmType', currently vmware-directory
     *        only supports 'SSO-v1-1'. required, non-null,
     * @return Principal id of the regular user after it has been created.
     * @throws Exception
     * @throws NoSuchTenantException    - when tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws InvalidPrincipalException    - empty username or user already exist
     */
    public
    PrincipalId
    addPersonUser(
        String       tenantName,
        String       userName,
        PersonDetail detail,
        byte[]       hashedPassword,
        String       hashingAlgorithm
        ) throws Exception
    {
        return getService().addUser(tenantName, userName, detail, hashedPassword, hashingAlgorithm, this.getServiceContext());
    }

    /**
     * Adds a JIT user to the tenant's system domain
     * @param tenantName Name of tenant. required, non-null, non-empty
     * @param userName Name of JIT user. required, non-null, non-empty
     * @param detail Detailed information about the user. required, non-null, non-empty
     * @param extIdpEntityId required, non-null, non-empty
     * @param extUserId required, non-null, non-empty
     * @return id of the JIT user after it has been created
     * @throws Exception
     * @throws NoSuchTenantException    - when tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws InvalidPrincipalException    - empty username or user already exist
     */
    public
    PrincipalId
    addJitUser(
            String       tenantName,
            String       userName,
            PersonDetail detail,
            String       extIdpEntityId,
            String       extUserId
            ) throws Exception
    {
        ValidateUtil.validateNotEmpty(extIdpEntityId, "ExternalIDP entity ID.");
        ValidateUtil.validateNotEmpty(extUserId, "ExternalIDP User ID.");

        if (isJitEnabledForExternalIdp(tenantName, extIdpEntityId))
        {
            return getService().addJitUser(tenantName, userName, detail, extIdpEntityId, extUserId, this.getServiceContext());
        }

        throw new InvalidPrincipalException(String.format("User %s cannot be added "
                + "since JIT is not enabled for external IDP with entityID %s",
                userName, extIdpEntityId), userName);
    }

    /**
     * Retrieve user hashed password blob
     *
     * @param tenantName Name of tenant
     * @param Principal id of the user
     * @return User's hashed password blob
     * @throws Exception
     * @throws NoSuchTenantException - if no such tenant exist
     * @throws NoSuchIdpException   - system tenant is not set up.
     * @throws InvalidArgumentException    -- if the tenant name is null or empty
     * @throws InvalidPrincipalException    - if user does not exist or multiple ones are found
     */
    public
    byte[]
    getUserHashedPassword(
        String tenantName,
        PrincipalId principal
    ) throws Exception
    {
        return getService().getUserHashedPassword(tenantName, principal, this.getServiceContext());
    }

    /**
     * Adds a security group to the tenant's system domain
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param groupName   Name of security group, required non-null, non-empty
     * @param groupDetail Detailed information about the group. required, non-null.
     * @return Principal id of the group after it has been created.
     * @throws Exception
     * @throws InvalidArgumentException    - illegal input valid was passed.
     * @throws NoSuchTenantException
     * @throws NoSuchIdpException        when system provider is missing
     */
    public
    PrincipalId
    addGroup(
        String      tenantName,
        String      groupName,
        GroupDetail groupDetail
        ) throws Exception
    {
        return getService().addGroup(tenantName, groupName, groupDetail, this.getServiceContext());
    }

    /**
     * Adds a regular user to a security group in the tenant
     *
     * @param tenantName Name of tenant,  required non-null, non-empty
     * @param userId     Name of regular user to be assigned group
     *                     membership.  required non-null.
     * @param groupName  Name of security group.  required non-null, non-empty
     * @return true if the user's group membership was successfully assigned,
     *         false otherwise.
     * @throws Exception
     * @throws NoSuchTenantException    - when tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws InvalidPrincipalException  - unable to find the group
     * @throws MemberAlreadyExistException    - user already exist
     *
     */
    public
    boolean
    addUserToGroup(
        String      tenantName,
        PrincipalId userId,
        String      groupName
        ) throws Exception
    {
        return getService().addUserToGroup(tenantName, userId, groupName, this.getServiceContext());
    }

    /**
     * Removes a principal from a security group in the tenant's system domain.
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param principalId Name of principal whose membership must be adjusted
     * @param groupName   Name of group from which the principal must be removed
     * @return true if the group membership was successfully adjusted
     * @throws IDMException
     * @throws Exception
     */
    public
    boolean
    removeFromLocalGroup(
        String      tenantName,
        PrincipalId principalId,
        String      groupName
        ) throws Exception
    {
        return getService().removeFromLocalGroup(
                tenantName,
                principalId,
                groupName, this.getServiceContext());
    }

    /**
     * Adds a security group to another in the tenant's system domain.
     *     Applies to system provider only. For external provider, we have read privilege only.
     *  If the group is already exist, nothing is changed.
     *
     * @param tenantName Name of tenant. required.
     * @param groupId    Name of security group to be added. required, non-null
     * @param groupName  Name of destination security group to add to. required, non-null, non-empty.
     * @return true if the membership has been successfully changed,
     *         false otherwise
     * @throws Exception
     * @throws InvalidArgumentException    illegal input
     * @throws NoSuchTenantException        tenant does not exist
     * @throws NoSuchIdpException            System tenant is missing
     * @throws MemberAlreadyExistException  the group is already a member
     * @throws InValidPrincipleException    groupId is invalid
     * @throws IDMException                other type of exception might be thrown due variety of reasons.
     */
    public
    boolean
    addGroupToGroup(
        String      tenantName,
        PrincipalId groupId,
        String      groupName
        ) throws Exception
    {
        return getService().addGroupToGroup(tenantName, groupId, groupName, this.getServiceContext());
    }

    /**
     * Deletes the principal from the tenant's system domain
     *
     * @param tenantName    Name of tenant
     * @param principalName Name of principal to be deleted
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    deletePrincipal(String tenantName, String principalName) throws Exception
    {
        getService().deletePrincipal(tenantName, principalName, this.getServiceContext());
    }

    /**
     * Enables a regular user account in the system domain of the tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param userId     Name of regular user
     * @return true if the account was enabled, false otherwise
     * @throws IDMException
     * @throws Exception
     */
    public
    boolean
    enableUserAccount(String tenantName, PrincipalId userId) throws Exception
    {
        return getService().enableUserAccount(tenantName, userId, this.getServiceContext());
    }

    /**
     * Disables a user account in the system domain of the tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param userId     Name of regular user, required, non-null.
     * @return true if the account was disabled by this call, false if already disabled.
     * @throws IDMException
     * @throws Exception
     * @throws NoSuchTenantException      - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     * @throws NoSuchIdpException         - can't find provider for this group.
     * @throws IDMException              - any other exceptions could be returned.
     */

    public
    boolean
    disableUserAccount(String tenantName, PrincipalId userId) throws Exception
    {
        return getService().disableUserAccount(tenantName, userId, this.getServiceContext());
    }

    /**
     * Unlocks a user account in the system domain of the tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param userId     Name of regular user
     * @return true if account was unlocked, false otherwise.
     * @throws IDMException
     * @throws Exception
     */
    public
    boolean
    unlockUserAccount(String tenantName, PrincipalId userId) throws Exception
    {
        return getService().unlockUserAccount(tenantName, userId, this.getServiceContext());
    }

    /**
     * Updates the details of a regular user in the system domain of the tenant
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param userName   Name of the regular user
     * @param detail     Details to be updated
     * @return Principal id of the regular user
     * @throws InvalidPrincipalException    - empty username or user does not exist
     * @throws NoSuchTenantException
     * @throws IDMException all other unexpected errors
     */
    public
    PrincipalId
    updatePersonUserDetail(
        String       tenantName,
        String       userName,
        PersonDetail detail
        ) throws Exception
    {
        return getService().updatePersonUserDetail(
                tenantName,
                userName,
                detail, this.getServiceContext());
    }

    /**
     * Updates a security group in the system domain of the tenant
     *
     * @param tenantName Name of tenant
     * @param groupName  Name of security group
     * @param detail     Group details to be updated
     * @return Principal id pertaining to the security group
     * @throws IDMException
     * @throws Exception
     */
    public
    PrincipalId
    updateGroupDetail(
        String      tenantName,
        String      groupName,
        GroupDetail detail
        ) throws Exception
    {
        return getService().updateGroupDetail(tenantName, groupName, detail, this.getServiceContext());
    }

    /**
     * Updates a service principal account in the tenant
     *
     * The service principal will be located in the system domain of the tenant
     *
     * @param tenantName Name of tenant
     * @param userName   Name of service principal
     * @param detail     Details of the service principal to be updated.
     * @return Principal Id of the service principal account
     * @throws InvalidPrincipalException    - empty username or user does not exist
     * @throws NoSuchTenantException
     * @throws IDMException all other unexpected errors
     */
    public
    PrincipalId
    updateSolutionUserDetail(
        String         tenantName,
        String         userName,
        SolutionDetail detail
        ) throws Exception
    {
        return getService().updateSolutionUserDetail(
                tenantName,
                userName,
                detail, this.getServiceContext());
    }

    /**
     * Sets the password
     *
     * The new password is not subject to password policy requirements
     *
     * This routine must be invoked in the (login) context of a user with
     * administrative privileges in the domain of the Identity Provider
     * that governs the principal.
     *
     * @param tenantName  Name of tenant, required.non-null non-empty
     * @param userName    Principal for whom the password must be set. required.non-null non-empty
     * @param newPassword New password. required.non-null non-empty
     * @throws PasswordPolicyViolationException    Illegal password, such as empty pw, was used
     * @throws InvalidPrincipalException  Empty user name or user can not be found
     * @throws NoSuchTenantException
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    setUserPassword(
            String tenantName,
            String userName,
            char[] newPassword
            ) throws Exception
    {
        getService().setUserPassword(tenantName, userName, newPassword, this.getServiceContext());
    }

    /**
     * Changes the user's password after verifying the current password
     *
     * The new password is subject to password policy requirements
     *
     * @param tenantName      Name of tenant
     * @param userName        Principal for which password must be changed
     * @param currentPassword Current password
     * @param newPassword     New password
     * @throws InvalidPrincipalException  Empty user name or user can not be found
     * @throws NoSuchTenantException
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    changeUserPassword(
            String tenantName,
            String userName,
            char[] currentPassword,
            char[] newPassword) throws Exception
    {
        getService().changeUserPassword(
                tenantName,
                userName,
                currentPassword,
                newPassword, this.getServiceContext());
    }

    public
    void
    updateSystemDomainStorePassword(String tenantName, char[] newPassword) throws Exception
    {
        getService().updateSystemDomainStorePassword(tenantName, newPassword, this.getServiceContext());
    }

    /**
     * Retrieves the password policy for the specified tenant.
     *
     * @param tenantName Name of tenant, required non-null, non-empty
     * @return policy     pw Policy for the tenant.
     * @throws Exception
     * @throws NoSuchTenantException        - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */

    public PasswordPolicy getPasswordPolicy(String tenantName) throws Exception
    {
        return getService().getPasswordPolicy(tenantName, this.getServiceContext());
    }

    /**
     * Sets the password policy for the specified tenant.
     *
     * @param tenantName Name of tenant, required non-null, non-empty
     * @param policy     password Policy for the tenant, required non-null
     * @throws Exception
     * @throws NoSuchTenantException        - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public
    void
    setPasswordPolicy(String tenantName, PasswordPolicy policy) throws Exception
    {
        getService().setPasswordPolicy(tenantName, policy, this.getServiceContext());
    }

    /**
     * Retrieves the lockout policy for the specified tenant.
     *
     * @param tenantName Name of tenant, required non-null, non-empty
     * @throws Exception
     * @throws NoSuchTenantException        - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public LockoutPolicy getLockoutPolicy(String tenantName) throws Exception
    {
        return getService().getLockoutPolicy(tenantName, this.getServiceContext());
    }

    /**
     * Sets the lockout policy for the specified tenant.
     *
     * @param tenantName Name of tenant, required non-null, non-empty
     * @param policy     Lockout Policy for the tenant, required non-null
     * @throws Exception
     * @throws NoSuchTenantException        - tenant does not exist
     * @throws InvalidArgumentException  - invalid input
     */
    public
    void
    setLockoutPolicy(String tenantName, LockoutPolicy policy) throws Exception
    {
        getService().setLockoutPolicy(tenantName, policy, this.getServiceContext());
    }

    /**
     * Search for a group by string based on account name.
     * @param tenantName name of the tenant to search a group for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param group name in one of the formats: <name>@[domain||alias]; [domain||alias]\\<name>;
     *        if default domain is configured, domain part could be omitted which means searching
     *        in the default domain only
     *        Required, non-null, non-empty.
     * @return Group.
     * @throws Exception
     * @throws NoSuchTenantException specified tenant could not be found
     * @throws NoSuchIdpException domain/alias cannot be recognized.
     * @throws InvalidPrincipalException specified group cannot be found
     * @throws IDMException In case of other failure.
     */
    public Group findGroup( String tenantName, String group )
            throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, IDMException, Exception
    {
        return getService().findGroup(tenantName, group, this.getServiceContext());
    }

    /**
     * Search for a user by string based on account name.
     * @param tenantName name of the tenant to search a user for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param user user name in one of the formats: <name>@[domain||alias]; [domain||alias]\\<name>;
     *        if default domain is configured, domain part could be omitted which means searching
     *        in the default domain only
     *        Required, non-null, non-empty.
     * @return Principal. This could be a SolutionUser or a PersonUser.
     * @throws NoSuchTenantException specified tenant could not be found
     * @throws NoSuchIdpException domain/alias cannot be recognized.
     * @throws InvalidPrincipalException specified user cannot be found
     * @throws IDMException In case of a failure.
     * @throws Exception
     */
    public Principal findUser( String tenantName, String user )
            throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        return getService().findUser(tenantName, user, this.getServiceContext());
    }

    /**
     * Search for an active user within tenant's system domain by string based on a specified attribute.
     * @param tenantName name of the tenant to search a user for.
     *        Required, non-null, non-empty, case-insensitive.
     * @param attributeName Name of the attribute to base the search on
     *        Required, non-null, non-empty.
     * @param attributeValue Value to map to.
     *        Required, non-null, non-empty.
     * @return PrincipalId. If of the active user if found. Can be null.
     * @throws NoSuchTenantException specified tenant could not be found
     * @throws NoSuchIdpException domain/alias cannot be recognized.
     * @throws InvalidPrincipalException specified user cannot be found
     * @throws IDMException In case of a failure.
     * @throws Exception
     */
    public PrincipalId findActiveUserInSystemDomain( String tenantName, String attributeName, String attributeValue )
            throws NoSuchTenantException, NoSuchIdpException, InvalidPrincipalException, Exception
    {
        return getService().findActiveUserInSystemDomain(tenantName, attributeName, attributeValue, this.getServiceContext());
    }

    /**
     * import tenant configuration defined in the configuration DOM file.
     * @param tenantName      Name of tenant the config file is associated.
     * @param configDoc        XML document contain the configuration of the tenant.
     * @throws IDMException
     * @throws Exception
     */
    public
    void
    importTenantConfiguration(
            String tenantName,
            Document configDoc) throws Exception
    {
        //validate against SAML schema.
        samlValidate(configDoc);

        SAMLImporter importer = new SAMLImporter(this);
        importer.importConfiguration(tenantName, configDoc);
    }

    /**
     * export tenant configuration to a DOM file.
     * @param tenantName      Name of tenant whose configuration to be exported.
     * @param exportPrivateData whether to export complete configuration (all private data included) or not.
     * @return Document        Document object that contain the configuration.
     * @throws IDMException
     * @throws Exception
     */
    public
    Document
    exportTenantConfiguration(
            String tenantName, boolean exportPrivateData) throws Exception
    {
        //We could use a factory if there is need in future for different style of exporter;
        SAMLExporter exporter = new SAMLExporter(this);
        Document doc = exporter.exportConfiguration(tenantName, exportPrivateData);

        return doc;
    }

    /**
     * returns the tenant's SAML2 metadata as a DOM file.
     *     refer http://docs.oasis-open.org/security/saml/v2.0/saml-metadata-2.0-os.pdf
     *     for more info on format description.
     * @param tenantName      Name of tenant whose configuration to be exported.
     * @param exportPrivateData whether to export complete configuration (all private data included) or not.
     * @return Document        Document object that contain the configuration.
     * @throws IDMException
     * @throws Exception
     */
    public
    Document
    getSsoSaml2Metadata(
            String tenantName) throws Exception
    {
        SAMLExporter exporter = new SAMLExporter(this);
        Document doc = exporter.exportSaml2Metadata(tenantName);

        return doc;
    }

    /**
     * import external IDP configuration defined in the configuration DOM file
     * into tenant.
     *
     * @param tenantName
     *           Name of tenant.
     * @param doc
     *           XML document contain the external IDP configuration for the
     *           tenant.
     * @return entityID of the imported configuration
     * @throws Exception
     * @throws AssertionError
     *            the XML document validation fails
     * @throws IDMException
     * @throws ExternalIDPCertChainInvalidTrustedPathException
     *            invalid certificate chain
     * @throws ExternalIDPExtraneousCertsInCertChainException
     *            extra certificates found outside of certificate chain.
     */
    public String importExternalIDPConfiguration(String tenantName, Document doc)
            throws ExternalIDPCertChainInvalidTrustedPathException,
            ExternalIDPExtraneousCertsInCertChainException, Exception {
        samlValidate(doc);
        return new SAMLImporter(this).importExternalIDPConfig(tenantName, doc);
    }

    /**
     * export Castle as SP profile in Castle<-->ExternalIDP federation together
     * with optional Castle's external IDP data
     *
     * @param tenantName
     *            name of the tenant
     * @param exportExternalIDP
     *            will include Castle's external IDP data if set to true
     * @return Castle's profile as defined in SAML 2.0 schema for
     *         SPSSODescriptor and IDPSSODescriptor(optional)
     * @throws Exception
     */
    public Document exportExternalIDPFederation(String tenantName, boolean exportExternalIDP) throws Exception
    {
        Document doc = new SAMLExporter(this).exportCastleSPProfile(tenantName, exportExternalIDP);
        return doc;
    }

    /**
     * Register a external IDP user in the system provider of the tenant
     *
     * @param tenantName
     *            Name of tenant, required non-null, non-empty
     * @param userId
     *            represent a globally unique external IDP user which is to be
     *            assigned default group membership as part of registration.
     *            required non-null.
     * @return true if the external IDP user was successfully registered.
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             invalid input
     * @throws NoSuchTenantException
     *             when tenant does not exist
     * @throws MemberAlreadyExistException
     *             external IDP user already registered
     *
     */
    public
    boolean
    registerThirdPartyIDPUser(
            String      tenantName,
            PrincipalId userId
            ) throws Exception
    {
        return getService().registerThirdPartyIDPUser(tenantName, userId, this.getServiceContext());
    }

    /**
     * Remove a external IDP user registration in the system provider of the tenant
     *
     * @param tenantName
     *            Name of tenant, required non-null, non-empty
     * @param userId
     *            represent a globally unique external IDP user whose registration in
     *            tenant's system provider is to be removed
     *            required non-null.
     * @return true if the external IDP user was successfully removed.
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             invalid input
     * @throws NoSuchTenantException
     *             when tenant does not exist
     * @throws InvalidPrincipalException
     *             external IDP user was not registered
     *
     */
    public
    boolean
    removeThirdPartyIDPUser(
        String      tenantName,
        PrincipalId userId
    ) throws Exception
    {
        return getService().removeThirdPartyIDPUser(tenantName, userId, this.getServiceContext());
    }

    public PersonUser find (String tenantName, PrincipalId userId) throws Exception
    {
        return getService().findRegisteredExternalIDPUser(tenantName, userId, this.getServiceContext());
    }

    public PersonUser findRegisteredExternalIDPUser(String tenantName, PrincipalId userId) throws Exception
    {
        return getService().findRegisteredExternalIDPUser(tenantName, userId, this.getServiceContext());
    }

    /**
     * @return the name of the group designated for external IDP user registration
     * @throws NotBoundException
     * @throws MalformedURLException
     */
    public String getExternalIDPRegistrationGroupName()
            throws Exception
    {
        return getService().getExternalIDPRegistrationGroupName(this.getServiceContext());
    }

    /**
     * Register UPN suffix for the tenant domain
     * @param tenantName
     *           cannot be null or empty
     * @param domainName
     *           cannot be null or empty
     * @param upnSuffix
     *           UPN suffix to register, such as "@gmail.com". cannot be null or
     *           empty.
     * @return true if value does not exist and added successfully; false if
     *         value is not added due to value already exist or unsuccessful
     *         operation handled gracefully.
     *         <p> Value matching is not case-sensitive.
     * @throws Exception
     * @throws IDMException
     * @throws InvalidArgumentException
     *             invalid input
     * @throws NoSuchTenantException
     *             when tenant does not exist
     * @throws NoSuchIdpException
     *             when domain does not exist for the tenant
     */
    public boolean registerUpnSuffix(String tenantName, String domainName, String upnSuffix) throws Exception
    {
       return getService().registerUpnSuffix(tenantName, domainName, upnSuffix, this.getServiceContext());
    }

   /**
    * un-Register UPN suffix for the tenant domain
    *
    * @param tenantName
    *           cannot be null or empty
    * @param domainName
    *           cannot be null or empty
    * @param upnSuffix
    *           UPN suffix to un-register, such as "@gmail.com". cannot be null
    *           or empty.
    * @return true if value existed and is removed successfully; false if value
    *         did not exist or operation is unsuccessful but handled gracefully.
    *         <p>
    *         Value matching is not case-sensitive.
    * @throws Exception
    * @throws IDMException
    * @throws InvalidArgumentException
    *            invalid input
    * @throws NoSuchTenantException
    *            when tenant does not exist
    * @throws NoSuchIdpException
    *            when domain does not exist for the tenant
    */
   public boolean unregisterUpnSuffix(String tenantName, String domainName,
         String upnSuffix) throws Exception {
      return getService().unregisterUpnSuffix(tenantName, domainName, upnSuffix, this.getServiceContext());
   }

   /**
    * Get UPN suffix for the tenant domain
    *
    * @param tenantName
    *           cannot be null or empty
    * @param domainName
    *           cannot be null or empty
    * @param upnSuffix
    *           UPN suffix to un-register, such as "@gmail.com". cannot be null
    *           or empty.
    * @return set of strings for the suffixes or {@code null} if none registered
    * @throws Exception
    * @throws IDMException
    * @throws InvalidArgumentException
    *            invalid input
    * @throws NoSuchTenantException
    *            when tenant does not exist
    * @throws NoSuchIdpException
    *            when domain does not exist for the tenant
    */
   public Set<String> getUpnSuffixes(String tenantName, String domainName) throws Exception {
      return getService().getUpnSuffixes(tenantName, domainName, this.getServiceContext());
   }

    /**
     * Validate the xml file using SAML name spaces
     * @throws SAXException
     *      If the {@link ErrorHandler} throws a {@link SAXException} or
     *      if a fatal error is found and the {@link ErrorHandler} returns
     *      normally.
     * @throws IOException
     *      If the validator is processing a
     *      {@link javax.xml.transform.sax.SAXSource} and the
     *      underlying {@link org.xml.sax.XMLReader} throws an
     *      {@link IOException}.
     */
    public void samlValidate(Document doc) throws SAXException, IOException
    {
        try
        {
            //validate the configuration file.
            Validator validator = getSamlSchema().newValidator();
            validator.validate(new DOMSource(doc));
        } catch (Exception e)
        {
            throw new AssertionError(e);
        }
    }

    /**
     * Retrieve the Active Directory Domain Join Status of the system the
     * identity server is executing on.
     * @return Active Directory Join Information
     * @throws Exception
     * @throws IDMException
     *            Errors during retrieve AD join status
     */
    public ActiveDirectoryJoinInfo
    getActiveDirectoryJoinStatus() throws Exception {
        return getService().getActiveDirectoryJoinStatus(this.getServiceContext());
    }

    /**
     * Retrieve the Active Directory Domain Trust info of the system the
     * identity server is executing on.
     * @return Domain Trust info
     * @throws Exception
     * @throws IDMException
     *            Errors during retrieve AD join status
     */
    public Collection<DomainTrustsInfo>
    getDomainTrustInfo() throws Exception {
        return getService().getDomainTrustInfo(this.getServiceContext());
    }

    /**
     * Queries the Cluster id for this instance
     *
     * @return Cluster Identifier
     * @throws Exception
     */
    public String getClusterId() throws Exception
    {
        return getService().getClusterId(this.getServiceContext());
    }

    /**
     * Queries the Deployment id for this instance
     *
     * @return Deployment identifier
     * @throws Exception
     */
    public String getDeploymentId() throws Exception
    {
        return getService().getDeploymentId(this.getServiceContext());
    }

    /**
     * @return Returns host name of the sso box. (IP as a fall-back)
     * @throws Exception
     * @throws IDMException
     */
    public String getSsoMachineHostName() throws Exception
    {
        return getService().getSsoMachineHostName(this.getServiceContext());
    }

    /**
     * Retrieve a collection of all joined systems, including Domain Controllers and,
     * optionally, Computer accounts.
     *
     * @param tenantName
     *           cannot be null or empty
     * @param getDCOnly
     *           indicates whether the caller is interested in Domain Controller accounts only.
     *           If <tt>false</tt>, retrieves Computer accounts as well.
     * @return list of VmHostData containing information about the joined systems.
     * @throws Exception
     * @throws IDMException
     */
    public Collection<VmHostData> getComputers(String tenantName, boolean getDCOnly) throws Exception
    {
       return getService().getComputers(tenantName, getDCOnly, this.getServiceContext());
    }

   /**
    * Operation to join the SSO server to AD domain
    * @param user
    *           cannot be null or empty
    * @param password
    *           cannot be null
    * @param domain
    *           cannot be null or empty
    * @param orgUnit
    *           can be null or empty
    * @return domain join information, could be null when SSO server is not joined to AD domain.
    * @throws Exception
    * @throws IDMException
    * @throws IdmADDomainException
    */
    public void joinActiveDirectory(String user, String password, String domain, String orgUnit) throws Exception
    {
       getService().joinActiveDirectory(user, password, domain, orgUnit, this.getServiceContext());
    }

    /**
     * return server SPN (HTTP/computername.domainname) if server is AD
     * domain-joined. Otherwise return computer name(HTTP/computername).
     *
     * @throws Exception
     */
    public String getServerSPN() throws Exception {
        return getService().getServerSPN();
    }
    /**
     * Operation for the SSO server to leave AD domain
     * @param user
     *           cannot be null or empty
     * @param password
     *           cannot be null
     * @throws Exception
     * @throws IDMException
     * @throws IdmADDomainAccessDeniedException
     * @throws ADIDSAlreadyExistException
     *            ADIDS is registered so that cannot un-join the AD domain
     * @throws IdmADDomainException
     */
     public void leaveActiveDirectory(String user, String password) throws Exception
     {
        getService().leaveActiveDirectory(user, password, this.getServiceContext());
     }

     /**
      * Operation for retrieving the IDM authentication cache
      *
      * @param tenantName
      * @throws Exception
      */
     public List<IIdmAuthStat> getIdmAuthStats(String tenantName)
             throws Exception
     {
         return getService().getIdmAuthStats(tenantName, this.getServiceContext());
     }

     /**
      * Operation for retrieving the status of the IDM authentication cache
      *
      * @param tenantName
      * @return the status of the IDM authentication cache
      * @throws Exception
      */
     public IIdmAuthStatus getIdmAuthStatus(String tenantName) throws Exception {
         return getService().getIdmAuthStatus(tenantName, this.getServiceContext());
     }

     /**
      * Operation for clearing the IDM authentication cache
      *
      * @param tenantName
      * @throws Exception
      */
     public void clearIdmAuthStats(String tenantName) throws Exception {
         getService().clearIdmAuthStats(tenantName, this.getServiceContext());
     }

     /**
      * Operation for setting the IDM authentication cache size
      *
      * @param tenantName
      * @param size
      * @throws Exception
      */
     public void setIdmAuthStatsSize(String tenantName, int size) throws Exception {
         getService().setIdmAuthStatsSize(tenantName, size, this.getServiceContext());
     }

     /**
      * Operation for enabling the IDM authentication cache
      *
      * @param tenantName
      * @throws Exception
      */
     public void enableIdmAuthStats(String tenantName) throws Exception {
         getService().enableIdmAuthStats(tenantName, this.getServiceContext());
     }

     /**
      * Operation for disabling the IDM authentication cache
      *
      * @param tenantName
      * @throws Exception
      */
     public void disableIdmAuthStats(String tenantName) throws Exception {
         getService().disableIdmAuthStats(tenantName, this.getServiceContext());
     }

    /**
     * Operation for retrieving the tenant Client Authentication Policy
     *
     * @param tenantName
     * @throws IDMException
     */
    public AuthnPolicy getAuthnPolicy(String tenantName) throws Exception {
        return getService()
                .getAuthNPolicy(tenantName, this.getServiceContext());
    }

    /**
     * Operation for setting the tenant Client Authentication Policy
     *
     * @param tenantName
     * @throws IDMException
     */
    public void setAuthnPolicy(String tenantName, AuthnPolicy policy)
            throws Exception {
        getService().setAuthNPolicy(tenantName, policy,
                this.getServiceContext());
    }

    /**
     * return RSA securID configuration for the tenant
     * @param tenantName
     * @param siteID
     * @return
     * @throws Exception
     */
    public RSAAgentConfig getRSAConfig(String tenantName) throws Exception {
        return getService()
                .getRSAConfig(tenantName, this.getServiceContext());
    }

    /**
     * set RSA securID configuration for the tenant
     * @param tenantName
     * @param rsaAgentConfig
     * @throws Exception
     */
    public void setRSAConfig(String tenantName, RSAAgentConfig rsaAgentConfig)
        throws Exception {
            getService().setRSAConfig(tenantName, rsaAgentConfig,
                    this.getServiceContext());
    }

    /**
     * add RSA securID instance configuration for a site
     * @param tenantName
     * @param instInfo
     * @throws Exception
     */
    public void addRSAInstanceInfo(String tenantName, RSAAMInstanceInfo instInfo) throws Exception {
            getService().addRSAInstanceInfo(tenantName, instInfo,this.getServiceContext());
    }

    /**
     * delete RSA securID instance configuration for a site
     * @param tenantName
     * @param siteID
     * @throws Exception
     */
    public void deleteRSAInstanceInfo(String tenantName, String siteID) throws Exception {
        getService().deleteRSAInstanceInfo(tenantName, siteID, this.getServiceContext());
    }

    /**
     * Retrieves fully configured IdentityManager.
     */
    private
    IIdentityManager
    getService() throws Exception
    {
        return identityManager;
    }

    /**
     * Queries the current diagnostics to fetch a correlationID, If the correlationId is not present in received request, then a new one is generated.
     *
     * @return Context information about IDM service which includes correlationId for diagnostics purpose
     */
    private IIdmServiceContext getServiceContext()
    {
        IIdmServiceContext serviceContext = null;
        if(this._serviceContextProvider != null)
        {
            serviceContext = this._serviceContextProvider.getServiceContext();
        }
        return serviceContext;
    }

    private synchronized Schema getSamlSchema() {
       if(schema==null) {
          try
          {
              SchemaFactory factory =
                      SchemaFactory
                      .newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
              ResourceResolver resolver = new ResourceResolver();
              factory.setResourceResolver(resolver);
              factory.setFeature(XMLConstants.FEATURE_SECURE_PROCESSING, false);
              try {
                  //After JDK 1.7.0_45, it's not possible to overwrite entityExpansionLimit through the above feature,
                  //if it's set as System property. So we have to set it explicitly as an API property.
                  factory.setProperty("http://www.oracle.com/xml/jaxp/properties/entityExpansionLimit",
                        "10000");
              }
              catch (Exception g) {
                   // The above call does not apply to xerces which does not recognize 'http://www.oracle.com/xml/jaxp/properties/entityExpansionLimit
                   // In this case we can ignore the exception since the limit does not apply anyway.
              }
              // load a schema, represented by a Schema instance
              Source schemaSource = new StreamSource(CasIdmClient.class.getResourceAsStream(SAML_SCHEMA_FILE));
              schema = factory.newSchema(schemaSource);

          } catch (SAXException e)
          {
              throw new AssertionError(e);
          }
       }
       return schema;
    }

}
