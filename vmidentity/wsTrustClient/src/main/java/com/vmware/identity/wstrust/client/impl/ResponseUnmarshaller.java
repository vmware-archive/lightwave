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
import javax.xml.bind.JAXBElement;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Unmarshaller;
import javax.xml.validation.Schema;

import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Node;
import org.xml.sax.SAXException;

import com.vmware.identity.token.impl.SamlTokenImpl;
import com.vmware.identity.token.impl.Util;
import com.vmware.identity.wstrust.client.SsoRequestException;

/**
 * Contains various common methods needed by ResponseUnmarshaller.
 */
class ResponseUnmarshaller<ResponseType> {

    public static final String TOKEN_TYPE_SAML2 = "urn:oasis:names:tc:SAML:2.0:assertion";
    public static final String ASSERTION_ELEMENT_NAME = "Assertion";

    private static final String ERR_LOADING_WS_TRUST_SCHEMA = "Error loading WS-Trust schema file";

    private final Logger log = LoggerFactory.getLogger(ResponseUnmarshaller.class);

    /*
     * NB: This class makes references to resources. If changing package of the
     * class consider also change in the resources location.
     */
    private static final String WS_TRUST_1_3_SCHEMA = "vmware-ws-trust-1.3.xsd";
    private static Schema wsTrustSchema = loadWsTrustSchema();

    private final JAXBContext jaxbContext;

    public ResponseUnmarshaller(JAXBContext context) throws SsoRequestException {
        if (wsTrustSchema == null) {
            throw new SsoRequestException(ERR_LOADING_WS_TRUST_SCHEMA);
        }

        assert context != null;
        this.jaxbContext = context;
    }

    /**
     * Parse and validate WS-Trust response of type
     * &lt;wst:RequestSecurityTokenResponseCollection&gt;, convert it to JAXB
     * object representation and return the single embedded
     * RequestSecurityTokenResponse object.
     */
    protected final RequestSecurityTokenResponseType parseStsResponse(Node soapResponseNode) throws ParserException {

        return parseStsResponse(soapResponseNode, RequestSecurityTokenResponseCollectionType.class, false)
                .getRequestSecurityTokenResponse();
    }

    /**
     * Parse and, optionally, validate WS-Trust response of type
     * "StsResponseType".
     */
    protected final <StsResponseType> StsResponseType parseStsResponse(Node soapResponseNode,
            Class<StsResponseType> responseType, boolean skipValidation) throws ParserException {

        JAXBElement<StsResponseType> jaxbParserResult;

        try {
            Unmarshaller unmarshaller = jaxbContext.createUnmarshaller();

            //
            // verify that response has correct schema format (unless the
            // concrete message processor doesn't require validation)
            //
            if (!skipValidation) {
                unmarshaller.setSchema(wsTrustSchema);
            }

            jaxbParserResult = unmarshaller.unmarshal(soapResponseNode, responseType);
        } catch (JAXBException e) {
            String message = "Error parsing the response document";
            log.debug(message);
            throw new ParserException(message, e);
        }

        log.debug("Parsing the responce object to JAXB classes completed");

        return jaxbParserResult.getValue();
    }

    /**
     * Loads WS-Trust schema file.
     *
     * @return WS-Trust Schema
     */
    private static Schema loadWsTrustSchema() {
        try {
            Schema wsTrustSchema = Util.loadXmlSchemaFromResource(ResponseUnmarshaller.class, WS_TRUST_1_3_SCHEMA);
            return wsTrustSchema;
        } catch (IllegalArgumentException e) {
            LoggerFactory.getLogger(SamlTokenImpl.class).error(
                    String.format("Schema resource `%s' is missing.", WS_TRUST_1_3_SCHEMA), e);
            throw new RuntimeException(String.format("Schema resource `%s' is missing.", WS_TRUST_1_3_SCHEMA), e);
        } catch (SAXException e) {
            LoggerFactory.getLogger(ResponseUnmarshaller.class).error(ERR_LOADING_WS_TRUST_SCHEMA, e);
            throw new RuntimeException(ERR_LOADING_WS_TRUST_SCHEMA, e);
        }
    }
}
