/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client.endpoint;

import java.io.IOException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.util.List;

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.joda.time.DateTime;
import org.opensaml.common.SAMLVersion;
import org.opensaml.common.impl.SecureRandomIdentifierGenerator;
import org.opensaml.saml2.core.AuthnContextClassRef;
import org.opensaml.saml2.core.AuthnContextComparisonTypeEnumeration;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Conditions;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.NameIDPolicy;
import org.opensaml.saml2.core.RequestedAuthnContext;
import org.opensaml.saml2.core.Scoping;
import org.opensaml.saml2.core.impl.AuthnContextClassRefBuilder;
import org.opensaml.saml2.core.impl.AuthnRequestBuilder;
import org.opensaml.saml2.core.impl.ConditionsBuilder;
import org.opensaml.saml2.core.impl.IssuerBuilder;
import org.opensaml.saml2.core.impl.NameIDPolicyBuilder;
import org.opensaml.saml2.core.impl.RequestedAuthnContextBuilder;
import org.opensaml.saml2.core.impl.ScopingBuilder;
import org.opensaml.xml.ConfigurationException;
import org.opensaml.xml.io.MarshallingException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import com.vmware.identity.saml.ext.DelegableType;
import com.vmware.identity.saml.ext.RenewableType;
import com.vmware.identity.saml.ext.impl.DelegableTypeBuilder;
import com.vmware.identity.saml.ext.impl.RenewableTypeBuilder;
import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.LogonProcessor;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MessageType;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SPConfiguration;
import com.vmware.identity.websso.client.SamlNames;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SharedUtils;
import com.vmware.identity.websso.client.SsoRequestSettings;
import com.vmware.identity.websso.client.WebssoClientException;

/**
 * SSO endpoint component responsible for constructing authentication request to
 * the IDP.
 *
 */
@Component
public class SsoRequestSender {

    private static Logger logger = LoggerFactory.getLogger(SsoRequestSender.class);
    private final SecureRandomIdentifierGenerator generator;

    @Autowired
    private MetadataSettings metadataSettings;

    @Autowired
    private MessageStore messageStore;

    @Autowired
    private LogonProcessor logonProcessor;

    public void setMetadataSettings(MetadataSettings metadataSettings) {
        this.metadataSettings = metadataSettings;
    }

    public MetadataSettings getMetadataSettings() {
        return this.metadataSettings;
    }

    public void setMessageStore(MessageStore messageStore) {
        this.messageStore = messageStore;
    }

    public MessageStore getMessageStore() {
        return this.messageStore;
    }

    public void setLogonProcessor(LogonProcessor logonProcessor) {
        this.logonProcessor = logonProcessor;
    }

    public LogonProcessor getLogonProcessor() {
        return this.logonProcessor;
    }

    public SsoRequestSender() throws NoSuchAlgorithmException {
        this.generator = new SecureRandomIdentifierGenerator();
        this.logonProcessor = null;
        this.messageStore = null;
        this.metadataSettings = null;
    }

    /**
     * Entry point for sending SSO request to IDP. This function responds a HTTP
     * redirect with authentication parameters attached to the url used in the
     * redirect.
     *
     * @param requestSettings
     *            SSO request settings
     * @param response
     *            HTTP response servlet used for redirecting the user browser
     *            signing on request..
     * @throws IOException
     * @throws ConfigurationException
     * @throws MarshallingException
     * @throws NoSuchAlgorithmException
     * @throws InvalidKeyException
     * @throws WebssoClientException
     */
    public void sendRequest(SsoRequestSettings requestSettings, HttpServletResponse response) throws IOException,
            ConfigurationException, MarshallingException, InvalidKeyException, NoSuchAlgorithmException,
            WebssoClientException {

        Validate.notNull(requestSettings, "requestSettings");
        Validate.notNull(response, "response");
        Validate.notNull(getMessageStore(), "messageStore");
        Validate.notNull(getMetadataSettings(), "metadataSettings");

        String redirectUrl = getRequestUrl(requestSettings);

        if (redirectUrl != null) {
            SharedUtils.SetNoCacheHeader(response);
            response.sendRedirect(redirectUrl);
        } else {
            logger.error("Fail to generate the SSO request!");
        }
    }


