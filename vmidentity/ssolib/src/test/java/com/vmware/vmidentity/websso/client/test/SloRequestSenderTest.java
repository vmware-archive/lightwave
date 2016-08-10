/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.vmidentity.websso.client.test;

import static org.easymock.EasyMock.createMock;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Map;

import javax.servlet.http.HttpServletResponse;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.util.Assert;

import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MessageStoreImpl;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SamlNames;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SingleLogoutService;
import com.vmware.identity.websso.client.SloRequestSettings;
import com.vmware.identity.websso.client.endpoint.SharedComponent;
import com.vmware.identity.websso.client.endpoint.SloRequestSender;

/**
 * @author root
 * 
 */
public class SloRequestSenderTest {

    private static Logger log = LoggerFactory.getLogger(SsoSenderTest.class);;

    private static String idpAlias = TestConfig.tenantName;

    private static String spAlias = TestConfig.tenantName;

    private static Boolean isSigned = false; // defaults to false

    private static String nameIDFormat = SamlNames.PERSISTENT; // defaults to
                                                               // persistant

    private static SloRequestSender requestSender, requestSenderNoIDPSLO;

    private static String sessionIndex = null;

    private static String subject = "j.doe@company.com";

    private static String relayState = "dummy relay state ";

    /**
     * Initializing for test
     */
    @Before
    public void setUp() throws Exception {
        // prepare data structures for sendrequest test
        SharedComponent.bootstrap();
        TestUtils.bootstrap();
        requestSender = new SloRequestSender();

        MessageStore messageStore = new MessageStoreImpl();
        MetadataSettings metadataSettings = TestUtils.CreateMetadataSettings();

        // call setters here since we are not using autowiring for the unit
        // testing.
        requestSender.setMessageStore(messageStore);
        requestSender.setLogoutProcessor(new LogoutProcessorImpl());
        requestSender.setMetadataSettings(metadataSettings);

        requestSenderNoIDPSLO = new SloRequestSender();

        MetadataSettings metadataSettingsNoIDPSLO = TestUtils.CreateMetadataSettings();
        // set IDP SLO service to an empty list.
        for (IDPConfiguration idp : metadataSettingsNoIDPSLO.getAllIDPConfigurations()) {
            idp.setSingleLogoutServices(new ArrayList<SingleLogoutService>());
        }

        // call setters here since we are not using autowiring for the unit
        // testing.
        requestSenderNoIDPSLO.setMessageStore(messageStore);
        requestSenderNoIDPSLO.setLogoutProcessor(new LogoutProcessorImpl());
        requestSenderNoIDPSLO.setMetadataSettings(metadataSettingsNoIDPSLO);
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
    }

    /**
     * Test null request.
     */
    @Test
    public final void testNullRequestSettings() {
        log.info("\nStart unit test: testNullRequestSettings");
        try {
            requestSender.getRequestUrl(null);
            fail();
        } catch (IllegalArgumentException e) {
            // succeeded.
        } catch (Exception f) {
            fail();
        }
    }

    /**
     * Test unsigned request-url creation.
     * 
     * @throws UnsupportedEncodingException
     * @throws MalformedURLException
     */
    @Test
    public final void testGetRequestUrl_NotSigned() throws UnsupportedEncodingException, MalformedURLException {
        log.info("\nStart unit test: testSendRequest");
        SloRequestSettings requestSettings = new SloRequestSettings(spAlias, idpAlias, false, subject, nameIDFormat,
                sessionIndex, relayState);

        // build request URL and check non-null
        String reqUrl = requestSender.getRequestUrl(requestSettings);
        Assert.notNull(reqUrl);

        // check URL components to make sure they are present as expected.
        URL url = new URL(reqUrl);
        Map<String, String> queryMap = TestUtils.getQueryMap(url.getQuery());

        Assert.notNull(TestUtils.getParameterFromQueryMap(queryMap, SamlUtils.SAML_REQUEST_PARAMETER));
        Assert.isNull(TestUtils.getParameterFromQueryMap(queryMap, SamlUtils.SIGNATURE_ALGORITHM_PARAMETER));
        Assert.isNull(TestUtils.getParameterFromQueryMap(queryMap, SamlUtils.SIGNATURE_PARAMETER));

    }

    /**
     * Test unsigned request-url creation, IDP does not support SLO.
     *
     * @throws UnsupportedEncodingException
     * @throws MalformedURLException
     */
    @Test
    public final void testGetRequestUrlNoIDPSLO_NotSigned() throws UnsupportedEncodingException, MalformedURLException {
        log.info("\nStart unit test: testSendRequest");
        SloRequestSettings requestSettings = new SloRequestSettings(spAlias, idpAlias, false, subject, nameIDFormat,
                sessionIndex, relayState);

        // build request URL and check is null
        String reqUrl = requestSenderNoIDPSLO.getRequestUrl(requestSettings);
        Assert.isNull(reqUrl);
    }

    /**
     * Test sending signed request-url creation.
     * 
     * @throws MalformedURLException
     * @throws UnsupportedEncodingException
     */
    @Test
    public final void testGetRequestUrl_Signed() throws MalformedURLException, UnsupportedEncodingException {
        log.info("\nStart unit test: testSendRequest");
        SloRequestSettings requestSettings = new SloRequestSettings(spAlias, idpAlias, true, subject, nameIDFormat,
                sessionIndex, relayState);
        // build request URL and check non-null
        String reqUrl = requestSender.getRequestUrl(requestSettings);
        Assert.notNull(reqUrl);

        // check URL components to make sure they are present as expected.
        URL url = new URL(reqUrl);
        Map<String, String> queryMap = TestUtils.getQueryMap(url.getQuery());

        Assert.notNull(TestUtils.getParameterFromQueryMap(queryMap, SamlUtils.SAML_REQUEST_PARAMETER));
        Assert.notNull(TestUtils.getParameterFromQueryMap(queryMap, SamlUtils.SIGNATURE_ALGORITHM_PARAMETER));
        Assert.notNull(TestUtils.getParameterFromQueryMap(queryMap, SamlUtils.SIGNATURE_PARAMETER));

    }

    /**
     * Test sending signed request-url creation, IDP does not support SLO.
     *
     * @throws MalformedURLException
     * @throws UnsupportedEncodingException
     */
    @Test
    public final void testGetRequestUrlNoIDPSLO_Signed() throws MalformedURLException, UnsupportedEncodingException {
        log.info("\nStart unit test: testSendRequest");
        SloRequestSettings requestSettings = new SloRequestSettings(spAlias, idpAlias, true, subject, nameIDFormat,
                sessionIndex, relayState);
        // build request URL and check is null
        String reqUrl = requestSenderNoIDPSLO.getRequestUrl(requestSettings);
        Assert.isNull(reqUrl);
    }

    /**
     * Test sending normal request.
     * 
     * @throws IOException
     */
    @Test
    public final void testSendRequest() throws IOException {
        log.info("\nStart unit test: testSendRequest");
        SloRequestSettings requestSettings = new SloRequestSettings(spAlias, idpAlias, isSigned, subject, nameIDFormat,
                sessionIndex, relayState);
        // create redirect response servlet.
        HttpServletResponse response = createMock(HttpServletResponse.class);
        requestSender.sendRequest(requestSettings, response);

    }

    /**
     * @return the subject
     */
    public static String getSubject() {
        return subject;
    }

    /**
     * @param subject
     *            the subject to set
     */
    public static void setSubject(String subject) {
        SloRequestSenderTest.subject = subject;
    }

}
