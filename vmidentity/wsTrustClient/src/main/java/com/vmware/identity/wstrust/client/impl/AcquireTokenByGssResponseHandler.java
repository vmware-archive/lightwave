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

import org.apache.commons.codec.binary.Base64;
import org.oasis_open.docs.ws_sx.ws_trust._200512.BinaryExchangeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.exception.MalformedTokenException;
import com.vmware.identity.wstrust.client.SsoRequestException;

/**
 * AcquireTokenByGssResponseHandler implementation of the
 * {@link ResponseUnmarshaller}. It can parse GSS related responses.
 */
final class AcquireTokenByGssResponseHandler implements ResponseHandler<GssResult> {

    private final ResponseUnmarshaller<Element> responseUnmarshaller;
    private static final String PROCESS_RSTR_ERROR = "Error processing the response to SPNego token request";
    private static final String RSTRC_ELEMENT_NAME = "RequestSecurityTokenResponseCollection";
    private static final String RSTRC_NAMESPACE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512";

    private final Logger log = LoggerFactory.getLogger(AcquireTokenByGssResponseHandler.class);

    public AcquireTokenByGssResponseHandler(JAXBContext context) throws SsoRequestException {
        this.responseUnmarshaller = new ResponseUnmarshaller<>(context);
    }

    @Override
    public GssResult parseResponse(Node response) throws ParserException, InvalidTokenException {

        Element returnedToken = findReturnedToken(response);
        RequestSecurityTokenResponseType parsedResponse = returnedToken == null ? parseIntermediateGssResponse(response)
                : parseFinalGssResponse(response);

        byte[] leg = extractBinaryExchangeData(parsedResponse.getBinaryExchange());

        GssResult parseResult = new GssResult(leg, returnedToken, parsedResponse.getContext());

        return parseResult;
    }

    /**
     * Helper: find, extract and serialize the SAML Token (i.e. <saml:Assertion>
     * element) under the given DOM Node.
     *
     * @param node
     *            the root of the DOM tree to search for a SAML token.
     * @return the serialized form of the token found, if any. If no token is
     *         found, returns {@code null}. If more than one token is found, a
     *         ParserException is thrown.
     */
    private Element findReturnedToken(Node node) throws ParserException {
        NodeList assertionNodes = ((Element) node).getElementsByTagNameNS(ResponseUnmarshaller.TOKEN_TYPE_SAML2,
                ResponseUnmarshaller.ASSERTION_ELEMENT_NAME);

        if (assertionNodes.getLength() == 0) {
            return null;
        }

        if (assertionNodes.getLength() > 1) {
            log.debug(PROCESS_RSTR_ERROR + ": more than one saml:Assertion returned");
            throw new ParserException(PROCESS_RSTR_ERROR);
        }

        return (Element) assertionNodes.item(0);
    }

    /**
     * Helper: Whether response contains an RSTRC
     *
     * @param node
     *            the root of the DOM tree to search for TSTRC.
     * @return true if rstrc is founf false otherwise.
     */
    private boolean hasRSTRC(Node node) throws ParserException {

        boolean found = false;
        if ((RSTRC_ELEMENT_NAME.equalsIgnoreCase(node.getLocalName()))
                && RSTRC_NAMESPACE.equalsIgnoreCase(node.getNamespaceURI())) {
            log.debug("Node is the RSTRC.");
            found = true;
        } else {
            NodeList rstrcNodes = ((Element) node).getElementsByTagNameNS(RSTRC_NAMESPACE, RSTRC_ELEMENT_NAME);

            log.debug(String.format("Found %d RSTRC elements.", ((rstrcNodes == null) ? 0 : rstrcNodes.getLength())));
            found = (rstrcNodes.getLength() > 0);
        }

        return found;
    }

    /**
     * Helper: parse to a JAXB object an intermediate (i.e. not containing a
     * Token) response to acquire-token-by-GSS-negotiation request.
     *
     * @throws ParserException
     *             if the response is not valid.
     */
    private RequestSecurityTokenResponseType parseIntermediateGssResponse(Node response) throws ParserException {

        RequestSecurityTokenResponseType parsedResponse = null;
        boolean hasRSTRC = hasRSTRC(response);

        // parse the response, but don't validate
        if (hasRSTRC) {
            log.debug("Intermediate response contains RSTRC. Parsing RSTR from there.");
            RequestSecurityTokenResponseCollectionType rstrc = responseUnmarshaller.parseStsResponse(response,
                    RequestSecurityTokenResponseCollectionType.class, true);
            parsedResponse = rstrc.getRequestSecurityTokenResponse();
        } else {
            log.debug("Intermediate response does not contain RSTRC. Should be direct RSTR.");
            parsedResponse = responseUnmarshaller.parseStsResponse(response, RequestSecurityTokenResponseType.class,
                    true);
        }

        if (parsedResponse == null) {
            log.error("Unable to get RSTR element from response.");
            throw new ParserException(PROCESS_RSTR_ERROR);
        }

        validateContextId(parsedResponse);

        return parsedResponse;
    }

    /**
     * Helper: parse to a JAXB object the final (i.e. the one containing a
     * token) response to acquire-token-by-GSS-negotiation request.
     *
     * @throws ParserException
     *             if the response is not valid.
     * @throws MalformedTokenException
     *             if the response's structure by itself is valid, but the
     *             contained token is not.
     */
    private RequestSecurityTokenResponseType parseFinalGssResponse(Node response) throws ParserException,
            MalformedTokenException {

        RequestSecurityTokenResponseType parsedResponse = responseUnmarshaller.parseStsResponse(response);

        new SamlTokenValidator().validateTokenType(parsedResponse);

        validateContextId(parsedResponse);

        return parsedResponse;
    }

    /**
     * Helper: validate the given BinaryExchange and extract it's data.
     *
     * @param element
     *            the JAXB object representation of a <wst:BinaryExchange>
     *            element or <code>null</code> when we are on the last
     *            negotiation step when using NTLM protocol.
     * @return the data in byte[] format (i.e. after base64 decoding) or
     *         <code>null</code> when the passed binary exchange parameter was
     *         <code>null</code>
     *
     * @throws ParserException
     *             if the BinaryExchange is not valid.
     */
    private byte[] extractBinaryExchangeData(BinaryExchangeType element) throws ParserException {

        if (element == null) {
            return null;
        }

        if (!Constants.ENCODING_TYPE_BASE64.equals(element.getEncodingType())
                || !Constants.BINARY_EXCHANGE_TYPE_SPNEGO.equals(element.getValueType())) {

            log.debug(PROCESS_RSTR_ERROR);
            throw new ParserException(PROCESS_RSTR_ERROR);
        }

        return Base64.decodeBase64(element.getValue());
    }

    /**
     * Helper: validate the Context attribute of the given
     * RequestSecurityTokenResponse (and throw ParseException if it is not
     * valid).
     */
    private void validateContextId(RequestSecurityTokenResponseType response) throws ParserException {

        String contextId = response.getContext();
        if (contextId == null) {
            log.debug(PROCESS_RSTR_ERROR + ": Context is null");
            throw new ParserException(PROCESS_RSTR_ERROR);
        }
    }
}