    /**
     * SSO request processing. Return redirect url to be sent via HTTP
     * redirect.
     *
     * @param requestSettings
     *            SSO request parameters
     * @param reqID
     *            optional request ID. Will be generated if not provided.
     * return  String  request url.
     */
    public String getRequestUrl(SsoRequestSettings requestSettings, String reqID) {

        String redirectUrl = null;
        Validate.notNull(requestSettings, "requestSettings");

        try {
            logger.info("Producing redirect url");

            SPConfiguration spConfig = this.metadataSettings.getSPConfiguration(requestSettings.getSPAlias());
            if (spConfig == null) {
                throw new IllegalArgumentException("service provider setting unavailable for "
                        + requestSettings.getSPAlias());
            }

            IssuerBuilder issuerBuilder = new IssuerBuilder();
            Issuer issuer = issuerBuilder.buildObject(SamlNames.ASSERTION, "Issuer", "samlp");
            issuer.setValue(spConfig.getEntityID());

            // Create NameIDPolicy
            NameIDPolicy nameIdPolicy = createNameIdPolicy(requestSettings.getNameIDFormat(), spConfig, requestSettings);

            // Create scoping

            Scoping scoping = createScoping(requestSettings);

            // Build AuthRuest object
            DateTime issueInstant = new DateTime();
            AuthnRequestBuilder authRequestBuilder = new AuthnRequestBuilder();
            AuthnRequest authRequest = authRequestBuilder.buildObject(SamlNames.PROTOCOL, "AuthnRequest", "samlp");
            if (reqID != null) {
                authRequest.setID(reqID);
            } else {
                authRequest.setID(this.generator.generateIdentifier());
            }
            authRequest.setForceAuthn(requestSettings.isForceAuthn());
            authRequest.setIsPassive(requestSettings.isPassive());
            authRequest.setIssueInstant(issueInstant);
            authRequest.setProtocolBinding(SamlNames.HTTP_POST); // expected
                                                                 // response
                                                                 // binding
            authRequest.setScoping(scoping);
            // both setting are optional. Only one, at most, should be set.
            String acsUrl = requestSettings.getAssertionConsumerServiceUrl();
            if (acsUrl != null && !acsUrl.isEmpty()) {
                authRequest.setAssertionConsumerServiceURL(requestSettings.getAssertionConsumerServiceUrl());
            } else {
                Integer index = requestSettings.getAssertionConsumerServiceIndex();
                if (index != null) {
                    authRequest.setAssertionConsumerServiceIndex(index);
                }
            }

            authRequest.setIssuer(issuer);
            authRequest.setNameIDPolicy(nameIdPolicy);

            if (requestSettings.getAllowRequestAuthnContext()) {

                // Create AuthnContextClassRef
                RequestedAuthnContext requestedAuthnContext = createRequestedAuthnContext(requestSettings);
                authRequest.setRequestedAuthnContext(requestedAuthnContext);
            }
            authRequest.setVersion(SAMLVersion.VERSION_20);
            authRequest.setProviderName(requestSettings.getSPAlias());

            IDPConfiguration idpConfig = this.metadataSettings.getIDPConfiguration(requestSettings.getIDPAlias());
            Validate.notNull(idpConfig);

            String destination = SamlUtils.getIdpSsoLocation(idpConfig, SamlNames.HTTP_REDIRECT);
            Validate.notEmpty(destination, "destination");
            authRequest.setDestination(destination);

            // add renewable/delegable conditions as needed
            ConditionsBuilder conditionsBuilder = new ConditionsBuilder();
            Conditions conditions = null;
            if (requestSettings.isRenewable() != null && requestSettings.isRenewable().booleanValue()) {
                if (conditions == null) {
                    conditions = conditionsBuilder.buildObject();
                }
                conditions.getConditions().add(createRenewable());
            }
            if (requestSettings.isDelegable() != null && requestSettings.isDelegable().booleanValue()) {
                if (conditions == null) {
                    conditions = conditionsBuilder.buildObject();
                }
                conditions.getConditions().add(createDelegable());
            }

            // add conditions object to the request
            if (conditions != null) {
                authRequest.setConditions(conditions);
            }

            StringBuilder urlStringBuilder = new StringBuilder();

            // Creating redirect url. The destination is not signed.
            urlStringBuilder.append(authRequest.getDestination());
            logger.info("Destination URL: " + authRequest.getDestination());

            // create and append signed portion - the query string
            String requestStr = createRequestString(authRequest, requestSettings.getRelayState(),
                    requestSettings.isSigned(), spConfig.getSigningPrivateKey(), spConfig.getSigningAlgorithm());
            urlStringBuilder.append(requestStr);
            redirectUrl = urlStringBuilder.toString();

            if (redirectUrl == null || redirectUrl.isEmpty()) {
                logger.warn("Redirect URL is null or empty.");
            }

            // create a authentication message in message store.
            Message authnMessage = new Message(MessageType.AUTHN_REQUEST, authRequest.getID(),
                    requestSettings.getRelayState(), issueInstant, authRequest.getIssuer().getValue(), destination, // target
                    null, // status
                    null, // substatus
                    null, // session index
                    null, // MessageDatda
                    null, // tag
                    false); //isIdpInitiated
            getMessageStore().add(authnMessage);

        } catch (Exception e) {
            logonProcessor.internalError(e, null, null);
            SharedUtils.logMessage(e);
            redirectUrl = null;
        }

        return redirectUrl;
    }

