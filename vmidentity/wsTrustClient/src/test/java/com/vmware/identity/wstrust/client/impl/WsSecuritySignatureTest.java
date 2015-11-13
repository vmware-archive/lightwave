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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.util.concurrent.TimeUnit;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.crypto.MarshalException;
import javax.xml.crypto.dsig.XMLSignature;
import javax.xml.crypto.dsig.XMLSignatureException;
import javax.xml.crypto.dsig.XMLSignatureFactory;
import javax.xml.crypto.dsig.dom.DOMValidateContext;

import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import com.vmware.vim.sso.client.DefaultTokenFactory;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.HolderOfKeyConfig;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.TokenSpec.DelegationSpec;

public class WsSecuritySignatureTest {

    private static final String USER_ID = "test";
    private static final String PASSWORD = "password";
    private static final String DELEGATE = "service";
    private static final String SIGNATURE_ELEMENT_NAME = "Signature";
    private static final String SIGNATURE_VALIDATION_ERROR_MSG = "Signature validation error";
    private static final int REQUEST_VALIDITY_IN_SECONDS = 600;
    private static final String WSSE_JAXB_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0";
    private static final String WSSU_JAXB_PACKAGE = "org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_utility_1_0";
    private static final String DIGITAL_SIGNATURE_NAMESPACE = "http://www.w3.org/2000/09/xmldsig#";

    private static HolderOfKeyConfig hokConfig;
    private static RequestBuilder userRequestBuilder;
    private static RequestBuilder userRequestWithActAsBuilder;
    private static RequestBuilder renewRequestBuilder;
    private static RequestBuilder validateRequestBuilder;
    private static RequestBuilder newTokenRequestBuilder;
    private static RequestBuilder newTokenWithAstAsRequestBuilder;
    private static RequestBuilder gssRequestBuilder;
    private static RequestBuilder gssRequestBuilderWithActAs;
    private static RequestBuilder gssContinueRequestBuilder;
    private static RequestBuilder solutionRequestBuilder;
    private static RequestBuilder solutionRequestBuilderWithActAs;
    private static SamlToken signedToken;
    private WsSecuritySignature signByCert;
    private WsSecuritySignature signByAssertion;
    private WsSecuritySignature nullSigner;

    @BeforeClass
    public static void init() throws SsoKeyStoreOperationException, IOException, InvalidTokenException,
            ParserException, JAXBException {
        KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
        hokConfig = new HolderOfKeyConfig(keystore.getPrivateKey(TestTokenUtil.TEST_KEYSTORE_PRIV_KEY_PASSWORD
                .toCharArray()), keystore.getCertificate(), null);

        String samlXml = TestTokenUtil.getValidSamlTokenString();

        signedToken = DefaultTokenFactory.createToken(samlXml, keystore.getCertificate());

        final TokenSpec tokenSpec = createTokenSpec(true, true);
        final TokenSpec tokenSpecActAs = createTokenSpecActAs(true);
        JAXBContext jaxbContext = JAXBContext.newInstance(Constants.WS_1_4_TRUST_JAXB_PACKAGE + ":"
                + Constants.WS_1_3_TRUST_JAXB_PACKAGE + ":" + WSSE_JAXB_PACKAGE + ":" + WSSU_JAXB_PACKAGE);
        userRequestBuilder = new AcquireTokenByUserPassRequestBuilder(USER_ID, PASSWORD, tokenSpec, true, jaxbContext,
                REQUEST_VALIDITY_IN_SECONDS);
        userRequestWithActAsBuilder = new AcquireTokenByUserPassRequestBuilder(USER_ID, PASSWORD, tokenSpecActAs, true,
                jaxbContext, REQUEST_VALIDITY_IN_SECONDS);
        renewRequestBuilder = new RenewTokenRequestBuilder(signedToken, tokenSpec.getTokenLifetime(), jaxbContext,
                REQUEST_VALIDITY_IN_SECONDS);
        validateRequestBuilder = new ValidateTokenRequestBuilder(signedToken, jaxbContext, REQUEST_VALIDITY_IN_SECONDS);
        newTokenRequestBuilder = new AcquireTokenByTokenRequestBuilder(signedToken, tokenSpec, true, jaxbContext,
                REQUEST_VALIDITY_IN_SECONDS);
        newTokenWithAstAsRequestBuilder = new AcquireTokenByTokenRequestBuilder(signedToken, tokenSpecActAs, true,
                jaxbContext, REQUEST_VALIDITY_IN_SECONDS);
        gssRequestBuilder = new AcquireTokenByGssInitiateRequestBuilder(tokenSpec, new byte[] { 0, 1, 2, 1, 2 }, true,
                jaxbContext, REQUEST_VALIDITY_IN_SECONDS);
        gssRequestBuilderWithActAs = new AcquireTokenByGssInitiateRequestBuilder(tokenSpecActAs, new byte[] { 0, 1, 2,
                1, 2 }, true, jaxbContext, REQUEST_VALIDITY_IN_SECONDS);
        gssContinueRequestBuilder = new AcquireTokenByGssContinueRequestBuilder("dummyCtx",
                new byte[] { 0, 1, 2, 1, 2 }, jaxbContext, REQUEST_VALIDITY_IN_SECONDS);
        solutionRequestBuilder = new AcquireTokenByCertificateRequestBuilder(tokenSpec, jaxbContext,
                REQUEST_VALIDITY_IN_SECONDS);
        solutionRequestBuilderWithActAs = new AcquireTokenByCertificateRequestBuilder(tokenSpecActAs, jaxbContext,
                REQUEST_VALIDITY_IN_SECONDS);
    }

