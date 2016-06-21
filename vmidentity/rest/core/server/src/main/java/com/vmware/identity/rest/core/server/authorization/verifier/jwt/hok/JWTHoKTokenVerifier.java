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
package com.vmware.identity.rest.core.server.authorization.verifier.jwt.hok;

import java.security.InvalidKeyException;
import java.security.PublicKey;
import java.security.SignatureException;

import javax.ws.rs.container.ContainerRequestContext;

import org.apache.commons.codec.DecoderException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidRequestException;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.jwt.hok.JWTHoKToken;
import com.vmware.identity.rest.core.server.authorization.verifier.jwt.bearer.JWTBearerTokenVerifier;
import com.vmware.identity.rest.core.server.exception.ServerException;
import com.vmware.identity.rest.core.server.util.VerificationUtil;

public class JWTHoKTokenVerifier extends JWTBearerTokenVerifier {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(JWTHoKTokenVerifier.class);

    private String signedRequest;
    private ContainerRequestContext context;

    public JWTHoKTokenVerifier(String signedRequest, ContainerRequestContext context, long skew, PublicKey publicKey) {
        super(skew, publicKey);
        this.signedRequest = signedRequest;
        this.context = context;
    }

    public String getSignedRequest() {
        return signedRequest;
    }

    public ContainerRequestContext getContext() {
        return context;
    }

    @Override
    public void verify(AccessToken token) throws InvalidTokenException, InvalidRequestException, ServerException {
        super.verify(token, TokenType.HOK);

        if (!(token instanceof JWTHoKToken)) {
            throw new IllegalArgumentException("Access token expected to be JWTHoKToken. Was " + token.getClass());
        }

        if (this.signedRequest == null) {
            log.debug("Token was HOK and did not come with a signed request");
            throw new InvalidRequestException(sm.getString("auth.ire.no.signature"));
        }


        JWTHoKToken hokToken = (JWTHoKToken) token;

        // TODO Not sure if this is necessary...
        if (!VerificationUtil.verifyRequestTime(context, skew, log)) {
            throw new InvalidTokenException(sm.getString("auth.ite.bad.verification"));
        }

        try {
            if (!VerificationUtil.verifySignature(this.signedRequest, context, hokToken.getPublicKey())) {
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

        VerificationUtil.verifyAudience(hokToken, Config.RESOURCE_SERVER_AUDIENCE, sm, log);
    }

}
