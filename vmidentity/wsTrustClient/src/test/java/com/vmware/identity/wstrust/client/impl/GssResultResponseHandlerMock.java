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

import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.vmware.vim.sso.client.exception.InvalidTokenException;

/**
 * Mock class for testing purposes
 */
public class GssResultResponseHandlerMock implements ResponseHandler<GssResult> {

    private static final String CONTEXT_ID = "dummyctxid";
    private final boolean _validateCtx;
    private final boolean _throwException;

    public GssResultResponseHandlerMock(boolean immediateReturnToken, boolean throwException) {
        _validateCtx = !immediateReturnToken;
        _throwException = throwException;
    }

    @Override
    public GssResult parseResponse(Node response) throws ParserException, InvalidTokenException {

        if (_throwException) {
            throw new ParserException("Parse exception");
        }
        boolean ctxValid = !_validateCtx || (response != null && response.getFirstChild() != null);

        if (_validateCtx && ctxValid) {
            String messageContext = response.getFirstChild().getTextContent();
            ctxValid = CONTEXT_ID.equals(messageContext);
        }

        Element returnedToken = ctxValid ? TestTokenUtil.getValidSamlTokenElement() : null;

        GssResult gssResult = new GssResult(new byte[] {}, returnedToken, CONTEXT_ID);

        return gssResult;
    }
}
