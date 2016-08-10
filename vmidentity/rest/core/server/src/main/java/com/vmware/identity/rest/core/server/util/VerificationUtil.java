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
package com.vmware.identity.rest.core.server.util;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.InvalidKeyException;
import java.security.PublicKey;
import java.security.SignatureException;
import java.security.cert.Certificate;
import java.util.Calendar;
import java.util.Date;

import javax.ws.rs.container.ContainerRequestContext;

import org.apache.commons.codec.DecoderException;
import org.glassfish.jersey.message.internal.ReaderWriter;

import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.rest.core.server.authorization.exception.InvalidTokenException;
import com.vmware.identity.rest.core.server.authorization.token.AccessToken;
import com.vmware.identity.rest.core.server.authorization.token.TokenType;
import com.vmware.identity.rest.core.util.RequestSigner;
import com.vmware.identity.rest.core.util.StringManager;

/**
 * Utility class for performing request verification.
 */
public class VerificationUtil {

    /**
     * Verifies the token type in an access token.
     *
     * @param token the token to verify
     * @param type the expected type of the token
     * @param sm a string manager to get the exception message from
     * @param log a logger to log the error to
     *
     * @throws InvalidTokenException if the token is of an invalid or incorrect type
     */
    public static void verifyTokenType(AccessToken token, TokenType type, StringManager sm, IDiagnosticsLogger log) {
        if (!type.getJsonName().equals(token.getTokenType())) {
            log.error("JWT was expected to have the token type '{}', but was '{}'", type.getJsonName(), token.getTokenType());
            throw new InvalidTokenException(sm.getString("auth.ite.bad.type"));
        }
    }

    /**
     * Perform signature verification using the signed request, the HTTP context, and a public key.
     *
     * @param signedRequestHex the request signature, in hex encoding, that was sent by the client
     * @param context the HTTP context that will be used for validation
     * @param pubKey the public key to verify the signature with
     *
     * @return true if the signature was verified, false if not
     *
     * @throws InvalidKeyException if the key is invalid
     * @throws SignatureException if the signature object is not initialized properly
     * @throws DecoderException if the signedRequestHex is not in a valid hex format
     */
    public static boolean verifySignature(String signedRequestHex, ContainerRequestContext context, PublicKey pubKey) throws InvalidKeyException, SignatureException, DecoderException {
        if (signedRequestHex == null) {
            return false;
        }

        String stringToSign = buildStringToSign(context);

        return RequestSigner.verify(signedRequestHex, stringToSign, pubKey);
    }

    /**
     * Perform signature verification using the signed request, the HTTP context, and a certificate.
     *
     * @param signedRequestHex the request signature, in hex encoding, that was sent by the client
     * @param context the HTTP context that will be used for validation
     * @param certificate the certificate to verify the signature with
     *
     * @return true if the signature was verified, false if not
     *
     * @throws InvalidKeyException if the key is invalid
     * @throws SignatureException if the signature object is not initialized properly
     * @throws DecoderException if the signedRequestHex is not in a valid hex format
     */
    public static boolean verifySignature(String signedRequestHex, ContainerRequestContext context, Certificate certificate) throws InvalidKeyException, SignatureException, DecoderException {
        if (signedRequestHex == null) {
            return false;
        }

        String stringToSign = buildStringToSign(context);

        return RequestSigner.verify(signedRequestHex, stringToSign, certificate);
    }

    /**
     * Verify the issued-at and expires-at dates in an access token
     *
     * @param token the token to verify
     * @param skew the amount of skew to allow in milliseconds
     * @param sm a string manager to get the exception messages from
     *
     * @throws InvalidTokenException if the token is at an invalid date
     */
    public static void verifyTimestamps(AccessToken token, long skew, StringManager sm) throws InvalidTokenException {
        Calendar now = Calendar.getInstance();
        Calendar issuedAt = Calendar.getInstance();
        Calendar expiresAt = Calendar.getInstance();

        if (token.getIssueTime() != null) {
            issuedAt.setTimeInMillis(token.getIssueTime().getTime() - skew);
        }

        if (token.getExpirationTime() != null) {
            expiresAt.setTimeInMillis(token.getExpirationTime().getTime() + skew);
        }

        if (token.getIssueTime() == null || issuedAt.after(now)) {
            throw new InvalidTokenException(sm.getString("auth.ite.bad.issue", issuedAt.getTime(), now.getTime()));
        }

        if (token.getExpirationTime() == null || expiresAt.before(now)) {
            throw new InvalidTokenException(sm.getString("auth.ite.bad.expiry", expiresAt.getTime(), now.getTime()));
        }
    }

    public static void verifyAudience(AccessToken token, String audience, StringManager sm, IDiagnosticsLogger log) throws InvalidTokenException {
        if (!token.getAudience().contains(audience)) {
            log.error("Invalid audience '{}'. Expected to contain '{}'", token.getAudience(), audience);
            throw new InvalidTokenException(sm.getString("auth.ite.bad.audience"));
        }
    }

    /**
     * Verifies the request was issued within a certain amount of skew.
     *
     * @param context the request context to verify
     * @param skew the amount of skew to allow in milliseconds
     * @param log a logger to log the error to
     *
     * @return true if the request time is valid, false otherwise
     */
    public static boolean verifyRequestTime(ContainerRequestContext context, long skew, IDiagnosticsLogger log) {
        long currentTime = System.currentTimeMillis();
        Date beginsAt = new Date(currentTime - skew);
        Date expiresAt = new Date(currentTime + skew);

        Date requestTime = context.getDate();

        if (requestTime.before(beginsAt) || requestTime.after(expiresAt)) {
            log.error(String.format("Request time outside of acceptable range. Request time: '%s'. Acceptable range: ['%s', '%s']", requestTime, beginsAt, expiresAt));
            return false;
        }

        return true;
    }

    private static String getMD5(ContainerRequestContext context) {
        if (!context.hasEntity()) {
            return "";
        }

        InputStream in = context.getEntityStream();

        try {
            ByteArrayOutputStream out = new ByteArrayOutputStream(in.available());

            final byte[] data = new byte[ReaderWriter.BUFFER_SIZE];
            int len = 0;

            while ((len = in.read(data)) > 0) {
                out.write(data, 0, len);
            }

            // Reset our entity stream since we consumed it and it may need to be read for object marshalling
            context.setEntityStream(new ByteArrayInputStream(out.toByteArray()));

            return RequestSigner.computeMD5(out.toString());
        } catch (IOException e) {
            throw new IllegalArgumentException(e);
        }
    }

    public static String buildStringToSign(ContainerRequestContext context) {
        return RequestSigner.createSigningString(context.getMethod(),
                getMD5(context),
                context.getMediaType().toString(),
                context.getDate(),
                context.getUriInfo().getRequestUri());
    }

}
