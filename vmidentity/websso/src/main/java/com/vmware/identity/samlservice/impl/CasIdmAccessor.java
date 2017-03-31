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
package com.vmware.identity.samlservice.impl;

import java.security.PrivateKey;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

import org.apache.commons.lang.Validate;
import org.w3c.dom.Document;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AssertionConsumerService;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.Group;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.SSOImplicitGroupNames;
import com.vmware.identity.idm.ServiceEndpoint;
import com.vmware.identity.idm.TokenClaimAttribute;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.ServerValidatableSamlToken.Subject;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.websso.client.Attribute;

/**
 * IDM Accessor class which talks to real IDM system (by wrapping CasIdmClient)
 *
 */
public class CasIdmAccessor implements IdmAccessor {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(CasIdmAccessor.class);

    private final CasIdmClient client;
    private String tenant;

    private static final char[] invalidCharsForUserName;
    private static final String UPNFormat = "http://schemas.xmlsoap.org/claims/UPN";
    private static final String UPNSeparator = "@";
    private static final String usernameDelimiter = "-";

    static {
        char[] invalidCharsForUserDetail = "^<>&%`".toCharArray();
        char upnSeparator = '@';
        char netbiosSeparator = '\\';
        invalidCharsForUserName =
             (String.valueOf(invalidCharsForUserDetail) + upnSeparator + netbiosSeparator)
                   .toCharArray();
    }

