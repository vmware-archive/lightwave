/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import com.vmware.certificate.Request;
import com.vmware.certificate.VMCAClient;

public class SigningCertificates {
    private static final String CERTIFICATE_COUNTRY = "US";
    private static long NUM_MSECS_IN_DAY = 24 * 60 * 60 * 1000;
    private static long NUM_MSECS_IN_YEAR = 365 * NUM_MSECS_IN_DAY;
    private static Integer KEY_LENGTH = 4096;

    private List<Certificate> _certs;
    private PrivateKey _key;

    public SigningCertificates(List<Certificate> certs, PrivateKey key) {
        Validate.validateNotNull(certs, "Certificate list");
        Validate.validateNotNull(key, "Private Key");
        _certs = certs;
        _key = key;
    }

    public List<Certificate> getCerts() {
        return _certs;
    }

    public PrivateKey getKey() {
        return _key;
    }

    public static SigningCertificates create(String hostname,
            String domainName, String username, String password)
            throws Exception {

        VMCAClient vmcaClient = new VMCAClient(username, domainName, password,
                hostname);
        Certificate[] certs = null;

        X509Certificate rootCert = vmcaClient.getRootCertificate();

        Request certRequest = new Request();
        certRequest.setCountry(CERTIFICATE_COUNTRY);

        KeyPair keyPair = certRequest.createKeyPair(KEY_LENGTH);

        Date now = new Date();
        Date notBefore = new Date(now.getTime() - 3 * NUM_MSECS_IN_DAY);
        Date notAfter = new Date(now.getTime() + 10 * NUM_MSECS_IN_YEAR);

        X509Certificate leafCert = vmcaClient.getCertificate(certRequest,
                keyPair, notBefore, notAfter);

        if (rootCert != null && leafCert != null) {
            certs = new Certificate[2];
            certs[0] = leafCert;
            certs[1] = rootCert;
        }

        if (certs == null) {
            throw new IllegalStateException(
                    "Error: No signing certificates found");
        }

        return new SigningCertificates(Arrays.asList(certs),
                keyPair.getPrivate());
    }
}
