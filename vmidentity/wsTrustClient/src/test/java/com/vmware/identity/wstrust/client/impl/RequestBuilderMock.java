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

/**
 * Test mock
 */
public class RequestBuilderMock implements RequestBuilder {

    private final boolean _throwException;
    private final String _contextId;

    private int _requestValidityInSeconds = 600;
    private final String _soapAction;

    public RequestBuilderMock(boolean throwException, int requestValidityInSeconds, String soapAction) {
        _throwException = throwException;
        _contextId = null;
        _requestValidityInSeconds = requestValidityInSeconds;
        this._soapAction = soapAction;
    }

    public RequestBuilderMock(String contextId, String soapAction) {
        _throwException = false;
        _contextId = contextId;
        this._soapAction = soapAction;
    }

    @Override
    public SoapMessage createRequest() throws ParserException {
        if (_throwException) {
            throw new ParserException("Parse exception");
        }

        return (_contextId != null && !_contextId.equals("")) ? TestUtil.createSoapMessage(_contextId, _soapAction)
                : TestUtil.createSoapMessage(_soapAction);
    }

    @Override
    public int getRequestValidityInSeconds() {
        return _requestValidityInSeconds;
    }

}
