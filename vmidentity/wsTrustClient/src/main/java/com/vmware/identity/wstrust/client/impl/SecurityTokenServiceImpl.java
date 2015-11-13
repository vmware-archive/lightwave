/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import java.net.URL;
import java.util.UUID;
import java.util.concurrent.Callable;
import java.util.concurrent.Future;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.vmware.vim.sso.client.BundleMessageSource.Key;
import com.vmware.vim.sso.client.DefaultTokenFactory;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.vim.sso.client.SamlTokenFactory;
import com.vmware.vim.sso.client.exception.SsoException;
import com.vmware.identity.wstrust.client.AsyncHandler;
import com.vmware.identity.wstrust.client.CertificateValidationException;
import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.GSSCredential;
import com.vmware.identity.wstrust.client.GSSServerCredential;
import com.vmware.identity.wstrust.client.MalformedResponseException;
import com.vmware.identity.wstrust.client.NegotiationHandler;
import com.vmware.identity.wstrust.client.RequestExpiredException;
import com.vmware.identity.wstrust.client.SecurityTokenService;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.ServerCommunicationException;
import com.vmware.identity.wstrust.client.ServerSecurityException;
import com.vmware.identity.wstrust.client.SsoRequestException;
import com.vmware.identity.wstrust.client.TimeSynchronizationException;
import com.vmware.identity.wstrust.client.TokenCredential;
import com.vmware.identity.wstrust.client.TokenRequestRejectedException;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.ValidateUtil;

/**
 * Provides WS-Trust over SOAP implementation for acquiring/validating SAML
 * tokens.
 */
class SecurityTokenServiceImpl implements SecurityTokenService {
    private static final String NULL_LEG_ERR_MSG = "Unexpected \"null\" leg before negotiation completion";
    private static final String INCOMPLETE_SSPI_ERR_MSG = "No token has been acquired. The negotiation should continue";

    private final SoapBinding binding;
    private final SecurityTokenServiceConfig stsConfig;
    private final SamlTokenFactory tokenFactory = new DefaultTokenFactory();
    private RequestParserFactoryProvider parserProvider;

    private final Logger log = LoggerFactory.getLogger(SecurityTokenServiceImpl.class);

    /**
     * Internal. Use SecurityTokenServiceFactory.getSecurityTokenServiceImpl()
     *
     * @param transport
     *            SoapTransport implementation. Cannot be null.
     * @param config
     *            Configuration object. Cannot be null.
     * @param messageProcessorFactory
     *            Factory for creating MessageProcessors. Cannot be null.
     */
    public SecurityTokenServiceImpl(SoapBinding binding, SecurityTokenServiceConfig config) {

        ValidateUtil.validateNotNull(binding, "Soap binding");
        ValidateUtil.validateNotNull(config, "STS configuration");

        this.binding = binding;
        this.stsConfig = config;
        this.parserProvider = new RequestParserFactoryProvider(stsConfig);
    }

    public void setParserProvider(RequestParserFactoryProvider parserProvider) {
        this.parserProvider = parserProvider;
    }

    @Override
    public SamlToken acquireToken(final Credential clientCredential, final TokenSpec tokenSpec)
            throws InvalidTokenException, TokenRequestRejectedException, SsoRequestException,
            ServerCommunicationException, ServerSecurityException, RequestExpiredException,
            CertificateValidationException, TimeSynchronizationException {

        ValidateUtil.validateNotNull(clientCredential, "Client credential");

        try (OperationFrame opFrame = new OperationFrame()) {
            log.debug("Acquiring token for: " + clientCredential + " started.");

            SamlToken token;

            if (isMultiStepNegotiationRequest(clientCredential)) {
                token = executeMultiStepNegotiation(clientCredential, tokenSpec);
            } else {
                token = executeSingleStepNegotiation(clientCredential, tokenSpec);
            }
            log.info("Successfully acquired token for: " + clientCredential);

            return token;
        }
    }