    /**
     * SSO request processing. Return a request url.to be send via HTTP
     * redirect.
     *
     * @param requestSettings
     *            SSO requestion parameters
     * @return String request url
     *
     */
    public String getRequestUrl(SsoRequestSettings requestSettings) {

        return this.getRequestUrl(requestSettings, null);
    }

    // Creates Scoping object if proxyCount is set
    // @param requestSettings. SsoRequestSettings obj.
    // @return Scoping obj. could be null.
    private Scoping createScoping(SsoRequestSettings requestSettings) {
        Scoping scoping = null;
        int proxyCount = 0;

        if (requestSettings.getAllowScopingElement()) {

            // Emit this scoping if proxyCount is set.
            if (requestSettings.getProxyCount() != null && requestSettings.getProxyCount() >= 0) {
                proxyCount = requestSettings.getProxyCount();
                scoping = new ScopingBuilder().buildObject();
                scoping.setProxyCount(proxyCount);
            }
        }

        return scoping;
    }

    private RenewableType createRenewable() {

        RenewableType proxy = new RenewableTypeBuilder().buildObject();

        logger.info("Added Renewable condition");
        return proxy;
    }

    private DelegableType createDelegable() {

        DelegableType proxy = new DelegableTypeBuilder().buildObject();

        logger.info("Added Delegable condition");
        return proxy;
    }

    private NameIDPolicy createNameIdPolicy(String idFormat, SPConfiguration spConfig,
            SsoRequestSettings requestSettings) {

        Validate.notNull(spConfig, "spConfig");
        NameIDPolicyBuilder nameIdPolicyBuilder = new NameIDPolicyBuilder();
        NameIDPolicy nameIdPolicy = nameIdPolicyBuilder.buildObject();
        if (idFormat != null) {
            nameIdPolicy.setFormat(idFormat);
        }

        if (requestSettings.getAllowSPNameQualifierInNameIDPolicy()) {
            nameIdPolicy.setSPNameQualifier(spConfig.getEntityID());
        }
        if (requestSettings.getAllowAllowCreateInNameIDPolicy()) {
            nameIdPolicy.setAllowCreate(true);
        }
        return nameIdPolicy;
    }

