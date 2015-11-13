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

class AcquireRequestGSSParserFactoryTest extends RequestParserAbstractFactory<GssResult> {

    private static final String WS_TRUST_ISSUE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Issue";

    private boolean immediateReturnToken = true;
    private boolean throwException = false;

    public AcquireRequestGSSParserFactoryTest(SecurityTokenServiceConfig config, boolean immediateReturnToken,
            boolean throwException) {
        super(config);
        this.immediateReturnToken = immediateReturnToken;
        this.throwException = throwException;
    }

    @Override
    public RequestParametersValidator createRequestParametersValidator() {
        return new DefaultRequestParametersValidator();
    }

    @Override
    public RequestBuilder createRequestBuilder(Credential clientCredential, TokenSpec tokenSpec) {
        return new RequestBuilderMock(((GSSServerCredential) clientCredential).getContextId(), WS_TRUST_ISSUE);
    }

    @Override
    public ResponseHandler<GssResult> createResponseHandler() {
        return new GssResultResponseHandlerMock(this.immediateReturnToken, this.throwException);
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