    @Override
    public Future<SamlToken> acquireTokenAsync(final Credential clientCredential, final TokenSpec tokenSpec,
            final AsyncHandler<SamlToken> asyncHandler) {
        ValidateUtil.validateNotNull(asyncHandler, "Async handler");

        Callable<SamlToken> task = new AsyncCommand<SamlToken>(asyncHandler) {
            @Override
            protected SamlToken executeAction() throws SsoException {
                return acquireToken(clientCredential, tokenSpec);
            }
        };

        return stsConfig.getExecutorService().submit(task);
    }

    @Override
    public Future<SamlToken> acquireTokenAsync(final Credential clientCredential, final TokenSpec tokenSpec) {

        Callable<SamlToken> task = new AsyncCommand<SamlToken>(new DefaultAsynHandler<SamlToken>()) {
            @Override
            protected SamlToken executeAction() throws SsoException {
                return acquireToken(clientCredential, tokenSpec);
            }
        };

        return stsConfig.getExecutorService().submit(task);
    }

    @Override
    public boolean validateToken(SamlToken token) throws InvalidTokenException, TokenRequestRejectedException,
            SsoRequestException, ServerCommunicationException, ServerSecurityException, RequestExpiredException,
            CertificateValidationException, TimeSynchronizationException {

        Credential clientCredential = new TokenCredential(token);

        RequestParserAbstractFactory<Boolean> credentialParser = this.parserProvider
                .getValidateCredentialParser(clientCredential.getClass());

        credentialParser.createRequestParametersValidator().validate(clientCredential, null);

        ResponseHandler<Boolean> responseHandler = credentialParser.createResponseHandler();

        OperationFrame opFrame = new OperationFrame();
        try {
            log.debug("Validating token for subject" + Util.getTokenSubjectForLog(token) + "started.");

            Node responseNode = executeSingleRequest(clientCredential, null, credentialParser);

            boolean isTokenConfirmedValid = false;
            try {
                isTokenConfirmedValid = responseHandler.parseResponse(responseNode);
            } catch (ParserException e) {
                String message = "Error occured during deserialization of response" + " to the token request";
                log.info(message, e);
                throw new MalformedResponseException(message, Key.UNEXPECTED_RESPONSE_FORMAT, e);
            }
            log.info("Is token valid confimation result: " + isTokenConfirmedValid);

            return isTokenConfirmedValid;
        } finally {
            opFrame.close();
        }
    }

    @Override
    public Future<Boolean> validateTokenAsync(final SamlToken token, AsyncHandler<Boolean> asyncHandler) {

        ValidateUtil.validateNotNull(asyncHandler, "Async handler");

        Callable<Boolean> task = new AsyncCommand<Boolean>(asyncHandler) {
            @Override
            protected Boolean executeAction() throws SsoException {
                return validateToken(token);
            }
        };

        return stsConfig.getExecutorService().submit(task);
    }

    @Override
    public Future<Boolean> validateTokenAsync(final SamlToken token) {

        return validateTokenAsync(token, new DefaultAsynHandler<Boolean>());
    }

