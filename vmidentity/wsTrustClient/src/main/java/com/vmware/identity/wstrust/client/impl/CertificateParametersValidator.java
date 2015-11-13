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
package com.vmware.identity.wstrust.client.impl;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.CertificateCredential;
import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.ValidateUtil;

class CertificateParametersValidator implements RequestParametersValidator {

    private static final String HOK_ERR_MSG = "Holder-of-key configuration is needed for this kind of request";
    private static final String CERT_ERR_MSG = "Certificate is needed for this kind of request";
    private static final String CERT_HOK_CERT_ERR_MSG = "Certificate and Holder-of-key configuration certificate must be equal";

    private SecurityTokenServiceConfig config;
    private Logger log = LoggerFactory.getLogger(CertificateParametersValidator.class);

    public CertificateParametersValidator(SecurityTokenServiceConfig stsConfig) {
        this.config = stsConfig;
    }

    @Override
    public void validate(Credential credential, TokenSpec tokenSpec) throws IllegalArgumentException {

        ValidateUtil.validateNotNull(credential, "Client credential");
        ValidateUtil.validateNotNull(tokenSpec, "Token specification");

        if (!(credential instanceof CertificateCredential)) {
            log.error(CERT_ERR_MSG);
            throw new IllegalArgumentException(CERT_ERR_MSG);
        }

        if (config.getHolderOfKeyConfig() == null) {
            log.error(HOK_ERR_MSG);
            throw new IllegalArgumentException(HOK_ERR_MSG);
        }

        CertificateCredential certCredential = (CertificateCredential) credential;
        if (!certCredential.getCertificate().equals(config.getHolderOfKeyConfig().getCertificate())) {
            log.error(CERT_HOK_CERT_ERR_MSG);
            throw new IllegalArgumentException(CERT_HOK_CERT_ERR_MSG);
        }
    }
}
