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
package com.vmware.identity.rest.idm.server.test.integration.util;

import java.security.cert.Certificate;
import java.util.Collection;

import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.CertificateUtil;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * Certificate utility which helps calling IDM directly. This helper is mostly used in two phases while running certificate resource integration tests :
 * <li> Preparing test set-up - Before running integration tests </li>
 * <li> Cleaning up set-up - After running integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class CertificateResourceHelper {

    private CasIdmClient idmClient;


    public CertificateResourceHelper(CasIdmClient idmClient) {
        this.idmClient = idmClient;
    }

    public Collection<Certificate> getAllStsCertificates(String tenantName) throws Exception {
        return idmClient.getAllCertificates(tenantName, CertificateType.STS_TRUST_CERT);
    }

    public void delete(String tenantName, String fingerprint) throws Exception {
        idmClient.deleteCertificate(tenantName, fingerprint, CertificateType.STS_TRUST_CERT);
    }

    public void createCertificate(String tenantName, Certificate cert) throws Exception {
        idmClient.addCertificate(tenantName, cert, CertificateType.STS_TRUST_CERT);
    }

    public Certificate getSTSCertificate(String tenantName, String fingerprint) throws Exception {
        Certificate stsCertificate = null;
        Collection<Certificate> stsCerts = idmClient.getAllCertificates(tenantName, CertificateType.STS_TRUST_CERT);
        for (Certificate cert : stsCerts) {
            String certFingerPrint = CertificateUtil.generateFingerprint(cert.getEncoded());
            if (certFingerPrint.equalsIgnoreCase(fingerprint)) {
                stsCertificate = cert;
                break;
            }
        }
        return stsCertificate;
    }

}