    @Override
    public SamlToken renewToken(SamlToken token, long tokenLifetimeSec) throws InvalidTokenException,
            TokenRequestRejectedException, SsoRequestException, ServerCommunicationException, ServerSecurityException,
            RequestExpiredException, CertificateValidationException, TimeSynchronizationException {

        Credential clientCredential = new TokenCredential(token);
        TokenSpec spec = new TokenSpec.Builder(tokenLifetimeSec).createTokenSpec();

        RequestParserAbstractFactory<Element> credentialParser = this.parserProvider
                .getRenewCredentialParser(clientCredential.getClass());

        credentialParser.createRequestParametersValidator().validate(clientCredential, spec);

        ResponseHandler<Element> responseHandler = credentialParser.createResponseHandler();

        try (OperationFrame opFrame = new OperationFrame()) {
            log.debug("Validating token for subject" + Util.getTokenSubjectForLog(token) + "started.");

            Node tokenNode = executeSingleRequest(clientCredential, spec, credentialParser);

            Element tokenElement = null;
            try {
                tokenElement = responseHandler.parseResponse(tokenNode);
            } catch (ParserException e) {
                String message = "Error occured during deserialization of response" + " to the token request";
                log.info(message, e);
                throw new MalformedResponseException(message, Key.UNEXPECTED_RESPONSE_FORMAT, e);
            }
            SamlToken renewedToken = tokenFactory.parseToken(tokenElement, stsConfig.getTrustedRootCertificates());

            log.info("Successfully renewed token for user: " + Util.getTokenSubjectForLog(renewedToken));

            return renewedToken;
        }
    }

    @Override
    public Future<SamlToken> renewTokenAsync(final SamlToken token, final long tokenLifetimeSec,
            final AsyncHandler<SamlToken> asyncHandler) {

        ValidateUtil.validateNotNull(asyncHandler, "Async handler");

        Callable<SamlToken> task = new AsyncCommand<SamlToken>(asyncHandler) {
            @Override
            protected SamlToken executeAction() throws SsoException {
                return renewToken(token, tokenLifetimeSec);
            }
        };

        return stsConfig.getExecutorService().submit(task);
    }

    @Override
    public Future<SamlToken> renewTokenAsync(SamlToken token, long tokenLifetimeSec) {
        return renewTokenAsync(token, tokenLifetimeSec, new DefaultAsynHandler<SamlToken>());
    }

    /**
     * Returns if the authentication needs multi-step negotiation
     *
     * @param clientCredential
     * @return
     */
    private boolean isMultiStepNegotiationRequest(Credential clientCredential) {
        return clientCredential instanceof GSSCredential;
    }

    private SamlToken executeSingleStepNegotiation(final Credential clientCredential, final TokenSpec tokenSpec)
            throws InvalidTokenException, TokenRequestRejectedException, CertificateValidationException {

        RequestParserAbstractFactory<Element> credentialParser = this.parserProvider
                .getSingleStepAcquireCredentialParser(clientCredential.getClass());

        credentialParser.createRequestParametersValidator().validate(clientCredential, tokenSpec);

        ResponseHandler<Element> responseHandler = credentialParser.createResponseHandler();

        Node tokenNode = executeSingleRequest(clientCredential, tokenSpec, credentialParser);

        Element tokenElement = null;
        try {
            tokenElement = responseHandler.parseResponse(tokenNode);
        } catch (ParserException e) {
            String message = "Error occured during deserialization of response" + " to the token request";
            log.info(message, e);
            throw new MalformedResponseException(message, Key.UNEXPECTED_RESPONSE_FORMAT, e);
        }

        SamlToken token = tokenFactory.parseToken(tokenElement, stsConfig.getTrustedRootCertificates());
        return token;
    }

    private Node executeSingleRequest(Credential clientCredential, TokenSpec tokenSpec,
            RequestParserAbstractFactory<?> credentialParser) throws SsoRequestException, ServerCommunicationException,
            ServerSecurityException, RequestExpiredException, TimeSynchronizationException, InvalidTokenException,
            TokenRequestRejectedException, CertificateValidationException {

        RequestBuilder requestBuilder = credentialParser.createRequestBuilder(clientCredential, tokenSpec);

        WsSecuritySignature signature = credentialParser.createWsSecuritySignature(clientCredential, tokenSpec);

        ServiceDiscovery ssoServiceDiscovery = new SiteAffinityServiceDiscovery(stsConfig.getConnectionConfig());
        URL serviceLocationURL = ssoServiceDiscovery.getServiceLocation();
        if (serviceLocationURL == null)
            throw new ServerCommunicationException("Cannot get the site-affinity URL");

        Node responseNode = new RequestResponseProcessor(requestBuilder, signature, binding)
                .executeRoundtrip(serviceLocationURL);

        return responseNode;
    }

