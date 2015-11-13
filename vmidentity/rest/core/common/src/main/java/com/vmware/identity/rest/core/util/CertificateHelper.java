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

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.Charset;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import org.apache.commons.codec.binary.Base64;

/**
 * The {@code CertificateHelper} class contains utilities for working with
 * {@code X509Certificates} and PEM-formatted encodings of {@code X509Certificates}.
 */
public class CertificateHelper {

    private static final String X509_CERT_TYPE = "X.509";
    private static final String FINGERPRINT_ALGORITHM = "SHA-1";
    private static final String BEGIN_CERT = "-----BEGIN CERTIFICATE-----";
    private static final String END_CERT = "-----END CERTIFICATE-----";
    private static final int LINE_LENGTH = 57;
    private static final byte[] CHUNK_SEPARATOR = { '\n' };

    /**
     * Converts a PEM formatted string to a {@code X509Certificate} instance.
     *
     * @param pem PEM formatted string
     * @return {@link X509Certificate}
     * @throws CertificateException If conversion fails
     */
    public static X509Certificate convertToX509(String pem) throws CertificateException {
        if (pem == null) {
            return null;
        }

        CertificateFactory certFactory = CertificateFactory.getInstance(X509_CERT_TYPE);
        InputStream inputStream = new ByteArrayInputStream(pem.getBytes());
        return (X509Certificate) certFactory.generateCertificate(inputStream);
    }

    /**
     * Encodes an {@code Certificate} into a PEM-formatted encoding.
     *
     * @param certificate the certificate to encode.
     * @return the PEM-formatted encoding of the certificate.
     * @throws CertificateEncodingException if an encoding error occurs.
     */
    public static String convertToPEM(Certificate certificate) throws CertificateEncodingException {
        if (certificate == null) {
            return null;
        }

        return convertToPEM(certificate.getEncoded());
    }

    /**
     * Converts an encoding into a PEM-formatted string.
     *
     * @param encoding the byte array to encode in the PEM format.
     * @return the PEM-formatted string of the encoding.
     */
    public static String convertToPEM(byte[] encoding) {
        StringBuilder builder = new StringBuilder();
        Base64 encoder = new Base64(LINE_LENGTH, CHUNK_SEPARATOR);
        builder.append(BEGIN_CERT).append("\n");

        String encoded = new String(encoder.encode(encoding), Charset.defaultCharset());
        builder.append(encoded);

        builder.append(END_CERT);
        return builder.toString();
    }

    /**
     * Generates the fingerprint of a certificate. The algorithm is as follows:
     * <ol>
     *  <li>Apply 0xFF mask to remove byte's sign extension
     *  <li>Convert every byte to a hex number
     *  <li>Format every hex number as two characters
     *  <li>Put ':' delimiter between every two characters
     * </ol>
     *
     * @param certificate the certificate to generate a fingerprint of.
     * @return the fingerprint string of the certificate.
     * @throws CertificateEncodingException if an encoding error occurs.
     */
    public static String generateFingerprint(Certificate certificate) throws CertificateEncodingException {
        MessageDigest digest;

        try {
            digest = MessageDigest.getInstance(FINGERPRINT_ALGORITHM);
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalArgumentException("An error occurred while getting the message digest", e);
        }

        // Calculate SHA-1 hash
        byte[] hash = digest.digest(certificate.getEncoded());

        char delimiter = ':';
        int len = hash.length * 2 + hash.length - 1;

        StringBuilder fingerprint = new StringBuilder(len);

        for (int i = 0; i < hash.length; i++) {
            hash[i] &= 0xFF;

            fingerprint.append(String.format("%02x", hash[i]));

            if (i < hash.length - 1) {
                fingerprint.append(delimiter);
            }
        }

        return fingerprint.toString();
    }
}
