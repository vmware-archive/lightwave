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

import java.util.List;

import javax.xml.bind.JAXBContext;

import org.oasis_open.docs.ws_sx.ws_trust._200512.BinaryExchangeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.DelegateToType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ObjectFactory;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ParticipantType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ParticipantsType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewingType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.AttributedString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.PasswordString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UsernameTokenType;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3._2005._08.addressing.AttributedURIType;
import org.w3._2005._08.addressing.EndpointReferenceType;

import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AdviceSetType;
import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AdviceType;
import com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.AttributeType;
import com.vmware.vim.sso.client.Advice;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.vim.sso.client.Advice.AdviceAttribute;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.TokenSpec.Confirmation;

/**
 * Abstract implementation of {@link RequestBuilder}. Contains various common
 * methods needed by its subclasses.
 */
abstract class AcquireTokenRequestBuilder implements RequestBuilder {

    protected static final String REQUEST_TYPE_ISSUE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Issue";
    private static final String SOAP_ACTION_ISSUE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue";

    private static final String PREFFERED_SIGNATURE_ALG = "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";
    private static final String HOLDER_OF_KEY_CONFIRMATION_TYPE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/PublicKey";
    private static final String BEARER_CONFIRMATION_TYPE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Bearer";

    private static final String WST14_ACTAS = "ActAs";
    private final RequestBuilderHelper requestBuilderHelper;

    private final TokenSpec spec;
    private final Logger log = LoggerFactory.getLogger(AcquireTokenRequestBuilder.class);
    private final boolean hokConfirmation;

    /**
     * @param tokenSpec
     *            The token spec stating the requirements for the new token.
     *            Cannot be <code>null</code>
     * @param hokConfirmation
     *            Parameter indicating if the SSO client configuration contains
     *            holder-of-key information.
     */
    public AcquireTokenRequestBuilder(TokenSpec tokenSpec, boolean hokConfirmation, JAXBContext jaxbContext,
            int requestValidityInSeconds) {
        this.requestBuilderHelper = new RequestBuilderHelper(jaxbContext, requestValidityInSeconds, SOAP_ACTION_ISSUE);
        assert tokenSpec != null;

        this.spec = tokenSpec;
        this.hokConfirmation = hokConfirmation;
    }

    @Override
    public int getRequestValidityInSeconds() {
        return requestBuilderHelper.getRequestValidityInSeconds();
    }

    protected final Object createBody(ObjectFactory wstFactory) throws ParserException {
        RequestSecurityTokenType request = wstFactory.createRequestSecurityTokenType();

        request.setRequestType(REQUEST_TYPE_ISSUE);
        request.setLifetime(requestBuilderHelper.createLifetimeElement(spec.getTokenLifetime()));
        request.setTokenType(RequestBuilderHelper.TOKEN_TYPE_SAML2);
        request.setRenewing(createRenewingElement(wstFactory));
        request.setSignatureAlgorithm(PREFFERED_SIGNATURE_ALG);
        setRequestKeyType(request);
        addAudienceRestriction(request, wstFactory);
        addAdvice(request);

        if (spec.getDelegationSpec() != null) {
            request.setDelegatable(spec.getDelegationSpec().isDelegable());
            if (spec.getDelegationSpec().getDelegateTo() != null) {
                assert spec.getDelegationSpec().getActAsToken() == null;
                DelegateToType delegateTo = wstFactory.createDelegateToType();
                delegateTo.setAny(createUsernameToken(spec.getDelegationSpec().getDelegateTo(), null));
                request.setDelegateTo(delegateTo);
            }
            if (spec.getDelegationSpec().getActAsToken() != null) {
                assert spec.getDelegationSpec().getDelegateTo() == null;
                // going to attach the token after Jaxb marchalls the request
                // into DOM
                request.setActAs(new org.oasis_open.docs.ws_sx.ws_trust._200802.ObjectFactory().createActAsType());
            }
        }

        return request;
    }

    protected final void postProcessRequest(SoapMessage message) throws ParserException {

        if (spec.getDelegationSpec() != null && spec.getDelegationSpec().getActAsToken() != null) {
            requestBuilderHelper.insertSamlToken(message, Constants.WS_TRUST14_NAMESPACE, WST14_ACTAS, spec
                    .getDelegationSpec().getActAsToken());
        }
    }

    protected final SecurityHeaderType createSecurityHeader() {
        return requestBuilderHelper.createSecurityHeader();
    }

    protected final SoapMessage wrapToSoapMessage(Object request, SecurityHeaderType secHeader) throws ParserException {
        return requestBuilderHelper.wrapToSoapMessage(request, secHeader);
    }

