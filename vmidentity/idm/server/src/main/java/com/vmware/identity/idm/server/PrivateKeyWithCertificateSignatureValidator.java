/*
 *
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
 *
 */
package com.vmware.identity.idm.server;

import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.cert.Certificate;

class PrivateKeyWithCertificateSignatureValidator {

    private static final String DEFAULT_SIGNATURE_ALG = "SHA256withRSA";

    public void validate(PrivateKey key, Certificate cert) throws IllegalArgumentException {

        if (!key.getAlgorithm().equals(cert.getPublicKey().getAlgorithm())) {
            throw new IllegalArgumentException(
                    "Private key algorithm does not match algorithm of public key in certificate (at index 0)");
        }

        try {
            byte[] originalText = "Signed text".getBytes();
            byte[] signedText = null;

            Signature signature = Signature.getInstance(DEFAULT_SIGNATURE_ALG);
            signature.initSign(key);
            signature.update(originalText);
            signedText = signature.sign();

            signature.initVerify(cert);
            signature.update(originalText);
            boolean verified = signature.verify(signedText);

            if (!verified) {
                throw new IllegalArgumentException("private key does not match certificate (at index 0)");
            }
        } catch (NoSuchAlgorithmException e) {
            throw new IllegalArgumentException(e.getMessage(), e);
        } catch (Exception e) {
            throw new IllegalArgumentException("private key does not match certificate (at index 0)", e);
        }
    }
}