    @Before
    public void testSetup() {
        signByCert = WsSecuritySignatureFactory.createWsSecuritySignatureCertificate(hokConfig);
        signByAssertion = WsSecuritySignatureFactory.createWsSecuritySignatureAssertion(hokConfig, signedToken.getId());
        nullSigner = WsSecuritySignatureFactory.createWsEmptySecuritySignature();
    }

    @Test
    public void createUserPassSignature() throws ParserException, SignatureException {
        checkSignature(signByCert, userRequestBuilder);
        checkSignature(signByCert, userRequestWithActAsBuilder);
    }

    @Test
    public void createGSSSignature() throws ParserException, SignatureException {
        checkSignature(signByCert, gssRequestBuilder);
        checkSignature(signByCert, gssRequestBuilderWithActAs);
    }

    @Test
    public void createContinueGSSSignature() throws SignatureException, ParserException {
        checkSignature(signByCert, gssContinueRequestBuilder);
    }

    @Test
    public void createRenewSignature() throws ParserException, SignatureException {
        checkSignature(signByCert, renewRequestBuilder);
    }

    @Test
    public void createNewTokenSignature() throws ParserException, SignatureException {
        checkSignature(signByAssertion, newTokenRequestBuilder);
        checkSignature(signByAssertion, newTokenWithAstAsRequestBuilder);
    }

    @Test
    public void createSolutionSignature() throws ParserException, SignatureException {
        checkSignature(signByCert, solutionRequestBuilder);
        checkSignature(signByCert, solutionRequestBuilderWithActAs);
    }

    @Test
    public void createNullSignature() throws ParserException, SignatureException {
        checkNoNewSignature(userRequestBuilder);
        checkNoNewSignature(userRequestWithActAsBuilder);
        checkNoNewSignature(renewRequestBuilder);
        checkNoNewSignature(validateRequestBuilder);
        checkNoNewSignature(newTokenRequestBuilder);
        checkNoNewSignature(newTokenWithAstAsRequestBuilder);
        checkNoNewSignature(gssRequestBuilder);
        checkNoNewSignature(gssRequestBuilderWithActAs);
        checkNoNewSignature(gssContinueRequestBuilder);
        checkNoNewSignature(solutionRequestBuilder);
        checkNoNewSignature(solutionRequestBuilderWithActAs);

    }

    /**
     * Validates that the message is signed
     *
     * @param message
     * @throws SignatureException
     */
    private void validateSignature(SoapMessage message) throws SignatureException {

        XMLSignatureFactory fac = XMLSignatureFactory.getInstance();
        DOMValidateContext valContext = new DOMValidateContext(hokConfig.getCertificate().getPublicKey(),
                getSignatureElementList(message).item(0));

        boolean isValid = false;
        try {
            XMLSignature signature = fac.unmarshalXMLSignature(valContext);
            isValid = signature.validate(valContext);
        } catch (MarshalException e) {
            throw new SignatureException(SIGNATURE_VALIDATION_ERROR_MSG, e);
        } catch (XMLSignatureException e) {
            throw new SignatureException(SIGNATURE_VALIDATION_ERROR_MSG, e);
        }

        assertTrue(isValid);
    }

    private NodeList getSignatureElementList(SoapMessage message) {
        return message.getMessage().getSOAPPart()
                .getElementsByTagNameNS(DIGITAL_SIGNATURE_NAMESPACE, SIGNATURE_ELEMENT_NAME);
    }

    /**
     * Creates a new valid token spec
     *
     * @throws InvalidTokenException
     * @throws ParserException
     * @throws SsoKeyStoreOperationException
     */
    private static TokenSpec createTokenSpec(boolean useDelegation, boolean renewable) throws InvalidTokenException,
            ParserException, SsoKeyStoreOperationException {
        DelegationSpec ds = useDelegation ? new DelegationSpec(true, DELEGATE) : null;
        return new TokenSpec.Builder(TimeUnit.HOURS.toSeconds(1L)).delegationSpec(ds).renewable(renewable)
                .createTokenSpec();
    }

    private static TokenSpec createTokenSpecActAs(boolean renewable) throws ParserException, InvalidTokenException,
            SsoKeyStoreOperationException {
        Element tokenElement = TestTokenUtil.getAnotherValidSamlTokenElement();
        SamlToken actas = DefaultTokenFactory.createTokenFromDom(tokenElement, TestTokenUtil.loadDefaultKeystore()
                .getCertificate());
        DelegationSpec ds = new DelegationSpec(actas, true);
        return new TokenSpec.Builder(TimeUnit.HOURS.toSeconds(1L)).delegationSpec(ds).renewable(renewable)
                .createTokenSpec();
    }

    private void checkSignature(WsSecuritySignature signature, RequestBuilder requestBuilder)
            throws SignatureException, ParserException {
        validateSignature(signature.sign(requestBuilder.createRequest()));
    }

    private void checkNoNewSignature(RequestBuilder requestBuilder) throws ParserException, SignatureException {

        final SoapMessage req = requestBuilder.createRequest();
        assertEquals(getSignatureElementList(req).getLength(), getSignatureElementList(nullSigner.sign(req))
                .getLength());
    }
}
