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

import javax.xml.bind.JAXBContext;

import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.w3c.dom.Node;

import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.identity.wstrust.client.SsoRequestException;

/**
 * ValidateTokenRequestBuilder implementation of the
 * {@link ResponseUnmarshaller} . It can parse responses specific for server
 * side token validation.
 */
public final class ValidateTokenReponseHandler implements ResponseHandler<Boolean> {

    private static final String STATUS_CODE_VALID = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/status/valid";

    private final ResponseUnmarshaller<Boolean> responseUnmarshaller;

    public ValidateTokenReponseHandler(JAXBContext context) throws SsoRequestException {
        this.responseUnmarshaller = new ResponseUnmarshaller<>(context);
    }

    @Override
    public Boolean parseResponse(Node response) throws ParserException, InvalidTokenException {

        RequestSecurityTokenResponseType parsedResponse = responseUnmarshaller.parseStsResponse(response,
                RequestSecurityTokenResponseType.class, false /* skipValidation */);

        new StatusTokenValidator().validateTokenType(parsedResponse);

        boolean status = parsedResponse.getStatus().getCode().equalsIgnoreCase(STATUS_CODE_VALID);

        return status;
    }
}
