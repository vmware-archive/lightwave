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
package com.vmware.identity.rest.core.server.authorization.verifier.saml;

import java.security.InvalidKeyException;
import java.security.SignatureException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

import javax.ws.rs.container.ContainerRequestContext;

import org.apache.commons.codec.DecoderException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.saml.SAMLToken;
import com.vmware.identity.rest.core.server.authorization.verifier.AccessTokenVerifier;
import com.vmware.identity.rest.core.server.exception.ServerException;
import com.vmware.identity.rest.core.server.util.VerificationUtil;
import com.vmware.identity.rest.core.util.StringManager;
import com.vmware.vim.sso.client.ConfirmationType;

public class SAMLTokenVerifier implements AccessTokenVerifier {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(SAMLTokenVerifier.class);

    private String signedRequest;
    private ContainerRequestContext context;
    protected Certificate certificate;
    protected long skew;
    protected StringManager sm;

    public SAMLTokenVerifier(String signedRequest, ContainerRequestContext context, long skew, Certificate certificate) {
        this.signedRequest = signedRequest;
        this.context = context;
        this.skew = skew;
        this.certificate = certificate;
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
    }

    public Certificate getCertificate() {
        return certificate;
    }

    public long getSkew() {
        return skew;
    }

    public String getSignedRequest() {
        return signedRequest;
    }

    public ContainerRequestContext getContext() {
        return context;
    }

    @Override
    public void verify(AccessToken token) throws InvalidTokenException, InvalidRequestException, ServerException {
        if (!(token instanceof SAMLToken)) {
            throw new IllegalArgumentException("Access token expected to be a SAMLToken. Was " + token.getClass());
        }

        SAMLToken samlToken = (SAMLToken) token;

        try {
            X509Certificate[] certs = new X509Certificate[] { (X509Certificate) certificate };
            // Convert skew time to seconds
            samlToken.getSAMLToken().validate(certs, skew / 1000);
        } catch (com.vmware.vim.sso.client.exception.InvalidTokenException e) {
            log.error("SAML could not be validated", e);
            throw new InvalidTokenException(sm.getString("auth.ite.bad.verification"));
        }

        VerificationUtil.verifyTimestamps(token, skew, sm);

        if (samlToken.getSAMLToken().getConfirmationType() == ConfirmationType.HOLDER_OF_KEY) {
            if (this.signedRequest == null) {
                log.debug("Token was HOK and did not come with a signed request");
                throw new InvalidRequestException(sm.getString("auth.ire.no.signature"));
            }

            // TODO Not sure if this is necessary...
            if (!VerificationUtil.verifyRequestTime(context, skew, log)) {
                throw new InvalidTokenException(sm.getString("auth.ite.bad.verification"));
            }

            try {
                if (!VerificationUtil.verifySignature(this.signedRequest, context, samlToken.getSAMLToken().getConfirmationCertificate())) {
                    log.debug("Unable to verify the signed request using the alleged public key");
                    throw new InvalidTokenException(sm.getString("auth.ite.bad.verification"));
                }
            } catch (InvalidKeyException | DecoderException e) {
                log.error("Unable to verify the signed request", e);
                throw new InvalidTokenException(sm.getString("auth.ite.bad.verification"));
            } catch (SignatureException e) {
                log.error("Error while verifying the signed request", e);
                throw new ServerException("An error occurred with the server");
            }
        }

        // VerificationUtil.verifyAudience(samlToken, Config.RESOURCE_SERVER_AUDIENCE, sm, log);
    }

}