    /**
     * Create IDM Accessor with an instance of the IDM client
     *
     * @param idmClient
     */
    public CasIdmAccessor(CasIdmClient idmClient) {
        logger.debug("CasIdmAccessor constructor called");

        Validate.notNull(idmClient);
        client = idmClient;
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.samlservice.IdmAccessor#setDefaultTenant()
     */
    @Override
    public void setDefaultTenant() {
        logger.debug("setDefaultTenant called");
        try {
            String defaultTenant = client.getDefaultTenant();
            Validate.notNull(defaultTenant);
            setTenant(defaultTenant);
        } catch (Exception e) {
            logger.error("setDefaultTenant: Caught exception {}", e.toString());
            throw new IllegalStateException("BadRequest", e);
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.samlservice.IdmAccessor#setTenant(java.lang.String)
     */
    @Override
    public void setTenant(String t) {
        logger.debug("setTenant: {}", t);
        tenant = t;
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.samlservice.IdmAccessor#getTenant()
     */
    @Override
    public String getTenant() {
        logger.debug("getTenant: {}", tenant);
        return tenant;
    }

    /*
     * (non-Javadoc)
     *
     * @see com.vmware.identity.samlservice.IdmAccessor#getIdpEntityId()
     */
    @Override
    public String getIdpEntityId() {
        logger.debug("getIdpEntityId");
        String retval = null;

        try {
            retval = client.getEntityID(tenant);
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException("BadRequest", e);
        }

        return retval;
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.samlservice.IdmAccessor#getAcsForRelyingParty(java
     * .lang.String, int, java.lang.String)
     */
    @Override
    public String getAcsForRelyingParty(String relyingParty, Integer acsIndex,
            String acsUrl, String binding, boolean validateWithMetadata) {
        logger.debug("getAcsForRelyingParty " + relyingParty + ", index "
                + acsIndex + ", URL " + acsUrl + ", binding " + binding);
        String retval = null;

        try {
            RelyingParty rp = client.getRelyingPartyByUrl(tenant, relyingParty);
            Validate.notNull(rp);
            Collection<AssertionConsumerService> assertionServices = rp
                    .getAssertionConsumerServices();
            Validate.notNull(assertionServices);
            Validate.isTrue(assertionServices.size() > 0);
            AssertionConsumerService[] services = assertionServices
                    .toArray(new AssertionConsumerService[assertionServices
                            .size()]);

            if (acsIndex != null) {
                // if index is present, URL must not be
                if (acsUrl != null) {
                    throw new IllegalStateException("BadRequest.AssertionIndex");
                }
                if (acsIndex < 0 || acsIndex >= assertionServices.size()) {
                    throw new IllegalStateException("BadRequest.AssertionIndex");
                }
                Validate.notNull(services[acsIndex]);
                retval = services[acsIndex].getEndpoint();
            } else if (acsUrl != null) {
                // we have no index specified, URL is present
                // find assertion consumer service by URL
                if (validateWithMetadata) {
                    for (AssertionConsumerService acs : assertionServices) {
                        if (acs != null && acsUrl.equals(acs.getEndpoint())) {
                            // check binding if specified
                            if (binding == null
                                    || (binding != null && binding.equals(acs
                                            .getBinding()))) {
                                retval = acs.getEndpoint();
                            }
                        }
                    }
                } else { // no validation
                    retval = acsUrl;
                }
                // by now we should have found something
                if (retval == null) {
                    throw new IllegalStateException(
                            "BadRequest.AssertionMetadata");
                }
            } else if (binding != null) {
                // we have to index or URL specified, lookup by binding
                for (AssertionConsumerService acs : assertionServices) {
                    if (acs != null && binding.equals(acs.getBinding())) {
                        retval = acs.getEndpoint();
                    }
                }
                // by now we should have found something
                if (retval == null) {
                    throw new IllegalStateException(
                            "BadRequest.AssertionBinding");
                }
            } else {
                // just look for the default service if any
                for (AssertionConsumerService acs : assertionServices) {
                    if (acs != null
                            && acs.getName() != null
                            && acs.getName().equals(
                                    rp.getDefaultAssertionConsumerService())) {
                        retval = acs.getEndpoint();
                    }
                }
                // by now we should have found something
                if (retval == null) {
                    throw new IllegalStateException(
                            "BadRequest.AssertionNoDefault");
                }
            }
        } catch (IllegalStateException e) {
            logger.error("Caught illegal state exception {}", e.toString());
            throw e;
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException("BadRequest", e);
        }

        Validate.notNull(retval);
        return retval;
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.samlservice.IdmAccessor#getCertificatesForRelyingParty
     * (java.lang.String)
     */
    @Override
    public CertPath getCertificatesForRelyingParty(String relyingParty) {
        logger.debug("getCertificatesForRelyingParty {}" , relyingParty);
        List<X509Certificate> certificates = new ArrayList<X509Certificate>();
        // only query relying party if it's not null
        // simply return an empty chain for 'null' relying party
        if (relyingParty != null) {
            try {
                // TODO support more than one cert
                RelyingParty rp = client.getRelyingPartyByUrl(tenant,
                        relyingParty);
                Validate.notNull(rp);
                Certificate c = rp.getCertificate();
                Validate.notNull(c);
                certificates.add((X509Certificate) c);
            } catch (Exception e) {
                logger.error("Caught exception ", e);
            }
        }
        try {
            CertificateFactory certFactory = CertificateFactory
                    .getInstance("X.509");
            CertPath certPath = certFactory.generateCertPath(certificates);
            return certPath;
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public List<Certificate> getSAMLAuthorityChain() {
        logger.debug("getSAMLAuthorityChain");
        List<Certificate> certs = Collections.emptyList();
        try {
            certs = client.getTenantCertificate(tenant);
        } catch (Exception e) {
            logger.error("Caught exception ", e);
        }
        Validate.notEmpty(certs);
        return certs;
    }

    @Override
    public Collection<List<Certificate>> getSAMLAuthorityChains() {
        logger.debug("getSAMLAuthorityChains");
        Collection<List<Certificate>> allChains = Collections.emptyList();
        try {
            allChains = client.getTenantCertificates(tenant);
        } catch (Exception e) {
            logger.error("Caught exception ", e);
        }
        Validate.notEmpty(allChains);
        return allChains;
    }

    @Override
    public PrivateKey getSAMLAuthorityPrivateKey() {
        logger.debug("getSAMLAuthorityPrivateKey");

        try {
            return client.getTenantPrivateKey(tenant);
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public long getClockTolerance() {
        logger.debug("getClockTolerance");

        try {
            return client.getClockTolerance(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public String getTenantSignatureAlgorithm() {
        logger.debug("getTenantSignatureAlgorithm");

        try {
            return client.getTenantSignatureAlgorithm(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public long getMaximumBearerTokenLifetime() {
        logger.debug("getMaximumBearerTokenLifetime");

        try {
            return client.getMaximumBearerTokenLifetime(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public long getMaximumHoKTokenLifetime() {
        logger.debug("getMaximumHoKTokenLifetime");

        try {
            return client.getMaximumHoKTokenLifetime(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public int getDelegationCount() {
        logger.debug("getDelegationCount");

        try {
            return client.getDelegationCount(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public int getRenewCount() {
        logger.debug("getRenewCount");

        try {
            return client.getRenewCount(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
	public void incrementGeneratedTokens(String tenant) {
		logger.debug("incrementGeneratedTokens");

		try {
			client.incrementGeneratedTokens(tenant);
		} catch (Exception e) {
			logger.error("Caught exception ", e);
		}
	}

    @Override
    public GSSResult authenticate(String contextId, byte[] decodedAuthData) {
        logger.debug("kerb authenticate");

        try {
            return client.authenticate(tenant, contextId, decodedAuthData);
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException(e);
        }

    }

    @Override
    public String getDefaultIdpEntityId() {
        logger.debug("getDefaultIdpEntityId");
        String retval = null;

        try {
            retval = getIdpEntityId();
            if (retval.endsWith(tenant)) {
                // effectively trim "/{tenant} from the end
                retval = retval.substring(0, retval.length() - tenant.length()
                        - 1);
            }
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException("BadRequest", e);
        }

        return retval;
    }

    @Override
    public CasIdmClient getIdmClient() {
        return client;
    }

    @Override
    public boolean getAuthnRequestsSignedForRelyingParty(String relyingParty) {
        logger.debug("getAuthnRequestsSignedForRelyingParty " + relyingParty);
        boolean retval = false;
        try {
            RelyingParty rp = client.getRelyingPartyByUrl(tenant, relyingParty);
            Validate.notNull(rp);
            retval = rp.isAuthnRequestsSigned();
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException(e);
        }
        return retval;
    }

    @Override
    public PrincipalId authenticate(String username, String password) {
        logger.debug("password authenticate");

        try {
            return client.authenticate(tenant, username, password);
        } catch (Exception e) {
            logger.error("Caught exception. ", e);
            throw new IllegalStateException(e);
        }

    }

	@Override
	public PrincipalId authenticate(X509Certificate[] tLSCertChain, String hint) {
        try {
            return client.authenticate(tenant, tLSCertChain, hint);
        } catch (Exception e) {
            logger.error("Caught exception. ", e);
            throw new IllegalStateException(e);
        }
	}

    @Override
    public RSAAMResult authenticatebyPasscode(String rsaSessionId,
            String username, String passcode) throws IDMSecureIDNewPinException {
        logger.debug("rsa secureID authenticate");

        try {
            return client.authenticateRsaSecurId(tenant, rsaSessionId,
                    username, passcode);
        } catch (IDMSecureIDNewPinException e) {
            logger.error("New pin required.", e);
            throw e;
        } catch (Exception e) {
            logger.error("Caught exception. ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public String getSloForRelyingParty(String relyingParty, String binding)
            throws IllegalStateException {
        logger.debug("getSloForRelyingParty " + relyingParty + ", binding "
                + binding);
        String retval = null;
        Validate.notNull(binding);

        try {
            RelyingParty rp = client.getRelyingPartyByUrl(tenant, relyingParty);
            Validate.notNull(rp);
            Collection<ServiceEndpoint> sloServices = rp
                    .getSingleLogoutServices();

            // SLO service is optional and if it does not exist or binding does not match, return null.
            if (sloServices != null && sloServices.size() > 0) {
                // lookup by binding
                for (ServiceEndpoint slo : sloServices) {
                    if (slo != null && binding.equals(slo.getBinding())) {
                        retval = slo.getResponseEndpoint();
                        if (retval == null || retval.isEmpty()) {
                            retval = slo.getEndpoint();
                        }
                    }
                }
                // by now we should have found something
                if (retval == null) {
                    logger.warn(String.format("SLO service for relying party %s exists, but does not support %s binding.",
                            relyingParty, binding));
                }
            } else {
                logger.warn(String.format("SLO service for relying party %s does not exist.", relyingParty));
            }
            return retval;
        } catch (IllegalStateException e) {
            throw e;
        } catch (Exception e) {
            throw new IllegalStateException("BadRequest", e);
        }
    }

    @Override
    public String exportConfigurationAsString() {
        logger.debug("export configuration");

        try {
            Document doc = client.getSsoSaml2Metadata(tenant);
            return Shared.getStringFromDocument(doc);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(e);
        }
    }

    @Override
    public String getIdpSsoEndpoint() {
        logger.debug("getIdpSsoEndpoint");
        String retval = null;

        try {
            retval = client.getEntityID(tenant).replace("/Metadata", "/SSO");
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException("BadRequest", e);
        }

        return retval;
    }

    @Override
    public String getIdpSloEndpoint() {
        logger.debug("getIdpSloEndpoint");
        String retval = null;

        try {
            retval = client.getEntityID(tenant).replace("/Metadata", "/SLO");
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException("BadRequest", e);
        }

        return retval;
    }

    @Override
    public String getDefaultIdpSsoEndpoint() {
        logger.debug("getDefaultIdpEntityId");
        String retval = null;

        try {
            retval = getIdpEntityId();
            if (retval.endsWith(tenant)) {
                // effectively trim "/{tenant} from the end
                retval = retval.substring(0, retval.length() - tenant.length()
                        - 1);
            }
            // change to SSO endpoint
            retval = retval.replace("/Metadata", "/SSO");
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException("BadRequest", e);
        }

        return retval;
    }

    @Override
    public Collection<IDPConfig> getExternalIdps() {
        logger.debug("getExternalIdps");
        Collection<IDPConfig> idps = Collections.emptyList();
        try {
            idps = client.getAllExternalIdpConfig(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException("BadRequest", e);
        }
        return idps;
    }

    @Override
    public String getBrandName() {
        logger.debug("getBrandName");

        try {
            return client.getBrandName(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ",e);
            throw new IllegalStateException(
                    "Failed to return tenant brand name for: " + tenant, e);
        }
    }

    @Override
    public String getLogonBannerContent() {
        logger.debug("getLogonBannerContent");
        try {
            return client.getLogonBannerContent(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception " + e.toString());
            throw new IllegalStateException("Failed to return tenant logon banner content for: " + tenant, e);
        }
    }

    @Override
    public String getLogonBannerTitle() {
        logger.debug("getLogonBannerTitle");
        try {
            return client.getLogonBannerTitle(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception " + e.toString());
            throw new IllegalStateException("Failed to return tenant logon banner title for: " + tenant, e);
        }
    }

    @Override
    public boolean getLogonBannerCheckboxFlag() {
        logger.debug("getLogonBannerCheckboxFlag");
        try {
            return client.getLogonBannerCheckboxFlag(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception " + e.toString());
            throw new IllegalStateException("Failed to return tenant logon banner checkbox for: " + tenant, e);
        }
    }

    @Override
    public List<Certificate> getTenantCertificate() {
        logger.debug("getTenantCertificate");
        try {
            return client.getTenantCertificate(tenant);
        } catch (Exception e) {
            logger.debug("Caught exception ",e);
            throw new IllegalStateException(
                    "Failed to return tenant signing cert for: " + tenant, e);
        }
    }

	@Override
	public IDPConfig getExternalIdpConfigForTenant(String tenant,
			String providerID) {
	    try {
	        return client.getExternalIdpConfigForTenant(tenant, providerID);
	    } catch (Exception e) {
            logger.debug("Caught exception ",e);
            throw new IllegalStateException(
                    "Failed to return external IDP configuration. provider: " + providerID, e);
        }
	}
    @Override
    public RelyingParty getRelyingPartyByUrl(String rpEntityId) {
        logger.debug("getRelyingPartyByUrl");

        try {
            return client.getRelyingPartyByUrl(tenant, rpEntityId);
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(
                    "Failed to return find relying party: " + rpEntityId, e);
        }
    }

    @Override
    public String getServerSPN() {
        try {
            return client.getServerSPN();
        } catch (Exception e) {
            logger.error("Caught exception ", e);
            throw new IllegalStateException("Failed to get server SPN", e);
        }

    }

    @Override
    public PrincipalId createUserAccountJustInTime(Subject subject, String tenant,
            IDPConfig extIdp, Collection<Attribute> claimAttributes) throws Exception {
        if (subject == null) {
            throw new InvalidTokenException("The subject retrieved from external token is null.");
        }

        // retrieve system domain
        EnumSet<DomainType> domains = EnumSet.of(DomainType.SYSTEM_DOMAIN);
        Iterator<IIdentityStoreData> iter = client
                .getProviders(tenant, domains).iterator();
        String systemDomain = iter.next().getName();
        PrincipalId subjectUpn = null;
        String userName = null;
        String upnSuffix = null;
        String extUserId = null;

        if (subject.subjectUpn() != null) {
            subjectUpn = subject.subjectUpn();
            upnSuffix = subjectUpn.getDomain();
            // generate a unique user name in order not to conflict with local users
            userName = subjectUpn.getName() + usernameDelimiter + upnSuffix;
            extUserId = subjectUpn.getUPN();
        } else {
            // to support non-upn subject format in external token
            // look for upn from token attributes
            extUserId = findUPNInTokenAttributes(claimAttributes);
            if (extUserId != null) {
                int pos = extUserId.indexOf(UPNSeparator);
                if (pos > 0) {
                    upnSuffix = extUserId.substring(pos + 1);
                    // generate a unique user name in order not to conflict with local users
                    userName = extUserId.substring(0, pos) + usernameDelimiter + upnSuffix;
                    // subject upn is the same as the upn is external token attribute
                    subjectUpn = new PrincipalId(extUserId.substring(0, pos), upnSuffix);
                } else {
                    // if the upn attribute is not in correct format
                    throw new IllegalStateException("Invalid UPN received from external token: " + extUserId);
                }
            } else {
                // upn is not found from token attributes
                String nameId = subject.subjectNameId().getName();
                // compose user name as sanitizedExternalID.GUID
                userName = sanitizeSubjectNameIdForUserName(nameId) + usernameDelimiter + UUID.randomUUID().toString();
                upnSuffix = extIdp.getUpnSuffix();
                if (upnSuffix == null || upnSuffix.isEmpty()) {
                    throw new IllegalStateException("UPN suffix is not set for external IDP: " + extIdp.getEntityID());
                }
                subjectUpn = new PrincipalId(userName, upnSuffix);
                extUserId = nameId;
        	}
        }

        // register upn suffix to system domain
        client.registerUpnSuffix(tenant, systemDomain, upnSuffix);

        logger.info("Creating a temporary user account for the user {} with domain {} "
                + "in VMware identity store since the user is not found "
                + "during delegated logon via SAML IDP federation.",
                subjectUpn.getUPN(), tenant);
        return client.addJitUser(
                tenant,
                userName,
                new PersonDetail.Builder()
                        .userPrincipalName(subjectUpn.getUPN())
                        .description(
                                "A JIT user account created for external IDP.").build(),
                                extIdp.getEntityID(),
                                extUserId);
    }

    private String sanitizeSubjectNameIdForUserName(String nameId) {
        String sanitizedNameId = nameId;
        int pos = nameId.indexOf(UPNSeparator);
        if (pos > 0) {
            sanitizedNameId = nameId.substring(0, pos);
        }
        for (char c : invalidCharsForUserName) {
            sanitizedNameId.replace(String.valueOf(c), "#");
        }

        return sanitizedNameId;
    }

    private String findUPNInTokenAttributes(Collection<Attribute> claimAttributes) {
        if (claimAttributes != null) {
            for (Attribute attr : claimAttributes) {
                if (UPNFormat.equalsIgnoreCase(attr.getName())) {
                    return attr.getValues().iterator().next();
                }
            }
        }
        return null;
    }

    @Override
    public void updateJitUserGroups(PrincipalId subjectUpn, String tenant, Map<TokenClaimAttribute,List<String>> mappings, Collection<Attribute> claimAttributes) throws Exception {
        Set<Group> currentGroups = client.findDirectParentGroups(tenant, subjectUpn);
        if (currentGroups == null) {
            currentGroups = new HashSet<>();
        }
        Set<Group> newGroups = new HashSet<>();

        if (mappings != null && claimAttributes != null) {
            for (Attribute attr : claimAttributes) {
                String attrName = attr.getName();
                for (String attrValue : attr.getValues()) {
                    TokenClaimAttribute tokenClaim = new TokenClaimAttribute(attrName, attrValue);
                    List<String> groups = mappings.get(tokenClaim);
                    if (groups == null || groups.isEmpty()) {
                        continue;
                    }
                    for (String groupSid : groups) {
                        try {
                            newGroups.add(client.findGroupByObjectId(tenant, groupSid));
                        } catch (Exception e) {
                            logger.error("Failed to find group with sid " + groupSid, e);
                        }
                    }
                }
            }

            for (Group g : newGroups) {
                if (!currentGroups.contains(g)) {
                    try {
                        client.addUserToGroup(tenant, subjectUpn, g.getName());
                        logger.debug("User {} added to group{}s in tenant {}",
                                subjectUpn.getUPN(), g.getName(), tenant);
                    } catch (Exception e) {
                        logger.error(String.format("Failed to add user %s to group %s in tenant %s. "
                                + "Continue updating user group membership...",
                                subjectUpn.getUPN(), g.getName(), tenant), e);
                    }
                }
            }
        }

        for (Group g : currentGroups) {
            if (!newGroups.contains(g) && !g.getName().equalsIgnoreCase(SSOImplicitGroupNames.getEveryoneGroupName())) {
                try {
                    client.removeFromLocalGroup(tenant, subjectUpn, g.getName());
                } catch (Exception e) {
                    logger.error(String.format("Failed to remove user %s from group %s in tenant %s. "
                            + "Continue updating user group membership...",
                            subjectUpn.getUPN(), g.getName(), tenant), e);
                }
            }
        }
    }

    @Override
    public boolean isJitEnabledForExternalIdp(String tenantName, String entityId) {
        try {
            return client.getExternalIdpConfigForTenant(tenantName, entityId).getJitAttribute();
        } catch (Exception e) {
            logger.debug("Caught exception ", e);
            throw new IllegalStateException(
                    String.format("Failed to return jit attribute for idp: %s for tenant %s.", entityId, tenantName), e);
        }
    }
	@Override
    public AuthnPolicy getAuthnPolicy(String tenantName) {
	    try {
	        return client.getAuthnPolicy(tenantName);
        } catch (Exception e) {
            throw new IllegalStateException(
                    String.format("Failed to return authentication policy object: for tenant %s.", tenantName), e);

        }
    }

	@Override
	public boolean getTenantIDPSelectionFlag(String tenantName) {
		try {
			return client.isTenantIDPSelectionEnabled(tenantName);
		} catch (Exception e) {
            throw new IllegalStateException(
                    String.format("Failed to return idp selection flag: for tenant %s.", tenantName), e);

        }
	}

	@Override
	public String getIDPAlias(String tenantName, String entityId) {
		try {
			if (this.getIdpEntityId().equals(entityId)) {
				return client.getLocalIDPAlias(tenantName);
			}
			return client.getExternalIDPAlias(tenantName, entityId);
		} catch (Exception e) {
            throw new IllegalStateException(
                    String.format("Failed to return idp display name: for idp %s tenant %s.", entityId, tenantName), e);

        }
	}

    @Override
    public Collection<String> getAllTenants() throws Exception {
        try {
            return client.getAllTenants();
        }catch (Exception e) {
            throw new IllegalStateException(
                    "Failed to return all tenant names.", e);
        }
    }

    @Override
    public Collection<RelyingParty> getRelyingParties(String tenant) {

        try {
            return client.getRelyingParties(tenant);
        } catch (Exception e) {
            throw new IllegalStateException(
                    "Failed to return relying part configurations for the tenant.", e);
        }
    }
}