    /**
     * Build RequestedAuthnContext
     * @return RequestedAuthnContext -  null possible
     */
    private RequestedAuthnContext createRequestedAuthnContext(SsoRequestSettings requestSettings) {

        // Create and setup RequestedAuthnContext
        RequestedAuthnContextBuilder requestedAuthnContextBuilder = new RequestedAuthnContextBuilder();
        RequestedAuthnContext requestedAuthnContext = requestedAuthnContextBuilder.buildObject();
        requestedAuthnContext.setComparison(AuthnContextComparisonTypeEnumeration.EXACT);

        // Populate AuthnContextClassRef list
        List<AuthnContextClassRef> authnContextClassRefs = requestedAuthnContext.getAuthnContextClassRefs();
        List<String> reqSettingsContextClasses = requestSettings.getRequestedAuthnContextClasses();

        if (null == reqSettingsContextClasses) {
            requestedAuthnContext = null;
        } else {
            for (String ref : reqSettingsContextClasses) {
                authnContextClassRefs.add(createAuthnContextClassReg(ref));
            }
        }
        return requestedAuthnContext;
    }

    private AuthnContextClassRef createAuthnContextClassReg(String authnClassURIStr) {

        AuthnContextClassRefBuilder authnContextClassRefBuilder = new AuthnContextClassRefBuilder();
        AuthnContextClassRef authnContextClassRef = authnContextClassRefBuilder.buildObject();

        authnContextClassRef.setAuthnContextClassRef(authnClassURIStr);

        return authnContextClassRef;
    }

    /**
     * Build part of redirect url that could be signed The portion is
     * "SAMLRequest=<base64 encoded request>&SigAlg=<signature algorithm>"
     *
     * @throws IOException
     * @throws MarshallingException
     * @throws WebssoClientException
     * @throws NoSuchAlDategorithmException
     */
    private String createRequestString(AuthnRequest authRequest, String relayState, Boolean isSigned,
            PrivateKey signingKey, String algorithmName) throws MarshallingException, IOException,
            NoSuchAlgorithmException, WebssoClientException {

        Validate.notNull(authRequest, "AuthnRequest object");
        if (isSigned && (signingKey == null || algorithmName == null)) {
            throw new WebssoClientException("Signing key missing, " + "or algorithm not specified.");
        }
        // 1 encode AuthnRequest to a string using saml service
        SamlUtils utils = new SamlUtils(null, signingKey, algorithmName, null, null // issuer
        );

        String samlRequestParameter = SamlUtils.encodeSAMLObject(authRequest, true); // base64
                                                                                     // encode
                                                                                     // it

        // 2. create to-be-signed string Request+algorithmName

        String notSignedRequest = SamlUtils.generateRedirectUrlQueryStringParameters(samlRequestParameter, null, // response
                                                                                                                 // parameter
                relayState, isSigned ? algorithmName : null, null); // signature
        logger.info("Relay State value is: " + relayState);
        if (notSignedRequest == null || notSignedRequest.isEmpty()) {
            logger.info("Found null or empty message to be signed. ");
        }

        // 3 Sign the string to produce signature
        String queryString = "?";

        if (isSigned) {

            String signature = utils.signMessage(notSignedRequest);
            logger.debug("Signature " + signature);

            String signedString = SamlUtils.generateRedirectUrlQueryStringParameters(samlRequestParameter, // to
                                                                                                           // check:
                                                                                                           // encoded
                                                                                                           // twice?
                    null, // response parameter
                    relayState, algorithmName, signature); // signature

            queryString += signedString;
        } else {
            queryString += notSignedRequest;
        }

        return queryString;
    }
}
