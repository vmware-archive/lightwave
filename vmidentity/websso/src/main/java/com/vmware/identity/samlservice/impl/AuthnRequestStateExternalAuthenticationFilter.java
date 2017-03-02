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

import java.security.NoSuchAlgorithmException;
import java.util.Collection;

import javax.servlet.http.HttpServletRequest;
import org.apache.commons.lang.Validate;
import org.opensaml.common.impl.SecureRandomIdentifierGenerator;
import org.opensaml.saml2.core.AuthnRequest;
import org.springframework.beans.factory.annotation.Autowired;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.proxyservice.LogonProcessorImpl;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.idm.IdmPrincipalAttributesExtractorFactory;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.DefaultIdmAccessorFactory;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SPConfiguration;
import com.vmware.identity.websso.client.SsoRequestSettings;
import com.vmware.identity.websso.client.endpoint.SsoRequestSender;

public class AuthnRequestStateExternalAuthenticationFilter implements
        AuthenticationFilter<AuthnRequestState> {
    @Autowired
    private MetadataSettings metadataSettings;

    @Autowired
    private SsoRequestSender ssoRequestSender;

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(AuthnRequestStateCookieWrapper.class);

    private volatile Thread idpMetadataSynchronizer  = null;

    private static SecureRandomIdentifierGenerator generator;

    static {
        try {
            generator = new SecureRandomIdentifierGenerator();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Unexpected error in creating SecureRandomIdentifierGenerator", e);
        }
    }

    /**
     * SAML Metadata refreshing thread refreshing at interval of 10 minutes (600000 milliseconds). The thread runs at service starts
     * and predetermined interval.
     * This thread dealt with following scenarios:
     * 1. IDP-Initiated SSO/SLO where no authentication request proceed the response to be received.
     * 2. SP or IDP Configuration changes. Without a STS reboot, the configuration for IDP_initiated flow should arrive at maximum delay
     *      of 10 minutes.
     */
    class SAMLMetadataSynchronizer extends Thread
    {
        private final Integer CheckInterval = 600000;   // Check every IDP configuration changes every 10 minitues
        private final DefaultIdmAccessorFactory factory;

        SAMLMetadataSynchronizer() {
            this.factory = new DefaultIdmAccessorFactory();
        }
        @Override
        public void run()
        {
                while(idpMetadataSynchronizer == Thread.currentThread())
                {
                    try
                    {
                        this.refreshMetadata();
                    }
                    catch (Throwable t)
                    {
                        log.error(String.format("IdpMetadataSynchronizer refresh tenant IDP failed : %s",
                                t.getMessage()), t);
                    }

                    try
                    {
                        Thread.sleep(CheckInterval);
                    }
                    catch (InterruptedException e)
                    {
                        log.error("IdpMetadataSynchronizer Thread is interrupted!");
                    }
                }

        }
        /**
         * Refresh all metadata configuration for WebSSO client library.
         * @throws Exception
         */
        private void refreshMetadata() throws Exception {

            IdmAccessor accessor = this.factory.getIdmAccessor();
            Collection<String> tenants = accessor.getAllTenants();

            metadataSettings.StartRebuilding();
            try {

                metadataSettings.clear();

                for (String tenant:tenants) {
                    accessor.setTenant(tenant);
                    Collection<IDPConfig> IdpConfigs = accessor.getExternalIdps();

                    if (null == IdpConfigs || IdpConfigs.isEmpty()) {
                        break;
                    }

                    //Add IDP configurations for the tenant
                    for (IDPConfig idpConfig : IdpConfigs)
                    {
                        IDPConfiguration clientLibIdpConfiguration =
                                SamlServiceImpl.generateIDPConfiguration(idpConfig);

                        metadataSettings.addIDPConfiguration(clientLibIdpConfiguration);
                    }

                    //Update the server's SP role for the tenant.
                    SPConfiguration spConfig = SamlServiceImpl.generateSPConfiguration(accessor);
                    metadataSettings.addSPConfiguration(spConfig);
                }

            } finally {
                metadataSettings.EndRebuilding();
            }

        }
    }

    public AuthnRequestStateExternalAuthenticationFilter() {
        this.idpMetadataSynchronizer = new SAMLMetadataSynchronizer();
        this.idpMetadataSynchronizer.start();
    }


    @Override
    public void preProcess(AuthnRequestState t) throws SamlServiceException {
        preAuthenticate(t);
    }

    @Override
    public void process(AuthnRequestState t) throws SamlServiceException {
        authenticate(t);
    }

    @Override
    public void preAuthenticate(AuthnRequestState t)
            throws SamlServiceException {
        log.debug("AuthnRequestStateExternalAuthenticationFilter.preAuthenticate is called");

        // check for session existence first
        Validate.notNull(t, "AuthnRequestState");
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request, "request");
        Validate.notNull(t.getIdmAccessor(), "idmAccessor");
    }

    @Override
    public void authenticate(AuthnRequestState t) throws SamlServiceException {
        log.debug("AuthnRequestStateExternalAuthenticationFilter.authenticate is called");
        Validate.isTrue(t.isProxying(),
                            "Not expected to use AuthnRequestStateExternalAuthenticationFilter!");
        try {
            IDPConfig extIDPConfig = t.getExtIDPToUse();
            Validate.notNull(extIDPConfig, "extIDPConfig");
            String spAlias = t.getIdmAccessor().getTenant();
            Validate.notEmpty(spAlias, "spAlias");

            t.setAuthnMethod(null);
            if (null == metadataSettings.getIDPConfigurationByEntityID(extIDPConfig.getEntityID())
                    || null == metadataSettings.getSPConfiguration(spAlias)) {
                SamlServiceImpl.initMetadataSettings(metadataSettings,extIDPConfig,t.getIdmAccessor() );
            }

            IDPConfiguration extIDPConfiguration = metadataSettings.getIDPConfigurationByEntityID(extIDPConfig.getEntityID());
            Validate.notNull(extIDPConfiguration, "extIDPConfiguration");

            String idpAlias = extIDPConfiguration.getAlias();
            Validate.notEmpty(idpAlias, "idpAlias");

            SPConfiguration spConfig = metadataSettings
                    .getSPConfiguration(spAlias);
            Validate.notNull(spConfig,
                    "VMWareSSO-as-SP role metadata was missing in client lib \"metadataSettins\"");
            String acsUrl = spConfig.getAssertionConsumerServices().get(0)
                    .getLocation();

            SsoRequestSettings extSSORequestSetting = new SsoRequestSettings(
                    spAlias,
                    idpAlias,
                    true,
                    OasisNames.IDENTITY_FORMAT_UPN,
                    true,  //allowProxy
                    false, // force authentication
                    false, // passive mode
                    null, // ACSIndex
                    acsUrl,
                    null  //relayState
                    );
            //ADFS specific settings
            extSSORequestSetting.setAllowScopingElement(false);
            extSSORequestSetting.setAllowAllowCreateInNameIDPolicy(false);
            extSSORequestSetting.setAllowSPNameQualifierInNameIDPolicy(false);
            extSSORequestSetting.setAllowRequestAuthnContext(false);

            //set proxy count.  We should set this according to spec.
            // But ADFS does not seems to support Scoping in authnrequest
            extSSORequestSetting.setProxyCount(this.getExtRequestProxyCount( t));

            LogonProcessorImpl logonProcessorImpl = (LogonProcessorImpl) getSsoRequestSender().getLogonProcessor();
            String outGoingReqID = generator.generateIdentifier();
            String redirectUrl = getSsoRequestSender().getRequestUrl(extSSORequestSetting, outGoingReqID);

            logonProcessorImpl.registerRequestState(t.getAuthnRequest().getID(),outGoingReqID, t);

            if (t.getTenantIDPSelectHeader() != null) {
                t.getResponse().addHeader(Shared.IDP_SELECTION_REDIRECT_URL, redirectUrl);
            } else {
                t.getResponse().sendRedirect(redirectUrl);
            }
        } catch (Exception e) {
            // failed to authenticate with via proxying.
            log.warn("Caught exception in authenticate via external IDP: {}", e.getMessage());
            ValidationResult vr = new ValidationResult(
                    OasisNames.RESPONDER,
                    "ExternalIDPAuthentication invocation failure.");
            t.setValidationResult(vr);
            throw new SamlServiceException(e);
       }
    }


    //Set delegated proxyAccount in request.  The value should be that from original request minus 1 if set.
    private Integer getExtRequestProxyCount(AuthnRequestState t) {
        Validate.notNull(t, "AuthnRequestState");
        Integer outProxyCount = null;
        Integer inProxyCount = t.getProxyCount();

        if (inProxyCount != null)
            outProxyCount = inProxyCount > 0? inProxyCount - 1:0;

        return outProxyCount;
    }


    private SsoRequestSender getSsoRequestSender() {
        return ssoRequestSender;
    }

     @Override
    public PrincipalAttributesExtractorFactory getPrincipalAttributeExtractorFactory(
            String idmHostName) {
        return new IdmPrincipalAttributesExtractorFactory(idmHostName);
    }

    @Override
    public ConfigExtractorFactory getConfigExtractorFactory(String idmHostName) {
        return new ConfigExtractorFactoryImpl();
    }

}