    private SamlToken executeMultiStepNegotiation(Credential clientCredential, TokenSpec tokenSpec)
            throws ServerCommunicationException, TimeSynchronizationException, InvalidTokenException,
            TokenRequestRejectedException, SsoRequestException, CertificateValidationException {

        RequestParserAbstractFactory<GssResult> credentialParser = this.parserProvider
                .getMultiStepAcquireCredentialParser(clientCredential.getClass());

        credentialParser.createRequestParametersValidator().validate(clientCredential, tokenSpec);

        NegotiationHandler handler = ((GSSCredential) clientCredential).getNegotiationHandler();

        GssResult gssResult = null;
        String contextId = null;
        SamlToken token = null;

        ServiceDiscovery ssoServiceDiscovery = new SiteAffinityServiceDiscovery(stsConfig.getConnectionConfig());
        URL serviceLocationURL = ssoServiceDiscovery.getServiceLocation();

        do {

            final byte[] leg = handler.negotiate(gssResult == null ? null : gssResult.getLeg());

            if (leg == null) {
                log.error("Incorrect SecurityTokenService.NegotiationHandler " + "implementation "
                        + handler.getClass().getSimpleName() + ": " + NULL_LEG_ERR_MSG);
                throw new IllegalArgumentException(NULL_LEG_ERR_MSG);
            }

            boolean isInitial = ValidateUtil.isEmpty(contextId);

            log.debug("Initial GSS negotiation leg: " + isInitial);
            Credential cred = new GSSServerCredential(leg, contextId);

            ResponseHandler<GssResult> responseHandler = credentialParser.createResponseHandler();

            final RequestBuilder requestBuilder = credentialParser.createRequestBuilder(cred, tokenSpec);

            WsSecuritySignature signature = credentialParser.createWsSecuritySignature(cred, tokenSpec);

            try {
                Node gssResultNode = new RequestResponseProcessor(requestBuilder, signature, binding)
                        .executeRoundtrip(serviceLocationURL);

                gssResult = responseHandler.parseResponse(gssResultNode);

            } catch (ParserException e) {
                String message = "Error occured during deserialization of response" + " to the token request";
                log.info(message, e);
                throw new MalformedResponseException(message, Key.UNEXPECTED_RESPONSE_FORMAT, e);
            }
            contextId = gssResult.getContextId();

            if (gssResult.getToken() == null) {
                log.warn(INCOMPLETE_SSPI_ERR_MSG);
            } else {
                token = DefaultTokenFactory.createTokenFromDom(gssResult.getToken(),
                        stsConfig.getTrustedRootCertificates());

                log.info("Successfully acquired token for user: " + Util.getTokenSubjectForLog(token));
            }

        } while (gssResult.getToken() == null);

        final byte[] leg = gssResult.getLeg();
        if (leg != null && handler.negotiate(leg) != null) {
            log.warn("Possible incorrect behavior at client implementation " + handler.getClass().getSimpleName()
                    + ": another leg returned after SAML token has been received");
        }

        log.info("Successfully acquired token for user: " + token.getSubject());

        return token;
    }

    /**
     * Defines an operation frame for improved log structure
     */
    private class OperationFrame implements AutoCloseable {

        private String opId;

        /**
         * Marks the start of the operation frame
         */
        public OperationFrame() {
            if (log.isDebugEnabled()) {
                this.opId = UUID.randomUUID().toString();
                log.debug(String.format("opId=%s START operation", this.opId));
            }
        }

        /**
         * Marks the end of the operation frame
         */
        @Override
        public void close() {
            if (log.isDebugEnabled()) {
                // null safe w.r.t _opId
                log.debug(String.format("opId=%s END operation", this.opId));
            }
        }
    }
}
