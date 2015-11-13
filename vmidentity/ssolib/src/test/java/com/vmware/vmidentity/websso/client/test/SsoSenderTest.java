/**
 *
 */
package com.vmware.vmidentity.websso.client.test;

import static org.easymock.EasyMock.createMock;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletResponse;

import org.junit.BeforeClass;
import org.junit.Test;
import org.opensaml.saml2.core.AuthnContext;
import org.opensaml.xml.ConfigurationException;
import org.opensaml.xml.io.MarshallingException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.util.Assert;

import com.vmware.identity.websso.client.MessageStore;
import com.vmware.identity.websso.client.MessageStoreImpl;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SamlNames;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SsoRequestSettings;
import com.vmware.identity.websso.client.WebssoClientException;
import com.vmware.identity.websso.client.endpoint.SharedComponent;
import com.vmware.identity.websso.client.endpoint.SsoRequestSender;

public class SsoSenderTest {

    private static Logger log = LoggerFactory.getLogger(SsoSenderTest.class);;

    private static String idpAlias = TestConfig.tenantName;

    private static String spAlias = TestConfig.tenantName;

    private static Boolean isSigned = false; // defaults to false

    private static String nameIDFormat = SamlNames.PERSISTENT; // defaults to
                                                               // persistant

    private static Boolean allowProxy = false; // defaults to true

    private static Boolean forceAuthn = false; // defaults to false

    private static Boolean isPassive = false; // defaults to false

    private final Integer assertionConsumerServiceIndex = 0;

    private final String assertionConsumerServiceUrl = TestConfig.ACS0_endpoint;

    private final String relayState = "dummy relateSate";

    private static SsoRequestSender requestSender = null;

    /**
     * Initializing for test
     */
    @BeforeClass
    public static void setUp() throws Exception {
        // prepare data structures for sendrequest test
        SharedComponent.bootstrap();
        TestUtils.bootstrap();
        requestSender = new SsoRequestSender();

        MessageStore messageStore = new MessageStoreImpl();
        MetadataSettings metadataSettings = TestUtils.CreateMetadataSettings();

        // call setters here since we are not using autowiring for the unit
        // testing.
        requestSender.setMessageStore(messageStore);
        requestSender.setLogonProcessor(new LogonProcessorImpl());
        requestSender.setMetadataSettings(metadataSettings);
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
            // succeeded
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
        log.info("\nStart unit test: testGetRequestUrl_NotSigned");
        SsoRequestSettings requestSettings = new SsoRequestSettings(spAlias, idpAlias, false, nameIDFormat, allowProxy,
                forceAuthn, isPassive, this.assertionConsumerServiceIndex, this.assertionConsumerServiceUrl,
                this.relayState);

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
     * Test sending signed request-url creation.
     *
     * @throws UnsupportedEncodingException
     * @throws MalformedURLException
     */
    @Test
    public final void testGetRequestUrl_Signed() throws UnsupportedEncodingException, MalformedURLException {
        log.info("\nStart unit test: testGetRequestUrl_Signed");
        SsoRequestSettings requestSettings = new SsoRequestSettings(spAlias, idpAlias, true, nameIDFormat, allowProxy,
                forceAuthn, isPassive, this.assertionConsumerServiceIndex, this.assertionConsumerServiceUrl,
                this.relayState);

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
     * Test sending normal request.
     *
     * @throws WebssoClientException
     * @throws MarshallingException
     * @throws ConfigurationException
     * @throws IOException
     * @throws NoSuchAlgorithmException
     * @throws InvalidKeyException
     */
    @Test
    public final void testSendRequest() throws InvalidKeyException, NoSuchAlgorithmException, IOException,
            ConfigurationException, MarshallingException, WebssoClientException {

        log.info("\nStart unit test: testSendRequest");
        SsoRequestSettings requestSettings = new SsoRequestSettings(spAlias, idpAlias, isSigned, nameIDFormat,
                allowProxy, forceAuthn, isPassive, this.assertionConsumerServiceIndex,
                this.assertionConsumerServiceUrl, this.relayState);
        List<String> reqAuthnTypes = new ArrayList<String>();
        reqAuthnTypes.add(SamlNames.INTEGRATED_WINDOWS);
        reqAuthnTypes.add(SamlNames.PASSWORD_PROTECTED_TRANSPORT);
        reqAuthnTypes.add(AuthnContext.TLS_CLIENT_AUTHN_CTX);
        // create redirect response servlet.
        requestSettings.setRequestedAuthnContextClasses(reqAuthnTypes);
        HttpServletResponse response = createMock(HttpServletResponse.class);
        requestSender.sendRequest(requestSettings, response);

    }
    /**
     * Test sending request with no requested authentication context.
     *
     * @throws WebssoClientException
     * @throws MarshallingException
     * @throws ConfigurationException
     * @throws IOException
     * @throws NoSuchAlgorithmException
     * @throws InvalidKeyException
     */
    @Test
    public final void testSendRequest_NullRAC() throws InvalidKeyException, NoSuchAlgorithmException, IOException,
            ConfigurationException, MarshallingException, WebssoClientException {

        log.info("\nStart unit test: testSendRequest");
        SsoRequestSettings requestSettings = new SsoRequestSettings(spAlias, idpAlias, isSigned, nameIDFormat,
                allowProxy, forceAuthn, isPassive, this.assertionConsumerServiceIndex,
                this.assertionConsumerServiceUrl, this.relayState);
        // create redirect response servlet.
        HttpServletResponse response = createMock(HttpServletResponse.class);
        requestSender.sendRequest(requestSettings, response);

    }

}
