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

import com.vmware.vim.sso.client.ConfirmationType;
import com.vmware.identity.token.impl.ValidateUtil;
import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.TokenCredential;
import com.vmware.identity.wstrust.client.TokenSpec;

class TokenParametersValidator implements RequestParametersValidator {

    private static final String HOK_ERR_MSG = "Holder-of-key configuration is needed for this kind of request";
    private static final String TOKEN_ERR_MSG = "Token is needed for this kind of request";
    private static final String BEARER_ERR_MSG = "Bearer tokens cannot be used for token exchange";
    private static final String DELEGATION_NOT_SUPPORTED_ERR_MSG = "Delegation is not supported for external tokens.";
    private static final String HOK_NOT_SUPPORTED_ERR_MSG = "Cannot acquire Holder-of-key token by Bearer token.";

    private Logger log = LoggerFactory.getLogger(TokenParametersValidator.class);
    private SecurityTokenServiceConfig config;

    public TokenParametersValidator(SecurityTokenServiceConfig stsConfig) {
        this.config = stsConfig;
    }

    @Override
    public void validate(Credential credential, TokenSpec tokenSpec) throws IllegalArgumentException {

        ValidateUtil.validateNotNull(credential, "Client credential");
        ValidateUtil.validateNotNull(tokenSpec, "Token specifications");

        if (!(credential instanceof TokenCredential)) {
            log.error(TOKEN_ERR_MSG);
            throw new IllegalArgumentException(TOKEN_ERR_MSG);
        }

        TokenCredential tokenCred = (TokenCredential) credential;

        if (!tokenCred.isExternal()) {
            // Bearer tokens cannot be used for token exchange
            if (tokenCred.getToken().getConfirmationType() == ConfirmationType.BEARER) {
                log.error(BEARER_ERR_MSG);
                throw new IllegalArgumentException(BEARER_ERR_MSG);
            }

            checkHoKConfig();
        } else {
            // we are not going to support delegation for the external tokens
            if (tokenSpec.getDelegationSpec() != null) {
                throw new IllegalArgumentException(DELEGATION_NOT_SUPPORTED_ERR_MSG);
            }

            // if external token is bearer - cannot acquire Hok token
            if (tokenCred.getToken().getConfirmationType() == ConfirmationType.HOLDER_OF_KEY) {
                checkHoKConfig();
            } else if ((tokenSpec.getConfirmation() != TokenSpec.Confirmation.NONE)
                    && (config.getHolderOfKeyConfig() != null)) {
                throw new IllegalArgumentException(HOK_NOT_SUPPORTED_ERR_MSG);
            }
        }
    }

    private void checkHoKConfig() {
        if (config.getHolderOfKeyConfig() == null) {
            log.error(HOK_ERR_MSG);
            throw new IllegalArgumentException(HOK_ERR_MSG);
        }
    }

}
