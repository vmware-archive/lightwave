/* *************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved. VMware Confidential
 * ************************************************************************/
package com.vmware.identity.websso.client;

import org.apache.commons.lang.Validate;

/**
 * Signature algorithms for making a signature of a token.
 */
public enum SignatureAlgorithm {
    /**
     * RSA SHA1 algorithm for signing
     */
    RSA_SHA1("http://www.w3.org/2000/09/xmldsig#rsa-sha1", "SHA1withRSA"),
    /**
     * RSA SHA256 algorithm for signing
     */
    RSA_SHA256("http://www.w3.org/2001/04/xmldsig-more#rsa-sha256", "SHA256withRSA"),
    /**
     * RSA SHA384 algorithm for signing
     */
    ALGO_ID_SIGNATURE_RSA_SHA384("http://www.w3.org/2001/04/xmldsig-more#rsa-sha384", "SHA384withRSA"),
    /**
     * RSA SHA512 algorithm for signing
     */
    RSA_SHA512("http://www.w3.org/2001/04/xmldsig-more#rsa-sha512", "SHA512withRSA");

    private final String algorithmURI;
    private final String algorithmName;

    private SignatureAlgorithm(String algorithmURI, String algorithmName) {
        assert algorithmURI != null;
        assert algorithmName != null;

        this.algorithmURI = algorithmURI;
        this.algorithmName = algorithmName;
    }

    @Override
    public String toString() {
        return algorithmURI;
    }

    /**
     * @return signature algorithm name. Cannot be null.
     */
    public String getAlgorithmName() {
        return algorithmName;
    }

    /**
     * Returns SignatureAlgorithm matching specific algorithm URI
     * 
     * @param signatureAlgorithmURI
     *            cannot be empty.
     * @return One of SignatureAlgorithm enum values or null if there is no such
     *         algorithm supported and/or algorithmURI is unknown.
     */
    public static SignatureAlgorithm getSignatureAlgorithmForURI(String signatureAlgorithmURI) {
        Validate.notEmpty(signatureAlgorithmURI);

        SignatureAlgorithm result = null;
        for (SignatureAlgorithm algo : SignatureAlgorithm.values()) {
            if (algo.toString().equals(signatureAlgorithmURI)) {
                result = algo;
                break;
            }
        }
        return result;
    }
}