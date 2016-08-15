/* ********************************************************************************
 * Copyright 2012 VMware, Inc. All rights reserved.
 **********************************************************************************/
package com.vmware.vmidentity.websso.client.test;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.SignatureException;
import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.lang.Validate;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.Response;
import org.opensaml.saml2.core.StatusCode;
import org.opensaml.xml.io.MarshallingException;
import org.opensaml.xml.io.UnmarshallingException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import com.vmware.identity.websso.client.AssertionConsumerService;
import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.MetadataSettingsImpl;
import com.vmware.identity.websso.client.SPConfiguration;
import com.vmware.identity.websso.client.SamlNames;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SharedUtils;
import com.vmware.identity.websso.client.SignatureAlgorithm;
import com.vmware.identity.websso.client.SingleLogoutService;
import com.vmware.identity.websso.client.SingleSignOnService;

/**
 * @author root
 *Utility class for client library unit test.
 *Signing keys: a keystore resource is build in the project. We could replace it with
 * a temp ks in tmp folder to overide the default ks in the project.
 * 
 */
public class TestUtils {
	private static Logger logger = LoggerFactory.getLogger(TestUtils.class);

	//Key information for singing. These will be used for creating simulated singedSAMLObject
	// sending to the library.

	private static KeyStore ks = null;
	private static char[] stsKeystorePassword = "ca$hc0w".toCharArray();
	private static String stsAlias = "stskey";
	private static PrivateKey signingKey = null;
	private static X509Certificate signingCertificate = null;


	/**
	 * This algorithm uri assciate to the algorithm used in "/sts-store.jks". I am not sure
	 * how to get from the resource grammatically. Hence hard code as constant here.
	 */
	private static String signingAlgorithmUri = "http://www.w3.org/2000/09/xmldsig#rsa-sha1";

	/**
	 * Initialize TestUtils class
	 * 
	 * What it does:
	 * 		init keystore and key/cert for signing and verification test.
	 * 
	 */
	public static void bootstrap() throws Exception {

		//setup key store for signing SP messages
		//The unit test
		if (signingKey == null || ks == null) {
			ks = KeyStore.getInstance(KeyStore.getDefaultType());
			ks.load(TestUtils.class.getResourceAsStream("/sts-store.jks"),
					stsKeystorePassword);

			Key key = ks.getKey(stsAlias, stsKeystorePassword);
			signingKey = (PrivateKey) key;
			signingCertificate = (X509Certificate)ks.getCertificate(stsAlias);
		}

	}

	/**
	 * @return the signingKey
	 */
	public static PrivateKey getPrivateKey() {
		return signingKey;
	}

	/**
	 * @return the signingCertificate
	 */
	public static X509Certificate getCertificate() {
		return signingCertificate;
	}

	/**
	 * @return the signingAlgorithmUri
	 */
	public static String getSigningAlgorithmUri() {
		return signingAlgorithmUri;
	}
	/**
	 * Creates a map that contain query key/value pairs.
	 */
	public static Map<String, String> getQueryMap(String query) {
		String[] params = query.split("&");
		Map<String, String> map = new HashMap<String, String>();
		for (String param : params) {
			String name = param.split("=")[0];
			String value = param.split("=")[1];
			map.put(name, value);
		}
		return map;
	}


	/**
	 * Return a parameter value from a HTTP query map.
	 */
	public static String getParameterFromQueryMap(
			Map<String, String> queryMap, String parameter)
					throws UnsupportedEncodingException {
		String retval = queryMap.get(parameter);
		if (retval != null) {
			retval = URLDecoder.decode(retval, "UTF-8");
		}
		return retval;
	}

