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
package com.vmware.identity.samlservice;

import java.io.IOException;

import javax.servlet.http.HttpServletRequest;

import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.Response;
import org.opensaml.ws.message.decoder.MessageDecodingException;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.io.UnmarshallingException;
import org.opensaml.xml.security.SecurityException;
import org.w3c.dom.Document;

public interface SamlService {
    /**
     * Signs message with known private key and algorithm. Throws on error.
     *
     * @param message
     * @return signature
     */
    String signMessage(String message) throws IllegalStateException;

    /**
     * Verifies signature of the message with a known public key and signature
     * algorithm. Throws on error.
     *
     * @param message
     * @param signature
     */
    void verifySignature(String message, String signature)
            throws IllegalStateException;

    /**
     * Decode the message sent to us.
     *
     * @param request
     */
    SignableSAMLObject decodeSamlRequest(HttpServletRequest request)
            throws MessageDecodingException, SecurityException;

    /**
     * Decode the message sent to us.
     *
     * @param request
     */
    AuthnRequest decodeSamlAuthnRequest(HttpServletRequest request)
            throws MessageDecodingException, SecurityException;

    /**
     * Creates SAML response object populated as needed.
     *
     * @param inResponseTo
     *            - the ID of the request
     * @param where
     *            - intended destination of our response
     * @param status
     *            - SAML status string
     * @param substatus
     *            - SAML substatus string (optional for some statuses)
     * @param message
     *            - user-friendly message (optional)
     * @param token
     *            - saml token/assertion in the form of XML DOM document
     *            (optional)
     * @throws UnmarshallingException
     */
    Response createSamlResponse(String inResponseTo, String where,
            String status, String substatus, String message, Document token)
            throws UnmarshallingException;

    /**
     * Creates auto-submit HTML form which will post specified response to the
     * specified destination
     *
     * @param response
     *            - response object to encode
     * @param relayState
     *            - (optional) relayState from original request
     * @param where
     *            - intended destination of our response
     */
    String buildPostResponseForm(Response response, String relayState,
            String where);

    /**
     * Create SAML request (expect callers to ensure that ServerConfig is
     * loaded)
     *
     * @param id
     * @param destination
     * @param providerEntityID
     * @param nameIDFormat
     * @param assertionConsumerServiceIndex
     * @param attributeConsumerServiceIndex
     * @param forceAuthn
     * @param isPassive
     * @return
     */
    public AuthnRequest createSamlAuthnRequest(String id, String destination,
            String providerEntityID, String nameIDFormat,
            Integer assertionConsumerServiceIndex,
            Integer attributeConsumerServiceIndex, Boolean forceAuthn,
            Boolean isPassive);

    /**
     * Create SAML Logout request
     *
     * @param id
     * @param where
     * @param nameIDFormat
     * @param nameID
     * @param sessionIndex
     * @return
     */
    public LogoutRequest createSamlLogoutRequest(String id, String where,
            String nameIDFormat, String nameID, String sessionIndex);

    /**
     * Creates SAML logout response object populated as needed.
     *
     * @param inResponseTo
     *            - the ID of the request
     * @param where
     *            - intended destination of our response
     * @param status
     *            - SAML status string
     * @param substatus
     *            - SAML substatus string (optional for some statuses)
     * @param message
     *            - user-friendly message (optional)
     */
    LogoutResponse createSamlLogoutResponse(String inResponseTo, String where,
            String status, String substatus, String message);

    /**
     * Generate query string parameters given parameter values. Does not include
     * '?' sign in the output
     *
     * @param samlRequest
     *            SAML Request
     * @param samlResponse
     *            SAML Response
     * @param relayState
     *            relay state
     * @param sigAlg
     *            signature algorithm
     * @param signature
     *            signature
     * @return
     */
    String generateRedirectUrlQueryStringParameters(String samlRequest,
            String samlResponse, String relayState, String sigAlg,
            String signature);

    /**
     * Encode SAML object (request/response) as a query string parameter for
     * REDIRECT binding
     *
     * @param signableSAMLObject
     * @return
     * @throws MarshallingException
     * @throws IOException
     */
    String encodeSAMLObject(SignableSAMLObject signableSAMLObject)
            throws MarshallingException, IOException;
}