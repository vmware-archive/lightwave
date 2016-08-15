/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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
import java.net.URI;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.CorrelationID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.State;

/**
 * @author Yehia Zayour
 */
public final class LogoutRequest extends ProtocolRequest {
    private static final String HTML_FORM =
            "<html>" +
            "    <head>" +
            "        <script language=\"JavaScript\" type=\"text/javascript\">" +
            "            function load(){ document.getElementById('LogoutRequestForm').submit(); }" +
            "        </script>" +
            "    </head>" +
            "    <body onload=\"load()\">" +
            "        <form method=\"post\" id=\"LogoutRequestForm\" action=\"%s\">" +
            "            %s" +
            "            <input type=\"submit\" value=\"Submit\" style=\"position:absolute; left:-9999px; width:1px; height:1px;\" />" +
            "        </form>" +
            "    </body>" +
            "</html>";

    private static final String HTML_FORM_PARAMETER = "<input type=\"hidden\" name=\"%s\" value=\"%s\" />";

    private final URI uri;
    private final IDToken idTokenHint;
    private final URI postLogoutRedirectUri;
    private final State state;
    private final ClientAssertion clientAssertion;
    private final CorrelationID correlationId;
    private final ClientID clientId;

    public LogoutRequest(
            URI uri,
            IDToken idTokenHint,
            URI postLogoutRedirectUri,
            State state,
            ClientAssertion clientAssertion,
            CorrelationID correlationId) {
        Validate.notNull(uri, "uri");
        Validate.notNull(idTokenHint, "idTokenHint");
        Validate.notNull(postLogoutRedirectUri, "postLogoutRedirectUri");
        Validate.notNull(state, "state");
        // nullable clientAssertion
        // nullable correlationId

        this.uri = uri;
        this.idTokenHint = idTokenHint;
        this.postLogoutRedirectUri = postLogoutRedirectUri;
        this.state = state;
        this.clientAssertion = clientAssertion;
        this.correlationId = correlationId;

        this.clientId = new ClientID(this.idTokenHint.getAudience().get(0));
    }

    public IDToken getIDTokenHint() {
        return this.idTokenHint;
    }

    public URI getPostLogoutRedirectURI() {
        return this.postLogoutRedirectUri;
    }

    public State getState() {
        return this.state;
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

    public String toHtmlForm() {
        Map<String, String> parameters = this.toParameters();
        StringBuilder formParameters = new StringBuilder();
        for (Map.Entry<String, String> entry : parameters.entrySet()) {
            String parameterName = entry.getKey();
            String parameterValue = entry.getValue();
            formParameters.append(String.format(HTML_FORM_PARAMETER, parameterName, parameterValue));
        }

        String formAction = this.uri.toString();
        return String.format(HTML_FORM, formAction, formParameters.toString());
    }

    @Override
    public HttpRequest toHttpRequest() {
        return HttpRequest.createGetRequest(this.uri, toParameters());
    }

    private Map<String, String> toParameters() {
        Map<String, String> result = new HashMap<String, String>();

        result.put("id_token_hint", this.idTokenHint.serialize());
        result.put("post_logout_redirect_uri", this.postLogoutRedirectUri.toString());
        result.put("state", this.state.getValue());
        if (this.clientAssertion != null) {
            result.put("client_assertion", this.clientAssertion.serialize());
        }
        if (this.correlationId != null) {
            result.put("correlation_id", this.correlationId.getValue());
        }

        return result;
    }

    public static LogoutRequest parse(HttpRequest httpRequest) throws ParseException {
        Validate.notNull(httpRequest, "httpRequest");

        Map<String, String> parameters = httpRequest.getParameters();

        ClientID clientId = null;
        URI postLogoutRedirectUri = null;
        State state = null;

        try {
            IDToken idTokenHint = IDToken.parse(ParameterMapUtils.getSignedJWT(parameters, "id_token_hint"));
            if (idTokenHint.getAudience().size() == 1) {
                clientId = new ClientID(idTokenHint.getAudience().get(0));
            }

            postLogoutRedirectUri = ParameterMapUtils.getURI(parameters, "post_logout_redirect_uri");
            state = State.parse(ParameterMapUtils.getString(parameters, "state"));

            ClientAssertion clientAssertion = null;
            if (parameters.containsKey("client_assertion")) {
                clientAssertion = ClientAssertion.parse(parameters);
            }

            CorrelationID correlationId = null;
            if (parameters.containsKey("correlation_id")) {
                correlationId = new CorrelationID(ParameterMapUtils.getString(parameters, "correlation_id"));
            }

            validate(idTokenHint, clientAssertion);

            return new LogoutRequest(
                    httpRequest.getURI(),
                    idTokenHint,
                    postLogoutRedirectUri,
                    state,
                    clientAssertion,
                    correlationId);
        } catch (com.vmware.identity.openidconnect.common.ParseException e) {
            throw new ParseException(e.getErrorObject(), clientId, postLogoutRedirectUri, state, e);
        }
    }

    private static void validate(
            IDToken idTokenHint,
            ClientAssertion clientAssertion) throws com.vmware.identity.openidconnect.common.ParseException {
        if (idTokenHint.getAudience().size() != 1) {
            throw new com.vmware.identity.openidconnect.common.ParseException("id_token_hint must have a single audience value containing the client_id");
        }

        String clientIdString = idTokenHint.getAudience().get(0);

        if (clientAssertion != null && !clientAssertion.getIssuer().getValue().equals(clientIdString)) {
            throw new com.vmware.identity.openidconnect.common.ParseException(ErrorObject.invalidClient("client_assertion issuer must match client_id"));
        }
    }

    public static class ParseException extends Exception {
        private static final long serialVersionUID = 1L;

        private final ErrorObject errorObject;
        private final ClientID clientId;
        private final URI postLogoutRedirectUri;
        private final State state;

        private ParseException(
                ErrorObject errorObject,
                ClientID clientId,
                URI postLogoutRedirectUri,
                State state,
                Throwable cause) {
            super(errorObject.getDescription(), cause);

            this.errorObject = errorObject;
            this.clientId = clientId;
            this.postLogoutRedirectUri = postLogoutRedirectUri;
            this.state = state;
        }

        public ErrorObject getErrorObject() {
            return this.errorObject;
        }

        public ClientID getClientID() {
            return this.clientId;
        }

        public URI getPostLogoutRedirectURI() {
            return this.postLogoutRedirectUri;
        }

        public LogoutErrorResponse createLogoutErrorResponse() {
            LogoutErrorResponse response = null;
            if (this.postLogoutRedirectUri != null && this.state != null) {
                response = new LogoutErrorResponse(this.postLogoutRedirectUri, this.state, this.errorObject);
            }
            return response;
        }
    }
}