	/**
	 * Creates and setup MetadataSettings
	 */
	static public MetadataSettings CreateMetadataSettings() {
		MetadataSettings metadataSettings = new MetadataSettingsImpl();

		//1 construct idpConfig
		List<String> idFormats = new LinkedList<String>();
		idFormats.add(SamlNames.EMAIL_ADDRESS);
		idFormats.add(SamlNames.PERSISTENT);

		//create sso service list
		List<SingleSignOnService> ssoServices = new LinkedList<SingleSignOnService>();
		SingleSignOnService ssoService0 = new SingleSignOnService(TestConfig.SsoService_loc,
				TestConfig.SsoService_binding_0);
		ssoServices.add(ssoService0);

		//create idp slo service list
		List<SingleLogoutService> idpSloServices = new LinkedList<SingleLogoutService>();
		SingleLogoutService idpSloService0 = new SingleLogoutService(TestConfig.IdpSloService_loc,
				TestConfig.IdpSloService_binding_0);
		idpSloServices.add(idpSloService0);

		//since we are not verify real signing from IDP, create a fake cert.
		IDPConfiguration idpConfiguration = new IDPConfiguration(TestConfig.tenantName,
				TestConfig.idpEntityID,
				TestUtils.getCertificate(),
				idFormats,
				ssoServices,
				idpSloServices);

		//2 consgtruct SPConfiguration

		//NameIDFormats
		List<String> spIDFormats = new LinkedList<String>();
		spIDFormats.add(SamlNames.EMAIL_ADDRESS);
		spIDFormats.add(SamlNames.PERSISTENT);

		//create Assertion consumer services
		List<AssertionConsumerService> ascoServices = new LinkedList<AssertionConsumerService>();
		AssertionConsumerService asco = new AssertionConsumerService(TestConfig.ACS0_endpoint,
				true, TestConfig.ACS0_binding, 0);
		ascoServices.add(asco);

		//create SP slo service list
		List<SingleLogoutService> spSloServices = new LinkedList<SingleLogoutService>();
		SingleLogoutService spSloServices0 = new SingleLogoutService(TestConfig.SpSloService_loc,
				TestConfig.SpSloService_binding_0);
		spSloServices.add(spSloServices0);

		SPConfiguration spConfiguration = new SPConfiguration(TestConfig.tenantName,
				TestConfig.spEntityID,
				TestConfig.authnSigned, //athu
				signingKey,  //signing key
				null,
				TestConfig.SpSign_algorithm_URI, // signingAlgorithm,
				spIDFormats,
				ascoServices,
				spSloServices);

		metadataSettings.addIDPConfiguration(idpConfiguration);
		metadataSettings.addSPConfiguration(spConfiguration);

		return metadataSettings;
	}


	/**
	 * Build a saml sso response with given token
	 * 		This function create an instance of  SamlUtils and populate call to
	 * SamlUtils::createSamlLoginResponse with test parameters.
	 * 
	 * @param inResponseTo		the request id this response is reponsing to
	 * @param token				assertion dom
	 * @return Response  object created.
	 */
	static public Response createSamlAuthnResponse(String inResponseTo, Document token)
			throws NoSuchAlgorithmException,
			UnmarshallingException {

		logger.info("createSamlAuthnResponse. ");
		SamlUtils samlUtils = new SamlUtils(null, //cert
				TestUtils.getPrivateKey(), 		//signing key used in producing the response
				TestUtils.getSigningAlgorithmUri(),//algorithm to generate assertion.
				//here use the same as the one used for Response.
				null, 						// algorithm for verifying. not used
				TestConfig.idpEntityID		//issuer url
				);

		// get parameters
		String destination = TestConfig.ACS0_endpoint;
		// create SAML request
		return samlUtils.createSamlLoginResponse(inResponseTo, destination, SamlNames.SUCCESS,
				null,
				null,	//message
				token);
	}

	/**
	 * Simulate a logout request heading towards SP
	 *
	 * @param id
	 * @param sessionIndex
	 * @return
	 * @throws NoSuchAlgorithmException
	 */
	public static LogoutRequest createSamlLogoutRequestFromIDP(String id,
			String sessionIndex) throws NoSuchAlgorithmException {
		// get parameters
		String issuerUrl = TestConfig.idpEntityID;
		String destination = TestConfig.SpSloService_loc;

		SamlUtils samlUtils = new SamlUtils( null,	//cert. not used.
				null,								//signing key not used.
				null,								//sign algorithm. not used.
				null,								//check algorithm. not used.
				issuerUrl
				);

		// create SAML request
		return samlUtils.createSamlLogoutRequest(id, destination,
				SamlNames.PERSISTENT, TestConstants.USER, sessionIndex);
	}

	/**
	 * Simulate a logout response heading towards SP
	 *
	 * @param inResponseTo		logout request id it responding to.
	 * @param sessionIndex
	 * @return
	 * @throws NoSuchAlgorithmException
	 */
	public static LogoutResponse createSamlLogoutResponseFromIDP(String inResponseTo,
			String sessionIndex) throws NoSuchAlgorithmException {
		// get parameters
		String issuerUrl = TestConfig.idpEntityID;
		String destination = TestConfig.SpSloService_loc;

		return SamlUtils
				.createSamlLogoutResponse( issuerUrl,
						inResponseTo,	//inResponseTo
						destination, //destination
						StatusCode.SUCCESS_URI,
						StatusCode.SUCCESS_URI,
						null);

	}

