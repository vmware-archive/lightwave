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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.KeyStore;
import java.security.cert.X509Certificate;
import java.util.Locale;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import javax.xml.namespace.QName;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPFactory;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.vmware.vim.sso.client.BundleMessageSource;
import com.vmware.vim.sso.client.BundleMessageSource.Key;
import com.vmware.vim.sso.client.ConfirmationType;
import com.vmware.vim.sso.client.DefaultTokenFactory;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.vim.sso.client.exception.SsoException;
import com.vmware.vim.sso.client.exception.SsoRuntimeException;
import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;
import com.vmware.identity.wstrust.client.AccountLockedException;
import com.vmware.identity.wstrust.client.AsyncHandler;
import com.vmware.identity.wstrust.client.AuthenticationFailedException;
import com.vmware.identity.wstrust.client.CertificateValidationException;
import com.vmware.identity.wstrust.client.Credential;
import com.vmware.identity.wstrust.client.GSSCredential;
import com.vmware.identity.wstrust.client.MalformedResponseException;
import com.vmware.identity.wstrust.client.NegotiationHandler;
import com.vmware.identity.wstrust.client.PasswordExpiredException;
import com.vmware.identity.wstrust.client.SecurityTokenService;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.ConnectionConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.HolderOfKeyConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.SSLTrustedManagerConfig;
import com.vmware.identity.wstrust.client.ServerCommunicationException;
import com.vmware.identity.wstrust.client.ServerSecurityException;
import com.vmware.identity.wstrust.client.SsoRequestException;
import com.vmware.identity.wstrust.client.TimeSynchronizationException;
import com.vmware.identity.wstrust.client.TokenCredential;
import com.vmware.identity.wstrust.client.TokenRequestRejectedException;
import com.vmware.identity.wstrust.client.TokenSpec;
import com.vmware.identity.wstrust.client.UsernamePasswordCredential;

public class SecurityTokenServiceImplTest {

    private static final String TEST_USER_ID = "testUser";
    private static final String TEST_PASSWORD = "testPass";
    private static final String AUTHENTICATION_FAULT = "ns0:FailedAuthentication";
    private static final String EXPIRED_PASSWORD_FAULT = "expired";
    private static final String LOCKED_ACCOUNT_FAULT = "locked";
    private static final String BAD_REQUEST_FAULT = "BadRequest";
    private static final int TASK_TIMEOUT_SEC = 20; // 20 seconds
    private static final String WS_TRUST_VALIDATE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Validate";
    private static final String WS_TRUST_ISSUE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/Issue";

    private static SecurityTokenServiceConfig bearerStsConfig;
    private static SoapBinding validBinding;
    private static SecurityTokenServiceImpl validSTS;
    private static KeyStoreData keystore;
    private static SoapBinding bindingMock;

    private IMocksControl mockControl = EasyMock.createControl();
    private RequestParserFactoryProvider provider;
    private RequestParserAbstractFactory<Element> requestCredentialParserAbstractFactory;
    private RequestParserAbstractFactory<Boolean> validateCredentialParserAbstractFactory;
    private ResponseHandler<Element> tokenResponseHandler;
    private ResponseHandler<Boolean> validateResponseHandler;
    private RequestBuilder requestBuilder;

    private SecurityTokenServiceImpl tokenService;

    @BeforeClass
    public static void setupSuite() throws SsoKeyStoreOperationException, InvalidTokenException, ParserException,
            MalformedURLException, CertificateValidationException {
        keystore = TestTokenUtil.loadDefaultKeystore();

        ConnectionConfig connConfig = new ConnectionConfig(new URL("http://sts.vmware.com"), null);
        bearerStsConfig = new SecurityTokenServiceConfig(connConfig,
                new X509Certificate[] { keystore.getCertificate() }, null, null);
        validBinding = new SoapBindingImpl(connConfig.getSSLTrustedManagerConfig());
        validSTS = new SecurityTokenServiceImpl(validBinding, bearerStsConfig);

        bindingMock = new SoapBindingMock();
    }

