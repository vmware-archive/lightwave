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

package com.vmware.identity.samlservice;

import java.security.PrivateKey;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.List;
import java.util.Map;

import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.ServerValidatableSamlToken.Subject;
import com.vmware.identity.websso.client.Attribute;

/**
 *
 * Interface that we will be using to access IDM.
 * We can plug in a demo implementation or a real IDM implementation depending on our needs.
 *
 */
public interface IdmAccessor {

	/**
	 * Specifies that we need to use default tenant
	 * (use getTenant() to get the name of that tenant)
	 */
	void setDefaultTenant();

	/**
	 * Sets the tenant we're working with
	 * @param tenant
	 */
	void setTenant(String tenant);

	/**
	 * Returns the current tenant
	 * @return
	 */
	String getTenant();

    /**
     * Returns all tenants
     * @return
     */
    Collection<String> getAllTenants() throws Exception;

	/**
	 * Returns entity id of the IdP (we can use it as myId parameter when generating SAML Responses)
	 * @return
	 */
	String getIdpEntityId();

    /**
     * Returns SSO endpoint of the IdP
     * @return
     */
    String getIdpSsoEndpoint();

    /**
     * Returns SLO endpoint of the IdP
     * @return
     */
    String getIdpSloEndpoint();

    /**
	 * Returns entity id of the IdP for default tenant (this is used in request validation)
	 * @return
	 */
	String getDefaultIdpEntityId();

    /**
     * Returns SSO endpoint of the IdP for default tenant (this is used in request validation)
     * @return
     */
    String getDefaultIdpSsoEndpoint();

    /**
     * Returns URL of Assertion Consumer Service for the specified Service
     * Provider by its index/url/binding. On error, will throw
     * IllegalStateException with a message code (load message text from
     * messages.properties)
     *
     * @param relyingParty
     * @param acsIndex
     *            // either index, url or binding is needed to identify the
     *            endpoint
     * @param acsUrl
     * @param binding
     * @param validateWithMetadata
     *            //Used when acsUrl is provided. whether to validate with
     *            registered relying party ACS
     * @return
     */
	String getAcsForRelyingParty(
			String relyingParty,
			Integer acsIndex,
			String acsUrl,
 String binding, boolean validateWithMetadata)
            throws IllegalStateException;

    /**
     * Returns URL of Single Logout Service for the specified Service Provider by its binding.
     * On error, will throw IllegalStateException with a message code (load message text from messages.properties)
     * @param relyingParty
     * @param binding
     * @return
     */
    String getSloForRelyingParty(
            String relyingParty,
            String binding) throws IllegalStateException;

    /**
	 * Returns a cert chain configured for the Service Provider
	 * @param relyingParty
	 * @return
	 */
	CertPath getCertificatesForRelyingParty(String relyingParty);

	/**
	 * Return a cert chain configured for current tenant
	 * @return
	 */
	List<Certificate> getSAMLAuthorityChain();

   /**
    * Return all cert chains configured for current tenant
    * @return
    */
   Collection<List<Certificate>> getSAMLAuthorityChains();

	/**
	 * Return private key configured for current tenant
	 * @return
	 */
	PrivateKey getSAMLAuthorityPrivateKey();

	/**
	 * Return clock tolerance setting (in milliseconds) for current tenant
	 * @return
	 */
	long getClockTolerance();

	/**
	 * Return maximum lifetime setting for BEARER tokens (in milliseconds) for current tenant
	 * @return
	 */
	long getMaximumBearerTokenLifetime();

	/**
	 * Return maximum lifetime setting for HoK tokens (in milliseconds) for current tenant
	 * @return
	 */
	long getMaximumHoKTokenLifetime();

	/**
	 * Returns delegation count, which is how many times token can be delegated, for current tenant
	 * @return
	 */
	int getDelegationCount();

	/**
	 * Returns renew count, which is how many times token can be renewed, for current tenant
	 * @return
	 */
	int getRenewCount();

	/**
	 * Increments the number of generated tokens for a given tenant.
	 */
	void incrementGeneratedTokens(String tenant);

	/**
	 * Issue Authentication call for the current tenant with the specified username and password
	 * @param username
	 * @param password
	 * @return
	 */
	PrincipalId authenticate(String username, String password);

	/**
	 * Issue Authentication call for the current tenant with the specified auth data
	 * @param decodedAuthData
	 * @return
	 */
	GSSResult authenticate(String contextId, byte[] decodedAuthData);

