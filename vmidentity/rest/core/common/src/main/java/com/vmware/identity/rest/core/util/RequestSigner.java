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
package com.vmware.identity.rest.core.util;

import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Signature;
import java.security.SignatureException;
import java.security.cert.Certificate;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;

/**
 * The {@code RequestSigner} class contains utilities for signing requests.
 */
public class RequestSigner {

    private static final String SHA256_WITH_RSA = "SHA256withRSA";

    /**
     * Signs a string using the private key and SHA 256 with RSA signing algorithm, and
     * returns it as a hex-encoded string.
     *
     * @param signingString the string to sign.
     * @param privateKey the private key to sign the string with.
     * @return the signed string in a hex-encoded format.
     * @throws InvalidKeyException if the key is invalid.
     * @throws SignatureException if the signature algorithm is unable to process the input
     * data provided.
     */
    public static String sign(String signingString, PrivateKey privateKey) throws InvalidKeyException, SignatureException {
        byte[] bytes = signingString.getBytes(StandardCharsets.UTF_8);

        Signature sig;

        try {
            sig = Signature.getInstance(SHA256_WITH_RSA);
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalArgumentException("An error occurred while getting the signature algorithm", e);
        }

        sig.initSign(privateKey);
        sig.update(bytes);

        return Hex.encodeHexString(sig.sign());
    }

    /**
     * Verify a signed request using a hex-formatted string, the string to sign, and a
     * certificate's public key.
     *
     * @param signedRequestHex a hex-encoded string representing the signed request to verify.
     * @param stringToSign the string that will be signed with the public key for
     * verification purposes.
     * @param certificate the certificate containing the public key used for verification.
     * @return true if the signature was verified, false if not.
     * @throws DecoderException if there is an error decoding the hex string.
     * @throws InvalidKeyException if the public key is invalid.
     * @throws SignatureException if the signature algorithm is unable to process the input
     * data provided.
     */
    public static boolean verify(String signedRequestHex, String stringToSign, Certificate certificate) throws DecoderException, InvalidKeyException, SignatureException {
        return verify(signedRequestHex, stringToSign, certificate.getPublicKey());
    }

    /**
     * Verify a signed request using a hex-formatted string, the string to sign, and a certificate's public key.
     *
     * @param signedRequestHex a hex-encoded string representing the signed request to verify.
     * @param stringToSign the string that will be signed with the public key for
     * verification purposes.
     * @param publicKey the public key used for verification.
     * @return true if the signature was verified, false if not.
     * @throws DecoderException if there is an error decoding the hex string.
     * @throws InvalidKeyException if the public key is invalid.
     * @throws SignatureException if the signature algorithm is unable to process the input
     * data provided.
     */
    public static boolean verify(String signedRequestHex, String stringToSign, PublicKey publicKey) throws DecoderException, InvalidKeyException, SignatureException {
        byte[] signedRequest = Hex.decodeHex(signedRequestHex.toCharArray());

        Signature sig;

        try {
            sig = Signature.getInstance(SHA256_WITH_RSA);
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalArgumentException("An error occurred while getting the signature algorithm", e);
        }

        sig.initVerify(publicKey);
        sig.update(stringToSign.getBytes(StandardCharsets.UTF_8));

        return sig.verify(signedRequest);
    }

    /**
     * Create a string for signing using the various necessary components.
     *
     * @param method the HTTP method (e.g. GET, PUT, etc.)
     * @param md5 a MD5 hash of the request entity
     * @param mediaType the media type of a request
     * @param date the timestamp for a request
     * @param uri the uri of a request
     * @return the formatted string to be signed.
     */
    public static String createSigningString(String method, String md5, String mediaType, Date date, URI uri) {
        StringBuilder sb = new StringBuilder();
        sb.append(method).append("\n");
        sb.append(md5).append("\n");
        sb.append(mediaType).append("\n");
        sb.append(getHttpFormattedDate(date)).append("\n");
        sb.append(sanitizeURI(uri));
        return sb.toString();
    }

    /**
     * Sanitizes the URI for signing purposes. Specifically it removes the
     * scheme, userInfo, host and port, leaving only the path, query, and
     * fragment.
     *
     * <p>For example:</p>
     *
     * <p><code>https://username:password@example.com:443/path/to/data?key=value&key2=value2#frag1</code></p>
     * becomes:
     * <p><code>/path/to/data?key=value&key2=value2#frag1</code></p>
     *
     * @param uri the URI to sanitize
     * @return the sanitized URI
     */
    public static URI sanitizeURI(URI uri) {
        try {
            return new URI(null, null, null, -1, uri.getPath(), uri.getQuery(), uri.getFragment());
        } catch (URISyntaxException e) {
            // This should never occur as we're reconstructing a new URI from an existing URI object.
            throw new IllegalStateException("URI Syntax Exception while sanitizing the URI", e);
        }
    }

    /**
     * Convert a date into the format utilized by web browsers
     * (i.e. "EEE, dd MMM yyyy HH:mm:ss z").
     *
     * @param date the date to convert into a timestamp.
     * @return the formatted timestamp string.
     */
    public static String getHttpFormattedDate(Date date) {
        SimpleDateFormat dateFormat = new SimpleDateFormat("EEE, dd MMM yyyy HH:mm:ss z", Locale.US);
        dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        return dateFormat.format(date);
    }

    /**
     * Computes the MD5 hash of a request entity.
     *
     * @param entity the entity to hash.
     * @return the MD5 hash of the entity.
     */
    public static String computeMD5(String entity) {
        if (entity == null || entity.isEmpty()) {
            return "";
        }

        MessageDigest md5;

        try {
            md5 = MessageDigest.getInstance("MD5");
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalArgumentException("An error occurred while getting the message digest", e);
        }

        return Hex.encodeHexString(md5.digest(entity.getBytes(StandardCharsets.UTF_8)));
    }

}