    @Before
    public void setUp() {
        provider = mockControl.createMock(RequestParserFactoryProvider.class);
        requestCredentialParserAbstractFactory = mockControl.createMock(RequestParserAbstractFactory.class);
        validateCredentialParserAbstractFactory = mockControl.createMock(RequestParserAbstractFactory.class);

        tokenResponseHandler = mockControl.createMock(ResponseHandler.class);
        validateResponseHandler = mockControl.createMock(ResponseHandler.class);
        requestBuilder = mockControl.createMock(RequestBuilder.class);

        tokenService = new SecurityTokenServiceImpl(bindingMock, bearerStsConfig);
    }

    //
    // Positive test cases
    //

    @Test
    public void acquireTokenByUsernamePasswordTest() throws IOException, SOAPException, SsoException {

        acquireTokenByUsernamePassInit();

        tokenService.setParserProvider(provider);
        mockControl.replay();

        Credential userPassCred = new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD);
        SamlToken result = tokenService.acquireToken(userPassCred, createTokenSpec());
        assertNotNull(result);

        mockControl.verify();
    }

    @Test
    public void validateValidTokenTest() throws SsoException, IOException, SOAPException {

        validateTokenInit(true, false);

        tokenService.setParserProvider(provider);
        mockControl.replay();

        assertTrue(tokenService.validateToken(DefaultTokenFactory.createToken(TestTokenUtil.getValidSamlTokenString(),
                bearerStsConfig.getTrustedRootCertificates())));

        mockControl.verify();
    }

    @Test
    public void validateInvalidTokenTest() throws SsoException, IOException, SOAPException {

        validateTokenInit(false, false);
        tokenService.setParserProvider(provider);
        mockControl.replay();

        assertFalse(tokenService.validateToken(DefaultTokenFactory.createToken(TestTokenUtil.getValidSamlTokenString(),
                bearerStsConfig.getTrustedRootCertificates())));

        mockControl.verify();
    }

    @Test
    public void acquireTokenByTokenTest() throws SsoException {
        String samlXml = TestTokenUtil.getValidSamlTokenString();

        KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
        SamlToken token = DefaultTokenFactory.createToken(samlXml, keystore.getCertificate());
        Credential tokenCredential = new TokenCredential(token, true);

        KeyStoreData keyStore = TestTokenUtil.loadDefaultKeystore();
        HolderOfKeyConfig hokConfig = new HolderOfKeyConfig(
                keyStore.getPrivateKey(TestTokenUtil.TEST_KEYSTORE_PRIV_KEY_PASSWORD.toCharArray()),
                keystore.getCertificate(), null);

        SecurityTokenServiceConfig hokStsConfig = new SecurityTokenServiceConfig(bearerStsConfig.getConnectionConfig(),
                bearerStsConfig.getTrustedRootCertificates(), null, hokConfig);

        SoapBinding bindingMock = new SoapBindingMock() {
            @Override
            public SoapMessage sendMessage(SoapMessage message, URL serviceURL) throws ServerCommunicationException,
                    SoapFaultException {
                String requestHeaderString;
                try {
                    requestHeaderString = TestUtil.serializeToString(message.getHeader());
                } catch (ParserException e) {
                    fail("Failed to serialize header.");
                    throw new RuntimeException(e);
                }
                // very basic verification that the header is signed with an
                // assertion signature
                assertTrue(requestHeaderString.contains("#SAMLID"));
                return super.sendMessage(message, serviceURL);
            }
        };
        tokenService = new SecurityTokenServiceImpl(bindingMock, hokStsConfig);

        acquireTokenByTokenInit(token, false, hokConfig, null);
        tokenService.setParserProvider(provider);
        mockControl.replay();

        SamlToken result = tokenService.acquireToken(tokenCredential, createTokenSpec());
        assertNotNull(result);

        mockControl.verify();
    }

    @Test
    public void acquireTokenAsyncTest() throws InterruptedException, ExecutionException {

        try {
            acquireTokenByUsernamePassInit();
        } catch (ParserException | InvalidTokenException e) {
            e.printStackTrace();
        }
        tokenService.setParserProvider(provider);
        mockControl.replay();

        TestAsyncHandler<SamlToken> asyncHandler = new TestAsyncHandler<SamlToken>(false);
        Credential userPassCred = new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD);
        Future<SamlToken> future = tokenService.acquireTokenAsync(userPassCred, createTokenSpec(), asyncHandler);

        assertNotNull(future);
        verifyFutureCompleted(future);
        asyncHandler.verifyCorrectExecution();

        mockControl.verify();
    }

    @Test
    public void acquireTokenAsyncNoHandlerTest() throws InterruptedException, ExecutionException {

        try {
            acquireTokenByUsernamePassInit();
        } catch (ParserException | InvalidTokenException e) {
            e.printStackTrace();
        }
        tokenService.setParserProvider(provider);
        mockControl.replay();

        Credential userPassCred = new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD);
        Future<SamlToken> future = tokenService.acquireTokenAsync(userPassCred, createTokenSpec());

        assertNotNull(future);
        verifyFutureCompleted(future);

        mockControl.verify();
    }

    @Test
    public void validateTokenAsyncTest() throws InvalidTokenException, InterruptedException, ExecutionException,
            ParserException {

        validateTokenInit(true, false);
        tokenService.setParserProvider(provider);
        mockControl.replay();

        TestAsyncHandler<Boolean> asyncHandler = new TestAsyncHandler<Boolean>(false);
        Future<Boolean> future = tokenService.validateTokenAsync(
                DefaultTokenFactory.createToken(TestTokenUtil.getValidSamlTokenString(),
                        bearerStsConfig.getTrustedRootCertificates()), asyncHandler);
        assertNotNull(future);
        verifyFutureCompleted(future);
        asyncHandler.verifyCorrectExecution();

        mockControl.verify();
    }

    @Test
    public void validateTokenAsyncNoHandlerTest() throws InvalidTokenException, InterruptedException,
            ExecutionException, ParserException {

        validateTokenInit(true, false);
        tokenService.setParserProvider(provider);
        mockControl.replay();

        Future<Boolean> future = tokenService.validateTokenAsync(DefaultTokenFactory.createToken(
                TestTokenUtil.getValidSamlTokenString(), bearerStsConfig.getTrustedRootCertificates()));
        assertNotNull(future);
        verifyFutureCompleted(future);

        mockControl.verify();
    }

    @Test
    public void acquireTokenByGssOneStepTest() throws SsoException {

        EasyMock.expect(provider.getMultiStepAcquireCredentialParser(GSSCredential.class)).andReturn(
                new AcquireRequestGSSParserFactoryTest(bearerStsConfig, true, false));
        tokenService.setParserProvider(provider);
        mockControl.replay();

        Credential cred = new GSSCredential(new GssNegotiationHandlerDummy());
        SamlToken token = tokenService.acquireToken(cred, createTokenSpec());
        assertNotNull(token);

        mockControl.verify();
    }

    @Test
    public void acquireTokenByGssOneStepAsyncTest() throws InterruptedException, ExecutionException {

        EasyMock.expect(provider.getMultiStepAcquireCredentialParser(GSSCredential.class)).andReturn(
                new AcquireRequestGSSParserFactoryTest(bearerStsConfig, true, false));
        tokenService.setParserProvider(provider);
        mockControl.replay();

        TestAsyncHandler<SamlToken> asyncHandler = new TestAsyncHandler<SamlToken>(false);
        Credential cred = new GSSCredential(new GssNegotiationHandlerDummy());
        Future<SamlToken> future = tokenService.acquireTokenAsync(cred, createTokenSpec(), asyncHandler);

        assertNotNull(future);
        verifyFutureCompleted(future);
        asyncHandler.verifyCorrectExecution();

        mockControl.verify();
    }

    @Test
    public void acquireTokenByGssMultiStepTest() throws SsoException {
        EasyMock.expect(provider.getMultiStepAcquireCredentialParser(GSSCredential.class)).andReturn(
                new AcquireRequestGSSParserFactoryTest(bearerStsConfig, false, false));
        tokenService.setParserProvider(provider);
        mockControl.replay();

        Credential cred = new GSSCredential(new GssNegotiationHandlerDummy());
        SamlToken token = tokenService.acquireToken(cred, createTokenSpec());
        assertNotNull(token);

        mockControl.verify();

    }

    @Test
    public void acquireTokenByGssMultiStepAsyncTest() throws InterruptedException, ExecutionException {

        EasyMock.expect(provider.getMultiStepAcquireCredentialParser(GSSCredential.class)).andReturn(
                new AcquireRequestGSSParserFactoryTest(bearerStsConfig, false, false));
        tokenService.setParserProvider(provider);
        mockControl.replay();

        Credential cred = new GSSCredential(new GssNegotiationHandlerDummy());

        TestAsyncHandler<SamlToken> asyncHandler = new TestAsyncHandler<SamlToken>(false);
        Future<SamlToken> future = tokenService.acquireTokenAsync(cred, createTokenSpec(), asyncHandler);

        assertNotNull(future);
        verifyFutureCompleted(future);
        asyncHandler.verifyCorrectExecution();
    }

    //
    // Negative test cases
    //

    @Test
    public void checkConstructorForNullArgs() throws SsoKeyStoreOperationException {
        assertConstructorThrowsException(null, bearerStsConfig);
        assertConstructorThrowsException(validBinding, null);
        assertConstructorThrowsException(null, null);
    }

    @Test
    public void checkAcquireTokenForNullArgsTest() throws SsoException {
        assertAcquireTokenThrowsException(null, TEST_PASSWORD, createTokenSpec());
        assertAcquireTokenThrowsException(TEST_USER_ID, null, createTokenSpec());
        assertAcquireTokenThrowsException(TEST_USER_ID, TEST_PASSWORD, null);

    }

    @Test(expected = IllegalArgumentException.class)
    public void checkAquireTokenForNullCredential() throws SsoException {
        validSTS.acquireToken(null, createTokenSpec());
    }

    @Test(expected = IllegalArgumentException.class)
    public void checkValidateTokenForNullArgsTest() throws ParserException, SsoException {
        validSTS.validateToken(null);
    }

    @Test(expected = MalformedResponseException.class)
    public void validateTokenParseExceptionTest() throws SsoException, IOException {

        validateTokenInit(true, true);
        tokenService.setParserProvider(provider);
        mockControl.replay();

        tokenService.validateToken(DefaultTokenFactory.createToken(TestTokenUtil.getValidSamlTokenString(),
                bearerStsConfig.getTrustedRootCertificates()));

        mockControl.verify();
    }

    @Test(expected = ExecutionException.class)
    public void validateTokenAsyncExceptionTest() throws InvalidTokenException, InterruptedException,
            ExecutionException, ParserException {

        validateTokenInit(true, true);
        tokenService.setParserProvider(provider);
        mockControl.replay();

        TestAsyncHandler<Boolean> asyncHandler = new TestAsyncHandler<Boolean>(true);
        Future<Boolean> future = tokenService.validateTokenAsync(
                DefaultTokenFactory.createToken(TestTokenUtil.getValidSamlTokenString(),
                        bearerStsConfig.getTrustedRootCertificates()), asyncHandler);
        assertNotNull(future);
        verifyFutureCompleted(future);
        asyncHandler.verifyCorrectExecution();
    }

    @Test(expected = ExecutionException.class)
    public void acquireTokenAsyncExceptionTest() throws InterruptedException, ExecutionException, ParserException,
            InvalidTokenException {

        acquireTokenByUsernamePassInit(new ParserException("Parse exception"));

        tokenService.setParserProvider(provider);
        mockControl.replay();

        TestAsyncHandler<SamlToken> asyncHandler = new TestAsyncHandler<SamlToken>(true);
        Future<SamlToken> future = tokenService.acquireTokenAsync(new UsernamePasswordCredential(TEST_USER_ID,
                TEST_PASSWORD), createTokenSpec(), asyncHandler);

        assertNotNull(future);
        verifyFutureCompleted(future);
        asyncHandler.verifyCorrectExecution();

        mockControl.verify();
    }

    @Test(expected = MalformedResponseException.class)
    public void acquireTokenByGssExceptionTest() throws SsoException {

        EasyMock.expect(provider.getMultiStepAcquireCredentialParser(GSSCredential.class)).andReturn(
                new AcquireRequestGSSParserFactoryTest(bearerStsConfig, true, true));
        tokenService.setParserProvider(provider);
        mockControl.replay();

        tokenService.acquireToken(new GSSCredential(new GssNegotiationHandlerDummy()), createTokenSpec());

        mockControl.replay();
    }

    @Test(expected = IllegalArgumentException.class)
    public void acquireTokenByGssNullLegTest() throws SsoException {

        NegotiationHandler negHandler = new NegotiationHandler() {

            @Override
            public byte[] negotiate(byte[] leg) {
                return null;
            }
        };
        Credential cred = new GSSCredential(negHandler);

        tokenService.acquireToken(cred, createTokenSpec());
    }

    @Test(expected = IllegalArgumentException.class)
    public void acquireTokenByGssNullHandlerTest() throws SsoException {
        Credential cred = new GSSCredential(null);
        tokenService.acquireToken(cred, createTokenSpec());
    }

    @Test(expected = ExecutionException.class)
    public void acquireTokenByGssAsyncExceptionTest() throws InterruptedException, ExecutionException {

        EasyMock.expect(provider.getMultiStepAcquireCredentialParser(GSSCredential.class)).andReturn(
                new AcquireRequestGSSParserFactoryTest(bearerStsConfig, true, true));
        tokenService.setParserProvider(provider);
        mockControl.replay();

        TestAsyncHandler<SamlToken> asyncHandler = new TestAsyncHandler<SamlToken>(true);
        Future<SamlToken> future = tokenService.acquireTokenAsync(new GSSCredential(new GssNegotiationHandlerDummy()),
                createTokenSpec(), asyncHandler);

        assertNotNull(future);
        verifyFutureCompleted(future);
        asyncHandler.verifyCorrectExecution();

        mockControl.verify();
    }

    @Test(expected = SsoRequestException.class)
    public void badRequestSoapFaultTest() throws SsoException {
        SecurityTokenService tokenService = new SecurityTokenServiceImpl(new SoapBindingMock(BAD_REQUEST_FAULT),
                bearerStsConfig);
        tokenService.acquireToken(new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD), createTokenSpec());
    }

    @Test(expected = AuthenticationFailedException.class)
    public void authSoapFaultTest() throws SsoException {
        authSoapFault(AUTHENTICATION_FAULT, "");
    }

    @Test(expected = PasswordExpiredException.class)
    public void authSoapFaultPasswordExpiredTest() throws SsoException {
        authSoapFault(AUTHENTICATION_FAULT, EXPIRED_PASSWORD_FAULT);
    }

    @Test(expected = AccountLockedException.class)
    public void authSoapFaultLockedAccountTest() throws SsoException {
        authSoapFault(AUTHENTICATION_FAULT, LOCKED_ACCOUNT_FAULT);
    }

    @Test(expected = SsoRequestException.class)
    public void otherSoapFaultTest() throws SsoException {
        SecurityTokenService tokenService = new SecurityTokenServiceImpl(new SoapBindingMock("Other fault"),
                bearerStsConfig);
        tokenService.acquireToken(new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD), createTokenSpec());
    }

    @Test(expected = TimeSynchronizationException.class)
    public void timeSynchronizationDetectionTest() throws SsoException {
        SecurityTokenService tokenService = new SecurityTokenServiceImpl(new SoapBindingMock("MessageExpired"),
                bearerStsConfig);
        tokenService.acquireToken(new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD), createTokenSpec());
    }

    @Test(expected = TimeSynchronizationException.class)
    public void requestExpiredTest() throws SsoException, MalformedURLException {
        KeyStore ks = null;
        ConnectionConfig connConfig = new ConnectionConfig(new URL("http://sts.vmware.com"),
                new SSLTrustedManagerConfig(ks));
        SecurityTokenServiceConfig stsConfig = new SecurityTokenServiceConfig(connConfig,
                new X509Certificate[] { keystore.getCertificate() }, null, null) {
            @Override
            public int getRequestValidityInSeconds() {
                return 1;
            }
        };
        SoapBindingMock transport = new SoapBindingMock("ExpiredData");
        transport.setDelay(5);

        SecurityTokenService tokenService = new SecurityTokenServiceImpl(transport, stsConfig);
        tokenService.acquireToken(new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD), createTokenSpec());
    }

    @Test(expected = ServerSecurityException.class)
    public void genericWSSecurityErrorTest() throws SsoException, MalformedURLException {
        SecurityTokenService tokenService = new SecurityTokenServiceImpl(new SoapBindingMock("InvalidSecurity"),
                bearerStsConfig);
        tokenService.acquireToken(new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD), createTokenSpec());
    }

    @Test
    public void localizationTest() throws Exception {
        SecurityTokenService tokenService = new SecurityTokenServiceImpl(new SoapBindingMock("InvalidSecurity"),
                bearerStsConfig);

        try {
            tokenService.acquireToken(new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD), createTokenSpec());
            fail("Should have thrown.");
        } catch (ServerSecurityException e) {
            BundleMessageSource messageSource = new BundleMessageSource(Locale.getDefault());
            String localMessage = messageSource.get(Key.POTENITAL_TAMPERING_OF_REQUEST);
            assertFalse(e.getMessage().startsWith(localMessage));
            assertTrue(messageSource.createMessage(e).startsWith(localMessage.replace("%s", "")));
        }
    }

    private void authSoapFault(String faultToBeReturned, String faultMessage) throws InvalidTokenException,
            TokenRequestRejectedException, CertificateValidationException, ServerCommunicationException,
            ParserException, SoapFaultException {

        SoapBinding soapBindingMock = mockControl.createMock(SoapBinding.class);

        if (faultToBeReturned != null) {
            try {
                SoapFaultException exception = new SoapFaultException("SOAP fault occured", new SoapFault(SOAPFactory
                        .newInstance().createFault(faultMessage, new QName(faultToBeReturned))));
                EasyMock.expect(soapBindingMock.sendMessage(EasyMock.isA(SoapMessage.class), EasyMock.isA(URL.class)))
                        .andThrow(exception);

            } catch (SOAPException e) {
                throw new RuntimeException("Error creating SOAPFault object");
            }
        } else {
            EasyMock.expect(soapBindingMock.sendMessage(EasyMock.isA(SoapMessage.class), EasyMock.isA(URL.class)))
                    .andReturn(EasyMock.isA(SoapMessage.class));
        }
        mockControl.replay();

        SecurityTokenService tokenService = new SecurityTokenServiceImpl(soapBindingMock,
        // new SoapBindingMock(faultToBeReturned),
                bearerStsConfig);
        tokenService.acquireToken(new UsernamePasswordCredential(TEST_USER_ID, TEST_PASSWORD), createTokenSpec());

        mockControl.verify();
    }

    /**
     * Creates a new valid token spec
     *
     * @return
     */
    private TokenSpec createTokenSpec() {
        return new TokenSpec.Builder(TimeUnit.HOURS.toSeconds(1L)).createTokenSpec();
    }

    /**
     * Asserts that an exception is thrown if one of the arguments is null
     *
     * @param binding
     * @param config
     * @param mpf
     */
    private void assertConstructorThrowsException(SoapBinding binding, SecurityTokenServiceConfig config) {
        boolean exceptionThrown = false;
        try {
            new SecurityTokenServiceImpl(binding, config);
        } catch (IllegalArgumentException e) {
            exceptionThrown = true;
        } finally {
            assertTrue(exceptionThrown);
        }
    }

    /**
     * Asserts that an exception is thrown if one of the arguments is null
     *
     * @param subject
     * @param password
     * @param spec
     * @throws SsoException
     * @throws SsoRuntimeException
     */
    private void assertAcquireTokenThrowsException(String subject, String password, TokenSpec spec) throws SsoException {
        boolean exceptionThrown = false;
        try {
            Credential cred = new UsernamePasswordCredential(subject, password);
            validSTS.acquireToken(cred, spec);
        } catch (IllegalArgumentException e) {
            exceptionThrown = true;
        } finally {
            assertTrue(exceptionThrown);
        }
    }

    /**
     * Verifies that the future completes successfully its execution and the
     * results are as expected
     *
     * @param future
     * @throws InterruptedException
     * @throws ExecutionException
     * @throws TimeoutException
     */
    private void verifyFutureCompleted(Future<? extends Object> future) throws InterruptedException, ExecutionException {
        try {
            assertNotNull(future.get(TASK_TIMEOUT_SEC, TimeUnit.SECONDS));
        } catch (TimeoutException e) {
            throw new ExecutionException("Timeout", e);
        }
        assertFalse(future.isCancelled());
        assertTrue(future.isDone());
    }

    private void acquireTokenByUsernamePassInit() throws ParserException, InvalidTokenException {
        acquireTokenByUsernamePassInit(null);
    }

    private void acquireTokenByUsernamePassInit(Throwable throwable) throws ParserException, InvalidTokenException {
        EasyMock.expect(provider.getSingleStepAcquireCredentialParser(UsernamePasswordCredential.class)).andReturn(
                requestCredentialParserAbstractFactory);

        EasyMock.expect(requestCredentialParserAbstractFactory.createRequestParametersValidator()).andReturn(
                new DefaultRequestParametersValidator());
        EasyMock.expect(
                requestCredentialParserAbstractFactory.createRequestBuilder(EasyMock.isA(Credential.class),
                        EasyMock.isA(TokenSpec.class))).andReturn(requestBuilder);

        EasyMock.expect(requestCredentialParserAbstractFactory.createResponseHandler()).andReturn(tokenResponseHandler);
        EasyMock.expect(
                requestCredentialParserAbstractFactory.createWsSecuritySignature(EasyMock.isA(Credential.class),
                        EasyMock.isA(TokenSpec.class))).andReturn(
                WsSecuritySignatureFactory.createWsEmptySecuritySignature());

        EasyMock.expect(tokenResponseHandler.parseResponse(EasyMock.isA(Node.class))).andReturn(
                TestTokenUtil.getValidSamlTokenElement());

        if (throwable != null)
            EasyMock.expect(requestBuilder.createRequest()).andThrow(throwable);
        else
            EasyMock.expect(requestBuilder.createRequest()).andReturn(TestUtil.createSoapMessage(WS_TRUST_ISSUE));

    }

    private void acquireTokenByTokenInit(SamlToken token, boolean isExternal, HolderOfKeyConfig hokConfig,
            Throwable throwable) throws ParserException, InvalidTokenException {
        EasyMock.expect(provider.getSingleStepAcquireCredentialParser(TokenCredential.class)).andReturn(
                requestCredentialParserAbstractFactory);

        EasyMock.expect(requestCredentialParserAbstractFactory.createRequestParametersValidator()).andReturn(
                new DefaultRequestParametersValidator());
        EasyMock.expect(
                requestCredentialParserAbstractFactory.createRequestBuilder(EasyMock.isA(Credential.class),
                        EasyMock.isA(TokenSpec.class))).andReturn(requestBuilder);

        EasyMock.expect(requestCredentialParserAbstractFactory.createResponseHandler()).andReturn(tokenResponseHandler);
        WsSecuritySignature signatureProvider = null;
        if (!isExternal) {
            signatureProvider = WsSecuritySignatureFactory.createWsSecuritySignatureAssertion(hokConfig, token.getId());
        } else {
            if (token.getConfirmationType() == ConfirmationType.HOLDER_OF_KEY) {
                signatureProvider = WsSecuritySignatureFactory.createWsSecuritySignatureAssertion(hokConfig,
                        token.getId());
            } else {
                signatureProvider = WsSecuritySignatureFactory.createWsEmptySecuritySignature();
            }
        }

        EasyMock.expect(
                requestCredentialParserAbstractFactory.createWsSecuritySignature(EasyMock.isA(Credential.class),
                        EasyMock.isA(TokenSpec.class))).andReturn(signatureProvider);

        EasyMock.expect(tokenResponseHandler.parseResponse(EasyMock.isA(Node.class))).andReturn(
                TestTokenUtil.getValidSamlTokenElement());

        if (throwable != null)
            EasyMock.expect(requestBuilder.createRequest()).andThrow(throwable);
        else
            EasyMock.expect(requestBuilder.createRequest()).andReturn(TestUtil.createSoapMessage(WS_TRUST_ISSUE));

    }

    private void validateTokenInit(boolean returnValue, boolean throwException) throws ParserException,
            InvalidTokenException {
        EasyMock.expect(provider.getValidateCredentialParser(TokenCredential.class)).andReturn(
                validateCredentialParserAbstractFactory);

        EasyMock.expect(validateCredentialParserAbstractFactory.createRequestParametersValidator()).andReturn(
                new ValidateTokenParametersValidator());
        EasyMock.expect(
                validateCredentialParserAbstractFactory.createRequestBuilder(EasyMock.isA(Credential.class),
                        EasyMock.isNull(TokenSpec.class))).andReturn(requestBuilder);

        EasyMock.expect(validateCredentialParserAbstractFactory.createResponseHandler()).andReturn(
                validateResponseHandler);
        EasyMock.expect(
                validateCredentialParserAbstractFactory.createWsSecuritySignature(EasyMock.isA(Credential.class),
                        EasyMock.isNull(TokenSpec.class))).andReturn(
                WsSecuritySignatureFactory.createWsEmptySecuritySignature());

        if (throwException) {
            EasyMock.expect(validateResponseHandler.parseResponse(EasyMock.isA(Node.class))).andThrow(
                    new ParserException("Parse exception"));
        } else {
            EasyMock.expect(validateResponseHandler.parseResponse(EasyMock.isA(Node.class))).andReturn(returnValue);
        }

        EasyMock.expect(requestBuilder.createRequest()).andReturn(TestUtil.createSoapMessage(WS_TRUST_VALIDATE));
    }

    private static class TestAsyncHandler<T> implements AsyncHandler<T> {

        private final boolean _failOnResponse;
        private boolean _correctHandlerExecuted = false;
        private boolean _failTest = false;

        public TestAsyncHandler(boolean failOnResponse) {
            _failOnResponse = failOnResponse;
        }

        @Override
        public void handleException(Exception exception) {
            if (_failOnResponse) {
                _correctHandlerExecuted = true;
            } else {
                _failTest = true;
            }
        }

        @Override
        public void handleResponse(T response) {
            if (_failOnResponse) {
                _failTest = true;
            } else {
                _correctHandlerExecuted = true;
            }
        }

        public void verifyCorrectExecution() {
            assertFalse(_failTest);
            assertTrue(_correctHandlerExecuted);
        }
    }
}
