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
package com.vmware.identity.rest.core.server.authorization.verifier.jwt.bearer;

import java.security.PublicKey;
import java.security.interfaces.RSAPublicKey;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSVerifier;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.server.authorization.token.jwt.bearer.JWTBearerToken;
import com.vmware.identity.rest.core.server.authorization.verifier.AccessTokenVerifier;
import com.vmware.identity.rest.core.server.exception.ServerException;
import com.vmware.identity.rest.core.server.util.VerificationUtil;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * An implementation of a bearer token verifier using the JSON Web Token format.
 *
 * @see <a href="http://jwt.io/">JSON Web Token Format</a>
 */
public class JWTBearerTokenVerifier implements AccessTokenVerifier {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(JWTBearerTokenVerifier.class);

    protected long skew;
    protected PublicKey publicKey;
    protected StringManager sm;

    /**
     * Constructs a new {@link JWTBearerTokenVerifier}.
     *
     * @param publicKey the public key to use for verification
     */
    public JWTBearerTokenVerifier(long skew, PublicKey publicKey) {
        this.skew = skew;
        this.publicKey = publicKey;
        this.sm = StringManager.getManager(Config.LOCALIZATION_PACKAGE_NAME);
    }

    public long getSkew() {
        return skew;
    }

    public PublicKey getPublicKey() {
        return publicKey;
    }

    @Override
    public void verify(AccessToken token) throws InvalidTokenException, ServerException {
        verify(token, TokenType.BEARER);
    }

    protected void verify(AccessToken token, TokenType type) {

        if (!(token instanceof JWTBearerToken)) {
            throw new IllegalArgumentException("Access token expected to be JWTBearerToken. Was " + token.getClass());
        }

        try {
            RSAPublicKey key = (RSAPublicKey) publicKey;
            JWSVerifier verifier = new RSASSAVerifier(key);

            if (!((JWTBearerToken) token).getJWT().verify(verifier)) {
                log.error("JWT could not be verified");
                throw new InvalidTokenException(sm.getString("auth.ite.bad.verification"));
            }

        } catch (JOSEException e) {
            log.error("JWT could not be verified", e);
            throw new InvalidTokenException(sm.getString("auth.ite.bad.verification"));
        }

        VerificationUtil.verifyTokenType(token, type, sm, log);
        VerificationUtil.verifyTimestamps(token, skew, sm);
        VerificationUtil.verifyAudience(token, Config.RESOURCE_SERVER_AUDIENCE, sm, log);
    }

}