    protected final void insertSamlToken(SoapMessage message, String elementNamespace, String elementLocalName,
            SamlToken token) throws ParserException {
        requestBuilderHelper.insertSamlToken(message, elementNamespace, elementLocalName, token);
    }

    protected final BinaryExchangeType createBinaryExchangeElement(ObjectFactory wstFactory, byte[] data) {
        return requestBuilderHelper.createBinaryExchangeElement(wstFactory, data);
    }

    /**
     * Creates a WS-Security UsernameToken element.
     *
     * @param subject
     * @param password
     * @return UsernameToken
     */
    protected final UsernameTokenType createUsernameToken(String subject, String password) {
        org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory objFactory = new org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory();

        UsernameTokenType userNameToken = objFactory.createUsernameTokenType();
        AttributedString user = objFactory.createAttributedString();
        user.setValue(subject.toString());
        userNameToken.setUsername(user);

        if (password != null) {
            // If the password is not specified (i.e. requesting a solution
            // token) do not create the password element
            PasswordString pass = objFactory.createPasswordString();
            pass.setValue(password);
            userNameToken.getAny().add(pass);
        }

        if (log.isDebugEnabled()) {
            log.debug("Username token for user: " + subject + "created");
        }

        return userNameToken;
    }

    /**
     * Creates element containing information about the token renewal ability.
     *
     * @param wstFactory
     * @return
     */
    private RenewingType createRenewingElement(ObjectFactory wstFactory) {
        RenewingType renewingType = wstFactory.createRenewingType();
        renewingType.setAllow(spec.isRenewable());
        // setOK(false) means that the token cannot be renewed after its
        // expiration. This is enforces by the profiled WS-Trust standard.
        renewingType.setOK(false);

        return renewingType;
    }

    /**
     * Sets the token confirmation type.
     *
     * @param request
     */
    private void setRequestKeyType(RequestSecurityTokenType request) {
        String keyType = (spec.getConfirmation() == Confirmation.DEFAULT && hokConfirmation) ? HOLDER_OF_KEY_CONFIRMATION_TYPE
                : BEARER_CONFIRMATION_TYPE;
        request.setKeyType(keyType);
    }

    /**
     * Adds audience restriction list to the request
     *
     * @param request
     * @param wstFactory
     */
    private void addAudienceRestriction(RequestSecurityTokenType request, ObjectFactory wstFactory) {
        if (!spec.getAudienceRestriction().isEmpty()) {
            ParticipantsType participants = wstFactory.createParticipantsType();
            List<ParticipantType> participantsList = participants.getParticipant();
            for (String participant : spec.getAudienceRestriction()) {
                org.w3._2005._08.addressing.ObjectFactory wsaFactory = new org.w3._2005._08.addressing.ObjectFactory();
                EndpointReferenceType endpoint = wsaFactory.createEndpointReferenceType();
                AttributedURIType address = wsaFactory.createAttributedURIType();
                address.setValue(participant);
                endpoint.setAddress(address);
                ParticipantType partElement = wstFactory.createParticipantType();
                partElement.setEndpointReference(endpoint);
                participantsList.add(partElement);
            }

            request.setParticipants(participants);
        }
    }

    /**
     * Adds advice list to the request
     *
     * @param request
     */
    private void addAdvice(RequestSecurityTokenType request) {
        if (!spec.getAdvice().isEmpty()) {
            com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.ObjectFactory adviceFactory = new com.rsa.names._2009._12.std_ext.ws_trust1_4.advice.ObjectFactory();
            AdviceSetType adviceSet = adviceFactory.createAdviceSetType();
            for (Advice advice : spec.getAdvice()) {
                AdviceType wsAdvice = adviceFactory.createAdviceType();
                wsAdvice.setAdviceSource(advice.getSource());
                List<AttributeType> attributeList = wsAdvice.getAttribute();

                for (AdviceAttribute attribute : advice.getAttributes()) {
                    AttributeType attr = adviceFactory.createAttributeType();
                    attr.setName(attribute.getName());

                    for (String attributeValue : attribute.getValue()) {
                        attr.getAttributeValue().add(attributeValue);
                    }

                    attributeList.add(attr);
                }

                adviceSet.getAdvice().add(wsAdvice);
            }

            request.setAdviceSet(adviceSet);
        }
    }

    protected String createRedactedDescription() {
        return getClass().getSimpleName() + " [tokenSpec=" + spec + ", hokConfirmation=" + hokConfirmation + "]";
    }
}
