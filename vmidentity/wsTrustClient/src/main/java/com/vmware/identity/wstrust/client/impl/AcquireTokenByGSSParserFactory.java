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

import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.GSSServerCredential;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.ValidateUtil;

/**
 * This class represents the factory implementation for creating the
 * {@link RequestParametersValidator}, {@link RequestBuilder},
 * {@link ResponseHandler} and {@link WsSecuritySignature} for request token
 * with GSS authentication.
 *
 * @param <GssResult>
 *            Type of the response from the Security Token Service
 */
class AcquireTokenByGSSParserFactory extends RequestParserAbstractFactory<GssResult> {

    public AcquireTokenByGSSParserFactory(SecurityTokenServiceConfig config) {
        super(config);
    }

    @Override
    public RequestParametersValidator createRequestParametersValidator() {
        return new DefaultRequestParametersValidator();
    }

    @Override
    public RequestBuilder createRequestBuilder(Credential clientCredential, TokenSpec tokenSpec) {

        if (!(clientCredential instanceof GSSServerCredential))
            throw new IllegalArgumentException("Credential type not valid for GSSCredentialFactory: "
                    + clientCredential.getClass());

        GSSServerCredential cred = (GSSServerCredential) clientCredential;

        String contextId = cred.getContextId();
        boolean isInitial = ValidateUtil.isEmpty(contextId);
        byte[] leg = cred.getLeg();
        boolean hokConfirmation = stsConfig.getHolderOfKeyConfig() != null;

        return isInitial ? new AcquireTokenByGssInitiateRequestBuilder(tokenSpec, leg, hokConfirmation, jaxbContext,
                stsConfig.getRequestValidityInSeconds()) : new AcquireTokenByGssContinueRequestBuilder(contextId, leg,
                jaxbContext, stsConfig.getRequestValidityInSeconds());
    }

    @Override
    public ResponseHandler<GssResult> createResponseHandler() {
        return new AcquireTokenByGssResponseHandler(jaxbContext);
    }

    @Override
    public WsSecuritySignature createWsSecuritySignature(Credential clientCredential, TokenSpec tokenSpec) {

        GSSServerCredential cred = (GSSServerCredential) clientCredential;

        String contextId = cred.getContextId();
        boolean isInitial = ValidateUtil.isEmpty(contextId);
        boolean isHokRequest = stsConfig.getHolderOfKeyConfig() != null;

        return isInitial && isHokRequest ? WsSecuritySignatureFactory.createWsSecuritySignatureCertificate(stsConfig
                .getHolderOfKeyConfig()) : WsSecuritySignatureFactory.createWsEmptySecuritySignature();
    }

}
