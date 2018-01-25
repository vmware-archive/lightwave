/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

package com.vmware.identity.openidconnect.protocol;

import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;

public class FederatedTokenRequest {

    private final IDToken idTokenHint;
    private final ClientAssertion clientAssertion;
    private final CorrelationID correlationId;
    private final ClientID clientId;

    public FederatedTokenRequest(IDToken idTokenHint, ClientAssertion clientAssertion,
            CorrelationID correlationId) {
        Validate.notNull(idTokenHint, "idTokenHint");
        // nullable clientAssertion
        // nullable correlationId

        this.idTokenHint = idTokenHint;
        this.clientAssertion = clientAssertion;
        this.correlationId = correlationId;
        this.clientId = new ClientID(this.idTokenHint.getAudience().get(0));
    }

    public ClientAssertion getClientAssertion() {
        return this.clientAssertion;
    }

    public CorrelationID getCorrelationID() {
        return this.correlationId;
    }

    public ClientID getClientID() {
        return this.clientId;
    }

    public IDToken getIDTokenHint() {
        return this.idTokenHint;
    }

    public static FederatedTokenRequest parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");

        Map<String, String> parameters = httpRequest.getParameters();
        IDToken idTokenHint = IDToken.parse(ParameterMapUtils.getSignedJWT(parameters, "id_token_hint"));

        ClientAssertion clientAssertion = null;
        if (parameters.containsKey("client_assertion")) {
            clientAssertion = ClientAssertion.parse(parameters);
        }

        CorrelationID correlationId = null;
        if (parameters.containsKey("X-Request-Id")) {
            correlationId = new CorrelationID(ParameterMapUtils.getString(parameters, "X-Request-Id"));
        } else if (parameters.containsKey("correlation_id")) {
            correlationId = new CorrelationID(ParameterMapUtils.getString(parameters, "correlation_id"));
        }

        validate(idTokenHint, clientAssertion);

        return new FederatedTokenRequest(idTokenHint, clientAssertion, correlationId);
    }

    private static void validate(IDToken idTokenHint, ClientAssertion clientAssertion)
            throws com.vmware.identity.openidconnect.common.ParseException {
        if (idTokenHint.getAudience().size() != 1) {
            throw new com.vmware.identity.openidconnect.common.ParseException(
                    "id_token_hint must have a single audience value containing the client_id");
        }

        String clientIdString = idTokenHint.getAudience().get(0);

        if (clientAssertion != null && !clientAssertion.getIssuer().getValue().equals(clientIdString)) {
            throw new com.vmware.identity.openidconnect.common.ParseException(
                    ErrorObject.invalidClient("client_assertion issuer must match client_id"));
        }
    }
}