	/**
	 * Issue Authentication call for the current tenant with the TLS Client certificate chain
	 * @param tLSCertChain
	 * @param hint   optional user hint (userNameHint@userdomain)
	 * @return
	 */
	PrincipalId authenticate(X509Certificate[] tLSCertChain, String hint);

	    /**
     * Issue Authentication call for the current tenant with the specified
     * username and passcode
     *
     * @param rsaSessionID
     * @param username
     * @param passcode
     * @param
     * @return
     * @throws IDMSecureIDNewPinException  need new pin error
     */
    RSAAMResult authenticatebyPasscode(String rsaSessionID, String username,
            String passcode) throws IDMSecureIDNewPinException;

	/**
	 * Returns underlying CasIdmClient object.
	 * Note that this may return null (if we're not talking to real IDM)
	 * @return
	 */
	CasIdmClient getIdmClient();

	/**
	 * Return AuthnRequestsSigned flag for a relying party (specified as url)
	 * @param relyingParty
	 * @return
	 */
	boolean getAuthnRequestsSignedForRelyingParty(String relyingParty);

	/**
	 * Return the algorithm which should be used for signing token
	 * @return
	 */
	String getTenantSignatureAlgorithm();

	/**
	 * Return IDP configuration as a string
	 * @return
	 */
	String exportConfigurationAsString();

    /**
     * Return all external idps for the tenant.
     * @return
     */
    Collection<IDPConfig> getExternalIdps();

    /**
     * Return brand name for the tenant.
     * @return
     */
     String getBrandName();

     /**
      * Return logon banner content for the tenant.
      * @return
      */
      String getLogonBannerContent();

      /**
       * Return logon banner title
       * @return
       */
      String getLogonBannerTitle();

      /**
       * Check if logon banner checkbox is enabled.
       * @return
       */
      boolean getLogonBannerCheckboxFlag();

      boolean isJitEnabledForExternalIdp(String tenantName, String entityId);

     /**
      * Return signing certificate for the tenant.
      * @return
      */
    List<Certificate> getTenantCertificate();

    IDPConfig getExternalIdpConfigForTenant(String tenant, String providerID);

     /**
      * Search a relying party by url (emtityID).
      * @return
      */
     RelyingParty getRelyingPartyByUrl(String rpEntityId);

    /**
     * return SPN to pass to login page.
     *
     * @return
     */
    String getServerSPN();

     /**
      * Creates a JIT user account associated with the external IDP entityID
      * if the user is not found during delegated logon via SAML IDP federation.
      * JIT users are instances of vmwExternalIdpUser auxiliary objectclass in LDAP.
      *
      * If the subject nameId format is UPN or UPN is sent as a token attribute,
      * we use the UPN in the external token as JIT user's UPN in local vmdir.
      *
      * If UPN cannot be found from the external token, we generate UPN in the following format:
      * sanitizedExternalNameID.GUID@UPNSuffix
      * UPNSuffix is an attribute of IDPConfig, which is configured by admin.
      *
      *@param subject the user subject
      *@param tenant name of the tenant
      *@param extIdp the configuration of external IDP where the user is from.
      *@param claimAttributes claim attributes from external saml token
      *@return id of the created jit user.
      */
     PrincipalId createUserAccountJustInTime(Subject subject, String tenant,
             IDPConfig extIdp, Collection<Attribute> claimAttributes) throws Exception;

     /**
      * Update Jit user's groups given the token attributes.
      *
      * @param subjectUpn upn of the subject
      * @param tenant name of the tenant
      * @param mappings claim group mappings from externalIdpConfig
      * @param claimAttributes claim attributes from the saml token
      * @throws Exception
      */
     void updateJitUserGroups(PrincipalId subjectUpn, String tenant, Map<TokenClaimAttribute,List<String>> mappings, Collection<Attribute> claimAttributes) throws Exception;

    /**
	 * @return AuthnPolicy
	 * @param tenantName
	 */
	AuthnPolicy getAuthnPolicy(String tenantName);

    /**
     * @return if the tenant turned on IDPSelection dialog
     * @param tenantName
     */
	boolean getTenantIDPSelectionFlag(String tenantName);

    /**
     * @return IDP alias name of the IDP
     * @param tenantName
     * @param entityId
     */
	String getIDPAlias(String tenantName, String entityId);

    /**
     * @return relying parties for the tenant
     * @param tenant
     */
    Collection<RelyingParty> getRelyingParties(String tenant);

}
