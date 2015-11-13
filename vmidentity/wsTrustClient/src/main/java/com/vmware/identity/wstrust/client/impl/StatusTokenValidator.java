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

import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;

import com.vmware.vim.sso.client.exception.MalformedTokenException;

class StatusTokenValidator implements TokenTypeValidator {

    private static final String TOKEN_TYPE_STATUS = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RSTR/Status";

    @Override
    public void validateTokenType(RequestSecurityTokenResponseType parsedResponse) throws MalformedTokenException {

        if (parsedResponse == null) {
            throw new MalformedTokenException("Unable to find a response to the token validation request");
        }

        if (!TOKEN_TYPE_STATUS.equalsIgnoreCase(parsedResponse.getTokenType())) {
            String message = "Unexpected token type value in the response. Got: " + parsedResponse.getTokenType()
                    + " Expected: " + TOKEN_TYPE_STATUS;
            throw new MalformedTokenException(message);
        }
    }

}
