/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.vmidentity.websso.client.test;

import static org.easymock.EasyMock.capture;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.io.StringWriter;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.easymock.Capture;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.websso.client.Error;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MessageStoreImpl;
import com.vmware.identity.websso.client.MessageType;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SharedUtils;
import com.vmware.identity.websso.client.endpoint.SharedComponent;
import com.vmware.identity.websso.client.endpoint.SloListener;

/**
 * @author root
 *
 */
public class SloListenerTest {

    private static Logger log = LoggerFactory.getLogger(SloListenerTest.class);;

    private static SloListener controller = null;

    private String relayStateParameter;

    // some constants
    private static String ssoRequestID = "45";
    private final String sessionIndex = "1234567";

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        // prepare data structures for sendrequest test
        SharedComponent.bootstrap();
        TestUtils.bootstrap();
        controller = new SloListener();

        MessageStore messageStore = new MessageStoreImpl();
        // define metadatasettingimpl
        MetadataSettings metadataSettings = TestUtils.CreateMetadataSettings();

        // call setters here since we are not using autowiring for the unit
        // testing.
        controller.setMessageStore(messageStore);
        controller.setLogoutProcessor(new LogoutProcessorImpl());
        controller.setMetadataSettings(metadataSettings);

        this.relayStateParameter = SharedUtils.encodeString(TestConstants.RELAY_STATE);
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }

    /**
     * Test receiving a valid logout request - The slo request from IDP is same whether in SP or IDP-initiated SLO workflow.
     * Approach of testing: Construct a
     * valid httpservletRequest that carry a legal logout request. Then setup a
     * mock HttpServeletResponse object specifying expected message. Pass the
     * reqeust to the receiving interface. Any exceptions or unexpected messages
     * sent to the mock response would trigger this test to fail. About testing
     * requested created signing: signed nameIDFormat: persistant
     */
    @Test
    public final void logoutRequestSuccess() {
        log.info("\nlogoutRequestSuccessSigned : ");
        try {
            StringBuffer sbResponseUrl = new StringBuffer();

            // 1 Construct simulated LogoutRequest to be received.
            LogoutRequest logoutRequest = TestUtils.createSamlLogoutRequestFromIDP(SloListenerTest.ssoRequestID,
                    this.sessionIndex);
            // 2. construct request url
            sbResponseUrl.append(logoutRequest.getDestination());

            String requestParameter = SamlUtils.encodeSAMLObject(logoutRequest, true);

            // 3 produce signature
            String messageToSign = SamlUtils.SAML_REQUEST_PARAMETER + "=" + requestParameter + "&"
                    + SamlUtils.SIGNATURE_ALGORITHM_PARAMETER + "=" + TestUtils.getSigningAlgorithmUri();
            String signature = TestUtils.signMessage(messageToSign);

            // print out complete GET url
            SharedUtils.logUrl(log, sbResponseUrl, logoutRequest, this.relayStateParameter, null, signature, null);

            // 4 build mock HttpServletRequest object
            HttpServletRequest request = TestUtils.buildMockHttpServletRequestObject(logoutRequest, true, null,
                    TestUtils.getSigningAlgorithmUri(), signature, sbResponseUrl);

            // 5. build mock HttpServletResponse.
            Capture<String> capturedUrl = new Capture<String>();
            HttpServletResponse response = buildMockResponseForSuccess(capturedUrl);

            // 6. send the mock request to the API.
            controller.slo(TestConfig.tenantName, request, response);

        } catch (Exception f) {
            fail();
        }
    }

    /**
     * Test disallow an unsigned logout request. Approach of testing:
     * Construct a valid httpservletRequest that carry a legal logout request.
     * Then setup a mock HttpServeletResponse object specifying expected
     * message. Pass the reqeust to the receiving interface. Any exceptions or
     * unexpected messages sent to the mock response would trigger this test to
     * fail. About testing requested created signing: signed nameIDFormat:
     * persistant
     */
    @Test
    public final void logoutRequestFailUnsigned() {
        log.info("\nlogoutRequestSuccessUnsigned: ");
        try {
            StringBuffer sbResponseUrl = new StringBuffer();

            // 1 Construct simulated LogoutRequest to be received.
            LogoutRequest logoutRequest = TestUtils.createSamlLogoutRequestFromIDP(SloListenerTest.ssoRequestID,
                    this.sessionIndex);
            // 2. construct request url
            sbResponseUrl.append(logoutRequest.getDestination());

            // print out complete GET url
            SharedUtils.logUrl(log, sbResponseUrl, logoutRequest, this.relayStateParameter, null, null, null);

            // 4 build mock HttpServletRequest object
            HttpServletRequest request = TestUtils.buildMockHttpServletRequestObject(logoutRequest, true, null, null,
                    null, sbResponseUrl);

            // 5. build mock HttpServletResponse.
            HttpServletResponse response = buildMockResponseForError(HttpServletResponse.SC_BAD_REQUEST,
                    Error.SIGNATURE);

            // 6. send the mock request to the API.
            controller.slo(TestConfig.tenantName, request, response);

        } catch (Exception f) {
            fail();
        }

    }

    /**
     * Test receiving a invalid logout request with a bogus issuer. Approach of
     * testing: signing: unsigned nameIDFormat: persistant
     */
    @Test
    public final void logoutRequestFailIssuer() {
        log.info("\nlogoutRequestSuccessUnsigned: ");
        try {
            StringBuffer sbResponseUrl = new StringBuffer();

            // 1 Construct simulated LogoutRequest to be received.
            LogoutRequest logoutRequest = TestUtils.createSamlLogoutRequestFromIDP(SloListenerTest.ssoRequestID,
                    this.sessionIndex);

            Issuer fakeIssuer = SamlUtils.spawnIssuer("fake issuer");
            logoutRequest.setIssuer(fakeIssuer);
            // 2. construct request url
            sbResponseUrl.append(logoutRequest.getDestination());

            // print out complete GET url
            SharedUtils.logUrl(log, sbResponseUrl, logoutRequest, this.relayStateParameter, null, null, null);

            // 4 build mock HttpServletRequest object
            HttpServletRequest request = TestUtils.buildMockHttpServletRequestObject(logoutRequest, true, null, null,
                    null, sbResponseUrl);

            // 5. build mock HttpServletResponse for error.
            HttpServletResponse response = buildMockResponseForError(HttpServletResponse.SC_BAD_REQUEST, Error.ISSUER);

            // 6. send the mock request to the API.
            controller.slo(TestConfig.tenantName, request, response);

        } catch (Exception f) {
            fail();
        }
    }

    /**
     * Fail case: No matching slo request been sent from this SP. Note: We have
     * to leave this before any success test on slo response recieved, or have
     * to remove message entries with the same ssoRequestID.
     */
    @Test
    public final void logoutResponseFailNoSLORequest() {

        log.info("\nlogoutResponseSuccessSigned : ");
        try {
            StringBuffer sbResponseUrl = new StringBuffer();

            // 2 Construct simulated LogoutRequest to be received.
            LogoutResponse logoutResponse = TestUtils.createSamlLogoutResponseFromIDP(SloListenerTest.ssoRequestID,
                    this.sessionIndex);
            // 3. construct request url
            sbResponseUrl.append(logoutResponse.getDestination());

            // print out complete GET url
            SharedUtils.logUrl(log, sbResponseUrl, logoutResponse, this.relayStateParameter, null, null, null);

            // 4 build mock HttpServletRequest object that coming as
            // logoutresponse
            HttpServletRequest request = TestUtils.buildMockHttpServletRequestObject(logoutResponse, true, null, null,
                    null, sbResponseUrl);

            HttpServletResponse response = buildMockResponseForError(HttpServletResponse.SC_BAD_REQUEST,
                    Error.IN_RESPONSE_TO);

            // 6. send the mock request to the API.
            controller.slo(TestConfig.tenantName, request, response);

        } catch (Exception f) {
            fail();
        }
    }

    /**
     * Test receiving a valid logout response signed. Approach of testing:
     * Construct a valid httpservletRequest that carry a legal logout response.
     * Then setup a mock HttpServeletResponse object specifying expected
     * message. Pass the response to the receiving interface. Any exceptions or
     * unexpected messages sent to the mock response would trigger this test to
     * fail. About testing requested created signing: signed nameIDFormat:
     * persistant
     */
    @Test
    public final void logoutResponseSuccessSigned() {
        log.info("\nlogoutResponseSuccessSigned : ");
        try {
            StringBuffer sbResponseUrl = new StringBuffer();

            // 1. Add a slo request message entry in the message store
            // so the slo response validation process can match it.

            Message ssoMessage = new Message(MessageType.LOGOUT_REQUEST, SloListenerTest.ssoRequestID, null, // relay
                                                                                                              // state
                    null, TestConfig.spEntityID, TestConfig.IdpSloService_loc, // target
                    null, // status
                    null, // substatus
                    this.sessionIndex, // session index
                    null, // MessageDatda
                    null, // tag
                    false); //IdpInitiated;
            SloListenerTest.controller.getMessageStore().add(ssoMessage);

            // 2. Construct simulated LogoutRequest to be received.
            LogoutResponse logoutResponse = TestUtils.createSamlLogoutResponseFromIDP(SloListenerTest.ssoRequestID,
                    this.sessionIndex);
            // 3. construct request url
            sbResponseUrl.append(logoutResponse.getDestination());

            // 4. produce signature
            String messageToSign = SamlUtils.SAML_RESPONSE_PARAMETER + "="
                    + SamlUtils.encodeSAMLObject(logoutResponse, true) + "&" + SamlUtils.SIGNATURE_ALGORITHM_PARAMETER
                    + "=" + TestUtils.getSigningAlgorithmUri();

            String signature = TestUtils.signMessage(messageToSign);
            // print out complete GET url
            SharedUtils.logUrl(log, sbResponseUrl, logoutResponse, this.relayStateParameter, null, null, null);

            // 5. build mock HttpServletRequest object that coming as
            // logoutresponse
            HttpServletRequest request = TestUtils.buildMockHttpServletRequestObject(logoutResponse, true, null,
                    TestUtils.getSigningAlgorithmUri(), signature, sbResponseUrl);

            StringWriter sw = new StringWriter();
            HttpServletResponse response = TestUtils.buildMockResponseObjectForResponse(sw);

            // 6. send the mock request to the API.
            controller.slo(TestConfig.tenantName, request, response);

        } catch (Exception f) {
            fail();
        }
    }

    /**
     * Test receiving a valid logout response unsigned. Approach of testing:
     * Construct a valid httpservletRequest that carry a legal logout response.
     * Then setup a mock HttpServeletResponse object specifying expected
     * message. Pass the response to the receiving interface. Any exceptions or
     * unexpected messages sent to the mock response would trigger this test to
     * fail. About testing requested created signing: unsigned nameIDFormat:
     * persistant
     */
    @Test
    public final void logoutResponseSuccessUnsigned() {
        log.info("\nlogoutResponseSuccessUnSigned : ");
        try {
            StringBuffer sbResponseUrl = new StringBuffer();

            // 1 Add a slo request message entry in the message store
            // so the slo response validation process can match it.

            Message ssoMessage = new Message(MessageType.LOGOUT_REQUEST, SloListenerTest.ssoRequestID, null, // relay
                                                                                                             // state
                    null, TestConfig.spEntityID, TestConfig.IdpSloService_loc, // target
                    null, // status
                    null, // substatus
                    this.sessionIndex, // session index
                    null, // MessageDatda
                    null, // tag
                    false); //IdpInitiated;
            SloListenerTest.controller.getMessageStore().add(ssoMessage);

            // 2 Construct simulated LogoutRequest to be received.
            LogoutResponse logoutResponse = TestUtils.createSamlLogoutResponseFromIDP(SloListenerTest.ssoRequestID,
                    this.sessionIndex);
            // 3. construct request url
            sbResponseUrl.append(logoutResponse.getDestination());

            // print out complete GET url
            SharedUtils.logUrl(log, sbResponseUrl, logoutResponse, this.relayStateParameter, null, null, null);

            // 4 build mock HttpServletRequest object that coming as
            // logoutresponse
            HttpServletRequest request = TestUtils.buildMockHttpServletRequestObject(logoutResponse, true, null, null,
                    null, sbResponseUrl);

            StringWriter sw = new StringWriter();
            HttpServletResponse response = TestUtils.buildMockResponseObjectForResponse(sw);

            // 6. send the mock request to the API.
            controller.slo(TestConfig.tenantName, request, response);

        } catch (Exception f) {
            fail();
        }
    }


    private HttpServletResponse buildMockResponseForSuccess(Capture<String> capturedStr) throws IOException {
        HttpServletResponse httpResponse = createMock(HttpServletResponse.class);
        httpResponse.addHeader(SharedUtils.CACHE_CONTROL_HEADER, "no-store");
        httpResponse.addHeader(SharedUtils.PRAGMA, "no-cache");
        httpResponse.sendRedirect(capture(capturedStr));

        replay(httpResponse);
        return httpResponse;
    }

    /**
     * Return a mocked HttpServletResponse for error cases in which it is
     * expected to send error response.
     *
     * @param errorCode
     *            error code in HttpServletResponse.sendError call.
     * @param errorMessage
     *            error message in HttpServletResponse.sendError call.
     *
     */
    private HttpServletResponse buildMockResponseForError(int errorCode, String errorMessage) throws IOException {
        HttpServletResponse response = createMock(HttpServletResponse.class);

        response.sendError(errorCode, errorMessage);
        replay(response);
        return response;
    }

}
