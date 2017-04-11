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

package com.vmware.identity.sts.auth.impl;

import java.io.ByteArrayInputStream;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.Signature;
import java.security.SignatureException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.Date;

import org.apache.commons.codec.binary.Base64;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.BinarySecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SignatureAlgorithmType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UserCertificateTokenType;
import org.w3._2000._09.xmldsig_.SignatureValueType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.util.JAXBExtractor;

final class UserCertAuthenticator implements Authenticator {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(UserCertAuthenticator.class);

    private final com.vmware.identity.sts.idm.Authenticator idmAuthenticator;

    UserCertAuthenticator(com.vmware.identity.sts.idm.Authenticator idmAuthenticator) {
        assert idmAuthenticator != null;

        this.idmAuthenticator = idmAuthenticator;
    }

    /* (non-Javadoc)
     * @see com.vmware.identity.sts.auth.Authenticator#authenticate(com.vmware.identity.sts.Request)
     */
    @Override
    public Result authenticate(Request req) throws AuthenticationFailedException, UnsupportedSecurityTokenException, NoSuchIdPException,
            RequestFailedException {
        assert req != null && req.getHeader() != null;
        log.trace("Authenticating by User Cert...");

        UserCertificateTokenType userCertToken = JAXBExtractor.extractFromSecurityHeader(req.getHeader(), UserCertificateTokenType.class);
        if (userCertToken == null) {
            log.debug("No UserCertificateToken found!");
            return null;
        }

        SignatureAlgorithmType signatureAlgorithm = userCertToken.getSignatureAlgorithm();
        BinarySecurityTokenType binarySecurityToken = userCertToken.getUserCertificate();
        String signedInfo = userCertToken.getSignatureInfo();
        SignatureValueType signatureValueType = userCertToken.getSignatureValue();
        if (signatureAlgorithm == null || binarySecurityToken == null || signedInfo == null || signatureValueType == null) {
            throw new InvalidCredentialsException("User certificate token does not include all required information!");
        }

        if (signatureAlgorithm != SignatureAlgorithmType.SHA_256_WITH_RSA) {
            log.debug(String.format("UserCertificateToken should use %s algorithm, but %s found. "
                    + "No authentication will be done",
                    SignatureAlgorithmType.SHA_256_WITH_RSA.value(),
                    signatureAlgorithm.value()));
            throw new InvalidCredentialsException("User certificate token signature algorithm should be SHA256RSA!");
        }

        String userCertificate = binarySecurityToken.getValue();

        byte[] decoded = new Base64().decode(userCertificate);
        X509Certificate x509Certificate = null;
        try {
            x509Certificate =
                    (X509Certificate) CertificateFactory.getInstance("X.509").generateCertificate(new ByteArrayInputStream(decoded));
        } catch (CertificateException e) {
            throw new InvalidCredentialsException("User certificate is not valid in User certificate token");
        }

        byte[] signatureValue = signatureValueType.getValue();
        if (signatureValue == null) {
            throw new InvalidCredentialsException("Signature value is required in User certificate token!");
        }

        if (!verifyUserCertSignature(x509Certificate, signedInfo, signatureValue)) {
            throw new InvalidCredentialsException("User certificate token signature validation failed.");
        }

        PrincipalId authenticatedPrincipal = this.idmAuthenticator.authenticate(new X509Certificate[] { x509Certificate });

        final Date authenticationTime = new Date();
        log.debug("Authenticated principal: {} by user certificate at time: {}", authenticatedPrincipal, authenticationTime);

        return new Result(authenticatedPrincipal, authenticationTime, Result.AuthnMethod.SMARTCARD);
    }

    private boolean verifyUserCertSignature(X509Certificate x509Certificate, String signedInfo, byte[] signatureValue) {

        try {
            PublicKey publicKey = x509Certificate.getPublicKey();
            Signature signature = Signature.getInstance("SHA256WithRSA");
            signature.initVerify(publicKey);
            signature.update(signedInfo.getBytes());
            return signature.verify(signatureValue);
        } catch (NoSuchAlgorithmException | InvalidKeyException | SignatureException e) {
            throw new InvalidCredentialsException("User certificate token signature validation failed.", e);
        }
    }
}
