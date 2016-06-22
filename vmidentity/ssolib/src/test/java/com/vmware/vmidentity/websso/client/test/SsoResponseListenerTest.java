/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.vmidentity.websso.client.test;

import static org.junit.Assert.fail;

import java.io.StringWriter;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.junit.After;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.opensaml.saml2.core.Response;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;

import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MessageStoreImpl;
import com.vmware.identity.websso.client.MessageType;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SharedUtils;
import com.vmware.identity.websso.client.endpoint.SharedComponent;
import com.vmware.identity.websso.client.endpoint.SsoResponseListener;

/**
 * SamlUtils
 *
 */
public class SsoResponseListenerTest {

    private static Logger log = LoggerFactory.getLogger(SsoResponseListenerTest.class);;

    private static SsoResponseListener controller = null;

    private String relayStateParameter;
    // some constants
    private static String ssoRequestID = "45"; // the request id of sso that the
                                               // assertion is responding to.

    /**
     * @throws java.lang.Exception
     */
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {
    }

    /**
     * setup for the test class
     *
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        // prepare data structures for sendrequest test
        SharedComponent.bootstrap();
        TestUtils.bootstrap();
        controller = new SsoResponseListener();

        MessageStore messageStore = new MessageStoreImpl();
        // define metadatasettingimpl
        MetadataSettings metadataSettings = TestUtils.CreateMetadataSettings();

        // call setters here since we are not using autowiring for the unit
        // testing.
        controller.setMessageStore(messageStore);
        controller.setLogonProcessor(new LogonProcessorImpl());
        controller.setMetadataSettings(metadataSettings);
        controller.setAssertionMustBeSigned(false);
        this.relayStateParameter = SharedUtils.encodeString(TestConstants.RELAY_STATE);
    }

    /**
     * Test expecting success sso response from mock saml response. Assertion
     * file is a non-Castle assertion sample file.
     */
    @Test
    public final void successResponseTest() {
        log.info("\nsuccessResponseTest: ");
        this.assertionTestSuccess("saml-assertion.xml",false);
    }

    /**
     * Test a signed assertion created from Castle. This is signature test case.
     *
     * Note: I have to disable this test since it contain the condition that has
     * expired. We can't remove the condition since there is signature in there.
     * To do sanity check, enable it locally and disable it before checking in.
     * This test is expected to pass signature validation but fail in condition
     * validation.
     */
    @Ignore
    @Test
    public final void signedAssertionTest() {
        log.info("\nsignedAssertionTest: ");
        this.assertionTestSuccess("signed-assertion.xml", false);
    }

    /**
     * Test a castle assertion with signature and notOnOrAfter condition
     * removed. Note: the response is signed. So this should be a valid
     * response.
     */
    @Test
    public final void castleAssertionTest() {
        log.info("\ncastleAssertionTest: ");
        this.assertionTestSuccess("castle-assertion.xml",false);
    }


    /**
     * Test a castle assertion with signature and notOnOrAfter condition
     * removed. Note: the response is signed. So this should be a valid
     * response.
     */
    @Test
    public final void idpSSOSuccessTest() {
        log.info("\ncastleAssertionTest: ");
        this.assertionTestSuccess("castle-assertion.xml",true);
    }


    /**
     * @param assertionRsourceName
     *            the file name of assertion test file in resource folder.
     */
    private void assertionTestSuccess(String assertionRsourceName,boolean idpInitiated) {
        try {

            StringBuffer sbResponseUrl = new StringBuffer();

            // Add a message entry in the message store so the response
            // validation process can match it.

            if (SsoResponseListenerTest.controller.getMessageStore().get(SsoResponseListenerTest.ssoRequestID) == null && !idpInitiated) {
                Message ssoMessage = new Message(MessageType.AUTHN_REQUEST, SsoResponseListenerTest.ssoRequestID, null, // relay
                                                                                                                        // state
                        null, TestConfig.spEntityID, TestConfig.SsoService_loc, // target
                        null, // status
                        null, // substatus
                        null, // session index
                        null, // MessageDatda
                        null, // tag
                        false); //IdpInitiated;
                SsoResponseListenerTest.controller.getMessageStore().add(ssoMessage);
            }
            Document token = TestUtils.readToken(assertionRsourceName);
            Response authnResponse = TestUtils.createSamlAuthnResponse(idpInitiated? null:SsoResponseListenerTest.ssoRequestID, token);
            sbResponseUrl.append(authnResponse.getDestination());

            String samlResponseParameter = SamlUtils.encodeSAMLObject(authnResponse, false);

            // produce signature
            String messageToSign = SamlUtils.SAML_RESPONSE_PARAMETER + "=" + samlResponseParameter + "&"
                    + SamlUtils.RELAY_STATE_PARAMETER + "=" + this.relayStateParameter + "&"
                    + SamlUtils.SIGNATURE_ALGORITHM_PARAMETER + "=" + TestUtils.getSigningAlgorithmUri();

            String signature = TestUtils.signMessage(messageToSign);

            // print out complete GET url
            SharedUtils.logUrl(log, sbResponseUrl, authnResponse, this.relayStateParameter, null, signature, null);

            // build mock request object with saml token
            HttpServletRequest request = TestUtils.buildMockHttpServletRequestObject(authnResponse, false,
                    this.relayStateParameter, TestUtils.getSigningAlgorithmUri(), signature, sbResponseUrl);

            StringWriter sw = new StringWriter();
            HttpServletResponse response = TestUtils.buildMockResponseObjectForResponse(sw);

            controller.consumeResponse("", request, response);

        } catch (Exception f) {
            fail();
        }

    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }

    /**
     * Test expecting error sso response from mock saml response.
     */
    // @Test
    public final void errorResponseTest() {
        // log.info("\nStart unit test: ");
        // TODO
    }

    /**
     * Test expecting error sso response from mock saml response.
     */
    // @Test
    public final void failAthenTest() {
        // log.info("\nStart unit test: ");
        // TODO
    }

}
