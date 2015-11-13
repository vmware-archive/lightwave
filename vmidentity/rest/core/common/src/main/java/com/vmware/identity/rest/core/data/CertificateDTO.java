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
package com.vmware.identity.rest.core.data;

import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.util.CertificateHelper;

/**
 * The {@code CertificateDTO} class represents certificates in a JSON format intended for
 * consumption by REST SSO classes.
 */
@JsonDeserialize(builder=CertificateDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class CertificateDTO extends DTO {

    private final String encoded;

    @JsonIgnore
    private final X509Certificate x509Certificate;
    @JsonIgnore
    private String fingerprint;

    /**
     * Construct a {@code CertificateDTO} from an {@code X509Certificate}. Converts the certificate
     * into a PEM-formatted encoding for JSON object.
     *
     * @param certificate the {@code X509Certificate} that the {@code CertificateDTO} will
     * represent.
     * @throws CertificateEncodingException if an error occurs while converting the certificate
     * into a PEM-formatted encoding.
     */
    public CertificateDTO(X509Certificate certificate) throws CertificateEncodingException {
        this.x509Certificate = certificate;
        this.encoded = CertificateHelper.convertToPEM(certificate);
    }

    /**
     * Constructs a {@code CertificateDTO} from a PEM-formatted encoding of an {@code X509Certificate}.
     * Converts the encoding into an {@code X509Certificate} for the sake of convenience.
     *
     * @param encoded the PEM-formatted encoding of an {@code X509Certificate}.
     * @throws CertificateException if an error occurs while converting the encoding into an
     * {@code X509Certificate}.
     */
    public CertificateDTO(String encoded) throws CertificateException {
        this.encoded = encoded;
        this.x509Certificate = CertificateHelper.convertToX509(encoded);
    }

    /**
     * Get the PEM-formatted encoding of the underlying {@code X509Certificate}.
     *
     * @return the PEM-formatted encoding of the {@code X509Certificate}.
     */
    public String getEncoded() {
        return encoded;
    }

    /**
     * Get the underlying {@link X509Certificate} that this object represents.
     *
     * @return the {@code X509Certificate} that this object represents.
     */
    public X509Certificate getX509Certificate() {
        return x509Certificate;
    }

    /**
     * Get the fingerprint string for the underlying {@code X509Certificate}.
     *
     * @return a fingerprint string for the underlying {@code X509Certificate}.
     * @throws CertificateEncodingException if an encoding error occurs.
     * @see CertificateHelper#generateFingerprint(java.security.cert.Certificate)
     */
    public String getFingerprint() throws CertificateEncodingException {
        if (fingerprint == null) {
            fingerprint = CertificateHelper.generateFingerprint(x509Certificate);
        }

        return fingerprint;
    }

    /**
     * Create an instance of the {@link CertificateDTO.Builder} class.
     *
     * @return a new {@code CertificateDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    /**
     * The JSON POJO Builder for this class. The builder class is meant mostly for
     * usage when constructing the object from its JSON string and thus only accepts
     * content for the canonical fields of the JSON representation. Other constructors
     * may exist that provide greater convenience.
     */
    @JsonIgnoreProperties(ignoreUnknown=true)
    @JsonPOJOBuilder
    public static class Builder {
        private String encoded;

        public Builder withEncoded(String encoded) {
            this.encoded = encoded;
            return this;
        }

        public CertificateDTO build() throws CertificateException {
            return new CertificateDTO(encoded);
        }
    }

}
