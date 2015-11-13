/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client;

/**
 * This class represents the credential for authentication by user certificate. Certificate has to match
 * the certificate provided in the {@link SecurityTokenServiceConfig}
 * Subject and the certificate should be already associated in the token service. Only holder of key tokens can be
 * issued using this credential.
 */
import java.security.cert.Certificate;

public class CertificateCredential extends Credential {
    private Certificate certificate;

    public CertificateCredential(Certificate certificate) {
        ValidateUtil.validateNotNull(certificate, "Certificate");
        this.certificate = certificate;
    }

    public Certificate getCertificate() {
        return certificate;
    }

    @Override
    public String toString() {
        return "CertificateCredential [solution user certificate]";
    }
}
