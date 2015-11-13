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

package com.vmware.identity.openidconnect.server;

import java.io.IOException;
import java.io.PrintWriter;
import java.security.NoSuchAlgorithmException;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.InvalidKeySpecException;
import java.util.List;

import javax.servlet.http.HttpServletResponse;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.JWSHeader;
import com.nimbusds.jose.JWSSigner;
import com.nimbusds.jose.crypto.RSASSASigner;
import com.nimbusds.jose.jwk.JWK;
import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.KeyType;
import com.nimbusds.jose.jwk.KeyUse;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.nimbusds.oauth2.sdk.ErrorObject;
import com.nimbusds.oauth2.sdk.GrantType;
import com.nimbusds.oauth2.sdk.OAuth2Error;
import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.openid.connect.sdk.OIDCScopeValue;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.SolutionUserCredentialsGrant;

/**
 * @author Yehia Zayour
 * @author Jun Sun
 */
public class Shared {
    private static final String SESSION_COOKIE_NAME = "oidc_session_id";

    private Shared() {
    }

    public static String getSessionCookieName(String tenant) {
        Validate.notEmpty(tenant, "tenant");
        return String.format("%s-%s", SESSION_COOKIE_NAME, tenant);
    }

    public static ErrorObject validateScope(Scope scope, GrantType grantType) {
        Validate.notNull(grantType, "grantType");

        ErrorObject error = null;

        if (scope == null || !scope.contains(OIDCScopeValue.OPENID)) {
            error = OAuth2Error.INVALID_REQUEST.setDescription("missing scope=openid parameter");
        }

        if (error == null) {
            for (String scopeValue : scope.toStringList()) {
                boolean valid =
                        scopeValue.equals(OIDCScopeValue.OPENID.toString()) ||
                        scopeValue.equals(OIDCScopeValue.OFFLINE_ACCESS.toString()) ||
                        scopeValue.startsWith("rs_") ||
                        ScopeValue.isDefined(scopeValue);
                if (!valid) {
                    error = OAuth2Error.INVALID_SCOPE.setDescription("unrecognized scope value: " + scopeValue);
                    break;
                }
            }
        }

        if (error == null) {
            boolean refreshTokenDisallowed =
                    grantType.equals(GrantType.IMPLICIT) ||
                    grantType.equals(GrantType.REFRESH_TOKEN) ||
                    grantType.equals(GrantType.CLIENT_CREDENTIALS) ||
                    grantType.equals(SolutionUserCredentialsGrant.GRANT_TYPE);
            if (refreshTokenDisallowed && scope.contains(OIDCScopeValue.OFFLINE_ACCESS)) {
                error = OAuth2Error.INVALID_SCOPE.setDescription("refresh token (offline_access) is not allowed for this grant_type");
            }
        }

        return error;
    }

    public static SignedJWT sign(
            ReadOnlyJWTClaimsSet claimsSet,
            RSAPrivateKey privateKey) throws ServerException {
        Validate.notNull(claimsSet, "claimsSet");
        Validate.notNull(privateKey, "privateKey");

        SignedJWT signedJWT = new SignedJWT(new JWSHeader(JWSAlgorithm.RS256), claimsSet);
        JWSSigner signer = new RSASSASigner(privateKey);
        try {
            signedJWT.sign(signer);
        } catch (JOSEException e) {
            throw new ServerException(OAuth2Error.SERVER_ERROR.setDescription("failed to sign jwt"), e);
        }
        return signedJWT;
    }

    public static RSAPublicKey extractRsa256PublicKey(JWKSet jwkSet) {
        Validate.notNull(jwkSet, "jwkSet");

        JWK jwk = null;
        List<JWK> keys = jwkSet.getKeys();
        if (keys != null && keys.size() == 1) {
            jwk = keys.get(0);
        }

        RSAKey rsaKey = null;
        if (
                jwk != null &&
                (KeyType.RSA).equals(jwk.getKeyType()) &&
                (JWSAlgorithm.RS256).equals(jwk.getAlgorithm()) &&
                (KeyUse.SIGNATURE).equals(jwk.getKeyUse())) {
            rsaKey = (RSAKey) jwk;
        }

        RSAPublicKey rsaPublicKey = null;
        if (rsaKey != null) {
            try {
                rsaPublicKey = rsaKey.toRSAPublicKey();
            } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            }
        }
        return rsaPublicKey;
    }

    public static String replaceLast(String string, String from, String to) {
        Validate.notNull(string, "string");

        int lastIndex = string.lastIndexOf(from);
        if (lastIndex < 0) {
            return string;
        }
        String tail = string.substring(lastIndex).replaceFirst(from, to);
        return string.substring(0, lastIndex) + tail;
    }

    public static void writeJSONResponse(HttpServletResponse httpServletResponse, int statusCode, JSONObject jsonObject) throws IOException {
        Validate.notNull(httpServletResponse, "httpServletResponse");
        Validate.notNull(jsonObject, "jsonObject");

        PrintWriter writer = null;
        try {
            httpServletResponse.setStatus(statusCode);
            httpServletResponse.setContentType("application/json");
            writer = httpServletResponse.getWriter();
            writer.print(jsonObject.toString());
        } finally {
            if (writer != null) {
                writer.close();
            }
        }
    }

    public static void logFailedRequest(IDiagnosticsLogger logger, ErrorObject error) {
        Validate.notNull(logger, "logger");
        Validate.notNull(error, "error");
        logFailedRequest(logger, new ServerException(error));
    }

    public static void logFailedRequest(IDiagnosticsLogger logger, ServerException e) {
        Validate.notNull(logger, "logger");
        Validate.notNull(e, "e");
        ErrorObject error = e.getErrorObject();
        logger.info("request failed: error_code [{}] error_description [{}] exception [{}]", error.getCode(), error.getDescription(), e.getCause());
    }
}
