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
package com.vmware.identity.rest.idm.data;

import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.Arrays;

import org.apache.commons.codec.binary.Base64;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonInclude.Include;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.annotation.JsonPOJOBuilder;
import com.vmware.identity.rest.core.data.DTO;

/**
 * The {@code PrivateKeyDTO} class represents private keys in an encoded format.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@JsonDeserialize(builder = PrivateKeyDTO.Builder.class)
@JsonIgnoreProperties(ignoreUnknown=true)
@JsonInclude(Include.NON_EMPTY)
public class PrivateKeyDTO extends DTO {

    private final String encoded;
    private final String algorithm;

    @JsonIgnore
    private final PrivateKey privateKey;

    /**
     * Construct a {@code PrivateKeyDTO} from the encoded string and algorithm.
     *
     * @param encoded a base-64 encoded string containing the encoded contents of a private key
     * @param algorithm name of the standard algorithm for the private key (e.g. RSA, DSA, etc.)
     * @throws InvalidKeySpecException if the requested key specification is inappropriate for the
     *  given key, or the given key cannot be processed (e.g., the given key has an unrecognized
     *  algorithm or format).
     * @throws NoSuchAlgorithmException if no Provider supports a KeyFactorySpi implementation for
     *  the specified algorithm.
     */
    public PrivateKeyDTO(String encoded, String algorithm) throws InvalidKeySpecException, NoSuchAlgorithmException {
        this.encoded = encoded;
        this.algorithm = algorithm;

        this.privateKey = decodePrivateKey(encoded, algorithm);
    }

    /**
     * Construct a {@code PrivateKeyDTO} from a {@link PrivateKey}.
     *
     * @param privateKey the private key to build the DTO from.
     * @throws InvalidKeySpecException if the requested key specification is inappropriate for the
     *  given key, or the given key cannot be processed (e.g., the given key has an unrecognized
     *  algorithm or format).
     * @throws NoSuchAlgorithmException if no Provider supports a KeyFactorySpi implementation for
     *  the specified algorithm.
     */
    public PrivateKeyDTO(PrivateKey privateKey) throws InvalidKeySpecException, NoSuchAlgorithmException {
        this.privateKey = privateKey;
        this.algorithm = privateKey == null ? null : privateKey.getAlgorithm();

        this.encoded = encodePrivateKey(privateKey);
    }

    /**
     * Get the base-64 encoded representation of the underlying private key.
     *
     * @return a base-64 encoded string representing the underlying private key.
     */
    public String getEncoded() {
        return encoded;
    }

    /**
     * Get the standard algorithm name for this key.
     * See Appendix A in the <a href=
     * "../../../technotes/guides/security/crypto/CryptoSpec.html#AppA">
     * Java Cryptography Architecture API Specification &amp; Reference </a>
     * for information about standard algorithm names.
     *
     * @return the name of the algorithm associated with this key.
     */
    public String getAlgorithm() {
        return algorithm;
    }

    /**
     * Get the underlying {@link PrivateKey} that this object represents.
     *
     * @return the {@code PrivateKey} that this object represents.
     */
    public PrivateKey getPrivateKey() {
        return privateKey;
    }

    /**
     * Creates an instance of the {@link PrivateKeyDTO.Builder} class.
     *
     * @return a new {@code PrivateKeyDTO.Builder}.
     */
    public static Builder builder() {
        return new Builder();
    }

    private static String encodePrivateKey(PrivateKey key) throws InvalidKeySpecException, NoSuchAlgorithmException {
        if (key == null) {
            return null;
        }

        KeyFactory keyFactory = KeyFactory.getInstance(key.getAlgorithm());
        PKCS8EncodedKeySpec spec = keyFactory.getKeySpec(key, PKCS8EncodedKeySpec.class);
        byte[] packed = spec.getEncoded();
        String encodePrivateKey = Base64.encodeBase64String(packed);
        Arrays.fill(packed, (byte) 0);
        return encodePrivateKey;
    }

    private static PrivateKey decodePrivateKey(String encoded, String algorithm) throws InvalidKeySpecException, NoSuchAlgorithmException {
        if (encoded == null) {
            return null;
        }

        byte[] clear = Base64.decodeBase64(encoded);
        PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(clear);
        KeyFactory fact = KeyFactory.getInstance(algorithm);
        PrivateKey privateKey = fact.generatePrivate(keySpec);
        Arrays.fill(clear, (byte) 0);
        return privateKey;
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
        private String algorithm;

        public Builder withEncoded(String encoded) {
            this.encoded = encoded;
            return this;
        }

        public Builder withAlgorithm(String algorithm) {
            this.algorithm = algorithm;
            return this;
        }

        public PrivateKeyDTO build() throws InvalidKeySpecException, NoSuchAlgorithmException {
            return new PrivateKeyDTO(encoded, algorithm);
        }
    }

}
