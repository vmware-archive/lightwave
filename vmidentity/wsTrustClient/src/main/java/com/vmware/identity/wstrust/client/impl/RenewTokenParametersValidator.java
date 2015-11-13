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

import org.slf4j.LoggerFactory;

import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.TokenCredential;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.ValidateUtil;

class RenewTokenParametersValidator implements RequestParametersValidator {

    private static final String TOKEN_ERR_MSG = "Token is needed for this kind of request";

    private SecurityTokenServiceConfig config;

    public RenewTokenParametersValidator(SecurityTokenServiceConfig stsConfig) {
        this.config = stsConfig;
    }

    @Override
    public void validate(Credential credential, TokenSpec tokenSpec) throws IllegalArgumentException {

        if (!(credential instanceof TokenCredential)) {
            LoggerFactory.getLogger(RenewTokenParametersValidator.class).error(TOKEN_ERR_MSG);
            throw new IllegalArgumentException(TOKEN_ERR_MSG);
        }

        TokenCredential tokenCred = (TokenCredential) credential;

        ValidateUtil.validateNotNull(tokenCred.getToken(), "SAML token");
        ValidateUtil.validatePositiveNumber(tokenSpec.getTokenLifetime(), "token lifetime");
        ValidateUtil.validateNotNull(config.getHolderOfKeyConfig(), "holder of key config");
    }

}