	/**
	 * Build a mock HttpServletRequest with given expected parameters.
	 * 
	 */
	public static HttpServletRequest buildMockHttpServletRequestObject(
			SignableSAMLObject samlObject,boolean doCompress, String relayStateParameter,
			String sigAlg, String signature, StringBuffer sbRequestUrl)
					throws MarshallingException, IOException {

		logger.info("buildMockHttpServletRequestObject. ");
		HttpServletRequest request = createMock(HttpServletRequest.class);

		if (samlObject instanceof LogoutResponse
				|| samlObject instanceof Response) {
			expect(request.getParameter(SamlUtils.SAML_RESPONSE_PARAMETER))
			.andReturn(SamlUtils.encodeSAMLObject(samlObject,doCompress))
			.anyTimes();
			expect(request.getParameter(SamlUtils.SAML_REQUEST_PARAMETER))
			.andReturn(null).anyTimes();
		} else if (samlObject instanceof LogoutRequest ) { //
			expect(request.getParameter(SamlUtils.SAML_REQUEST_PARAMETER))
			.andReturn(SamlUtils.encodeSAMLObject(samlObject, doCompress))
			.anyTimes();
			expect(request.getParameter(SamlUtils.SAML_RESPONSE_PARAMETER))
			.andReturn(null).anyTimes();
		}
		else {
			//any other obj, throw.
			throw new IllegalArgumentException();
		}


		expect(request.getParameter(SamlUtils.RELAY_STATE_PARAMETER)).andReturn(
				relayStateParameter).anyTimes();
		expect(request.getParameter(SamlUtils.SIGNATURE_ALGORITHM_PARAMETER))
		.andReturn(sigAlg).anyTimes();
		expect(request.getParameter(SamlUtils.SIGNATURE_PARAMETER)).andReturn(
				signature).anyTimes();
		expect(request.getRequestURL()).andReturn(sbRequestUrl).anyTimes();

		String queryString = null;
		if (samlObject instanceof LogoutResponse || samlObject instanceof Response) {
			queryString = SamlUtils.SAML_RESPONSE_PARAMETER + "="
				+ SamlUtils.encodeSAMLObject(samlObject, doCompress);
		} else if (samlObject instanceof LogoutRequest) {
			queryString = SamlUtils.SAML_REQUEST_PARAMETER + "="
					+ SamlUtils.encodeSAMLObject(samlObject, doCompress);
		} else {
			//any other obj, throw.
			throw new IllegalArgumentException();
		}

		if (relayStateParameter != null) {
			queryString = queryString + "&" + SamlUtils.RELAY_STATE_PARAMETER
					+ "=" + relayStateParameter;
		}
		if (sigAlg != null) {
			queryString = queryString + "&"
					+ SamlUtils.SIGNATURE_ALGORITHM_PARAMETER + "=" + sigAlg;
		}
		if (signature != null) {
			queryString = queryString + "&" + SamlUtils.SIGNATURE_PARAMETER + "="
					+ signature;
		}
		expect(request.getQueryString()).andReturn(queryString).anyTimes();

		replay(request);
		return request;
	}


	/**
	 * Return a mocked HttpServletResponse for successfully validating
	 * SSO or SLO response received. No message is expected to send out via this response.
	 */
	public static HttpServletResponse buildMockResponseObjectForResponse(
			final StringWriter sw)
					throws IOException {
		HttpServletResponse response = createMock(HttpServletResponse.class);
		response.setContentType("text/html");
		expect(response.getWriter()).andReturn(new PrintWriter(sw)).anyTimes();
		replay(response);
		return response;
	}


	/**
	 * Create Dom from assertion xml file.
	 * @param tokenResource	file name of the token in the resource folder.
	 * @throw SAXException, IOException, ParserConfigurationException
	 */
	public static Document readToken(final String tokenResource)
			throws SAXException, IOException, ParserConfigurationException {

		Validate.notNull(tokenResource);
		String resourcePath = "/"+tokenResource;
		logger.info("Creating assertion token from "+resourcePath);
		InputStream is = TestUtils.class.getResourceAsStream(
				resourcePath);
		return SharedUtils.createDOM(is);
	}

	/**
	 * @return
	 * @throws IOException
	 * @throws SAXException
	 * @throws ParserConfigurationException
	 */
	public static Document readSignedToken()
			throws ParserConfigurationException, SAXException, IOException {
		logger.info("Creating assertion token from signed-assertion.xml. ");
		InputStream is = TestUtils.class.getResourceAsStream(
				"/signed-assertion.xml");
		return SharedUtils.createDOM(is);

	}

	/**
	 * Sign the given message string and return the signature string.
	 * @param messageToSign
	 * @return the signature string, Base64 encoded and URL encoded using "UTF-8"
	 * @throws NoSuchAlgorithmException
	 * @throws InvalidKeyException
	 * @throws SignatureException
	 * @throws UnsupportedEncodingException
	 */
	public static String signMessage(String messageToSign) throws NoSuchAlgorithmException,
	InvalidKeyException, SignatureException, UnsupportedEncodingException {
		String signingAlgoUri = getSigningAlgorithmUri();
		SignatureAlgorithm algo = SignatureAlgorithm.
				getSignatureAlgorithmForURI(signingAlgoUri);
		Signature sig = Signature.getInstance(algo.getAlgorithmName());
		sig.initSign(TestUtils.getPrivateKey());
		byte[] messageBytes = messageToSign.getBytes();
		sig.update(messageBytes);
		byte[] sigBytes = sig.sign();
		return URLEncoder.encode(SharedUtils.encodeBytes(sigBytes), "UTF-8");
	}
}
