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
import javax.xml.bind.JAXBException;

import com.vmware.identity.token.impl.ValidateUtil;
import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.TokenSpec.Confirmation;

/**
 * This class represents the abstract factory for creating the
 * {@link RequestParametersValidator}, {@link RequestBuilder},
 * {@link ResponseHandler} and {@link WsSecuritySignature} according to the type
 * of credential used.
 *
 * @param <T>
 *            Type of the response from the Security Token Service
 */
abstract class RequestParserAbstractFactory<T> {
    private static final String WSSE_JAXB_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0";
    private static final String WSSU_JAXB_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0";

    protected JAXBContext jaxbContext;
    protected SecurityTokenServiceConfig stsConfig;

    public RequestParserAbstractFactory(SecurityTokenServiceConfig config) {
        assert config != null;

        ValidateUtil.validateNotNull(config, "Security Token Service configuration");

        try {
            jaxbContext = JAXBContext.newInstance(Constants.WS_1_4_TRUST_JAXB_PACKAGE + ":"
                    + Constants.WS_1_3_TRUST_JAXB_PACKAGE + ":" + WSSE_JAXB_PACKAGE + ":" + WSSU_JAXB_PACKAGE);
        } catch (JAXBException e) {
            throw new IllegalStateException("JAXBContext cannot be initialized", e);
        }

        this.stsConfig = config;
    }

    public abstract RequestParametersValidator createRequestParametersValidator();

    public abstract RequestBuilder createRequestBuilder(Credential clientCredential, TokenSpec tokenSpec);

    public abstract ResponseHandler<T> createResponseHandler();

    public abstract WsSecuritySignature createWsSecuritySignature(Credential clientCredential, TokenSpec tokenSpec);

    /**
     * Returns the proper instance of {@link WsSecuritySignature} depending on
     * the input parameters.
     *
     * @param spec
     *            the token specification
     * @return
     */
    protected WsSecuritySignature getWsDefaultSecuritySignature(TokenSpec tokenSpec) {
        return (stsConfig.getHolderOfKeyConfig() == null || tokenSpec.getConfirmation() == Confirmation.NONE) ? WsSecuritySignatureFactory
                .createWsEmptySecuritySignature() : WsSecuritySignatureFactory
                .createWsSecuritySignatureCertificate(stsConfig.getHolderOfKeyConfig());
    }
}
