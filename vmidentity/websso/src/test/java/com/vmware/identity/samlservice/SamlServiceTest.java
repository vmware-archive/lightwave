/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */
package com.vmware.identity.samlservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Signature;
import java.security.SignatureException;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.servlet.http.HttpServletRequest;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.lang.StringEscapeUtils;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.opensaml.common.SAMLVersion;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Response;
import org.opensaml.xml.io.UnmarshallingException;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;

import com.vmware.identity.SharedUtils;
import com.vmware.identity.TestConstants;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.saml.SignatureAlgorithm;

@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class SamlServiceTest {

    private final String RESPONSE_MESSAGE = "Request contains unsupported elements";
    private final String VSPHERE_LOCAL_TENANT = "vsphere.local";

    private static SamlService service;
    private static PrivateKey privateKey;
    private static X509Certificate x509Certificate;
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(SamlServiceTest.class);

    private static String acsUrl;

    @BeforeClass
    public static void setUp() throws Exception {
        SharedUtils.bootstrap(false); // use real data
        String tenantName = ServerConfig.getTenant(0);
        String rpName = ServerConfig.getRelyingParty(tenantName, 0);
        String issuerUrl = ServerConfig.getRelyingPartyUrl(rpName);
        String acsName = ServerConfig.getAssertionConsumerService(rpName, 0);
        acsUrl = ServerConfig.getServiceEndpoint(acsName);

        KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
        InputStream is = new FileInputStream(SamlServiceTest.class.getResource(
                "/sts-store.jks").getFile());
        char[] stsKeystorePassword = "ca$hc0w".toCharArray();
        ks.load(is, stsKeystorePassword);

        String stsAlias = "stskey";
        Certificate certificate = ks.getCertificate(stsAlias);
        Key key = ks.getKey(stsAlias, stsKeystorePassword);

        List<X509Certificate> certificates = new ArrayList<X509Certificate>();
        certificates.add((X509Certificate) certificate);

        CertificateFactory certFactory = CertificateFactory
                .getInstance("X.509");
        CertPath certPath = certFactory.generateCertPath(certificates);

        privateKey = (PrivateKey) key;
        x509Certificate = (X509Certificate) certificate;

        SamlServiceFactory factory = new DefaultSamlServiceFactory();
        service = factory.createSamlService(privateKey,
                SignatureAlgorithm.RSA_SHA256, SignatureAlgorithm.RSA_SHA256,
                issuerUrl, certPath);
    }

    @AfterClass
    public static void cleanUp() throws Exception {
        SharedUtils.cleanupTenant();
    }

    @Test
    public void testVerifySignature() throws NoSuchAlgorithmException,
            InvalidKeyException, SignatureException {
        // pick a sample message
        String message = "This is a sample message to be encoded";

        // sign using our algorithm
        SignatureAlgorithm algo = SignatureAlgorithm
                .getSignatureAlgorithmForURI(TestConstants.SIGNATURE_ALGORITHM);
        Signature sig = Signature.getInstance(algo.getAlgorithmName());
        sig.initSign(privateKey);

        byte[] messageBytes = message.getBytes();
        sig.update(messageBytes);

        byte[] sigBytes = sig.sign();
        String signature = Shared.encodeBytes(sigBytes);

        // verify signature here
        sig.initVerify(x509Certificate.getPublicKey());
        sig.update(messageBytes);
        boolean verifies = sig.verify(sigBytes);
        log.debug("signature verifies in test: " + verifies);

        // just call verifySignature method and expect to not throw
        service.verifySignature(message, signature);
    }

    @Test
    public void testVerifySignatureVcd() throws Exception {
        // pick a sample VCD message
        String message = "SAMLResponse=fZJNb9swDIb%2FiqF7LPnbEeIUw4oCAVoMqN" +
                "MedhlomU49yJJrSml%2F%2FuykX8uhFwGkXpKvHmpz9Tro4IgT9dZULAoF" +
                "C9Ao2%2FbmULGH%2Fc2qZFfbDcGg41He2oP17h5ptIYwmEsNyfNdxfxkpA" +
                "XqSRoYkKRTsv5xdyvjUMhxss4qq1lwjeR6A%2B407sm5kSTnkQijtAgzES" +
                "apTNOEK219y%2B104IpGvkzg9exI49kBB90D8aNqWbC7rtifrmkhVaop26h" +
                "oijJLEOajXBdtl2KSZ7PMvNve24pBmkG%2BLiBumr%2BJyNZJLqKuzRC7J" +
                "4RmZrAj8rgz5MC4isUiilciW8XRPo5klktRhlGZ%2FmbB4zu5%2BZXsjZM" +
                "8FU9f8XxPB4hwWoiw7UJkBqJ7419XAGMeNqg1Hj2Gx%2BEFJgyVHWQhSsF" +
                "fsCGyfOkS87r%2BtaDa8K8OPvZWO3CeLsKftsXgEbTH793RSS1rrxQSMX7" +
                "R5m5OwgG39%2Fjs590GdNZ1Xr%2BZuRRepD%2Fj%2F3%2FX9h8%3D&SigA" +
                "lg=http%3A%2F%2Fwww.w3.org%2F2001%2F04%2Fxmldsig-more%23rs" +
                "a-sha256";

        // sign using our algorithm
        SignatureAlgorithm algo = SignatureAlgorithm
                .getSignatureAlgorithmForURI(TestConstants.SIGNATURE_ALGORITHM);
        Signature sig = Signature.getInstance(algo.getAlgorithmName());
        sig.initSign(privateKey);

        byte[] messageBytes = message.getBytes();
        sig.update(messageBytes);

        byte[] sigBytes = sig.sign();
        String signature = Shared.encodeBytes(sigBytes);

        // verify signature here
        sig.initVerify(x509Certificate.getPublicKey());
        sig.update(messageBytes);
        boolean verifies = sig.verify(sigBytes);
        log.debug("signature verifies in test: " + verifies);

        // just call verifySignature method and expect to not throw
        service.verifySignature(message, signature);
        /* disabled now: task 1301740
        // import our csp settings
        CasIdmClient idmClient = new CasIdmClient(SharedUtils.getIdmHostName());

        SharedUtils.importConfiguration(idmClient, VSPHERE_LOCAL_TENANT,
                "/csp.xml");

        CasIdmAccessor idmAccessor = new CasIdmAccessor(idmClient);
        log.debug("CSP settings imported successfully");
        idmAccessor.setTenant(VSPHERE_LOCAL_TENANT);

        // create new SamlService
        SamlServiceFactory factory2 = new DefaultSamlServiceFactory();
        CertificateFactory certFactory = CertificateFactory
                .getInstance("X.509");
        CertPath certPath = certFactory.generateCertPath(idmAccessor
                .getSAMLAuthorityChain());
        SamlService service2 = factory2.createSamlService(
                idmClient.getTenantPrivateKey(VSPHERE_LOCAL_TENANT),
                SignatureAlgorithm.RSA_SHA256, SignatureAlgorithm.RSA_SHA256,
                idmClient.getEntityID(VSPHERE_LOCAL_TENANT), certPath);

        // now call it again with generated signature
        String vcdSignature = "YkgxdGRqY3FiVlQvUWRLTWRHUjF1V2dJeGJZa0pHNTJJ" +
                "NGd0RUsyUEtZTDAzcloyNWJ3dmxuLzg3TlNMN1JsSVhYc2NOSkxTaVZ4Mm" +
                "c4TjNxWTBTLzg2Z0dvYjZVdVU5elY2cEZtQnJ2N0ZFZFdndFJwVDlvZE5w" +
                "VVpaa3BxQ1ROZVU4STRQYTltMVVOTDB1TUp5ckJvaVBnY3dUbk5LTko4S0" +
                "dxMWNLMlVuWTZBZGlodW5XaXdTZW5CVDVVRjZ6MHFHWmZ2d25kM2dkTWl4" +
                "eHY2WWovVElXWUg5REZYN2FJN3R0a3RTaSs5dUhTbUViMTFWRElNcGhpbm" +
                "1rdldGT3VWWHIxWFR5RUNKYnpLNXhYR3ArZXZ1UGk2TzR1UDlEVjlVdjlU" +
                "V01uVVNPYkw1aExEUDFadC9Vbzl0S1MySWIwcUp0OGIzVzV2UzVDWVdlUU" +
                "JGRTBnPT0%3D";
        vcdSignature = URLDecoder.decode(vcdSignature, "UTF-8");
        vcdSignature = Shared.decodeString(vcdSignature);
        // just call verifySignature method and expect to not throw

        service2.verifySignature(message, vcdSignature);*/
    }

    @Test
    public void testVerifySignatureVcd2() throws Exception {
        // pick a sample VCD message
        String message = "SAMLResponse=fZJNb%2BIwEIb%2FSuR7%2FJWQJhYBrRatVK" +
                "lcCsuhl5XjTCAosbMZm%2B7P3wClHxx6nPE7M%2B884%2FnyX99FJxixdb" +
                "YkgnISgTWubu2%2BJL%2B3v%2BKcLBdz1H0nB%2FXk9i74Z8DBWYRoKrWo" +
                "rm8lCaNVTmOLyuoeUHmjNj%2FWT0pSrobReWdcR6IVoG%2Bt9pdxB%2B8H" +
                "VIyJQlKR5VTIggqu8jRNmOlcqNm5OdtMZjq4Dme6azWyk6lJ9LgqyZ%2Bq" +
                "SUzKCw41AM%2BNEEZC1dQJT7K0qJJmktmb460riZamqhueFW1xNKI6pqbh" +
                "TXtIjqbJk6Sa5IgBHi16bX1JJBcy5lkssy3nSsyUEFTM8hcS7W7QpgXJGy" +
                "J1KR4%2Fk%2FkejEaE8QyDLG4whrE%2FxLbtq4DxqY%2FFwywWklOwe3rq" +
                "X%2FUI1LhePaRpyl6hQnTs3E2yNXhda6%2BZwWHOPtt5v9%2FGax%2FwLv" +
                "zpaoh2ugvwvVW8qNUmGAOIhN21WU9JvYfFM%2FwN040jvOqa0L2ZuRfepT" +
                "%2Fir79s8R8%3D&SigAlg=http%3A%2F%2Fwww.w3.org%2F2001%2F04%" +
                "2Fxmldsig-more%23rsa-sha256";

        // sign using our algorithm
        SignatureAlgorithm algo = SignatureAlgorithm
                .getSignatureAlgorithmForURI(TestConstants.SIGNATURE_ALGORITHM);
        Signature sig = Signature.getInstance(algo.getAlgorithmName());
        sig.initSign(privateKey);

        byte[] messageBytes = message.getBytes();
        sig.update(messageBytes);

        byte[] sigBytes = sig.sign();
        String signature = Shared.encodeBytes(sigBytes);

        // verify signature here
        sig.initVerify(x509Certificate.getPublicKey());
        sig.update(messageBytes);
        boolean verifies = sig.verify(sigBytes);
        log.debug("signature verifies in test: " + verifies);

        // just call verifySignature method and expect to not throw
        service.verifySignature(message, signature);
        /* disabled now: task 1301740
        // import our csp settings
        CasIdmClient idmClient = new CasIdmClient(SharedUtils.getIdmHostName());

        SharedUtils.importConfiguration(idmClient, VSPHERE_LOCAL_TENANT,
                "/csp2.xml");

        CasIdmAccessor idmAccessor = new CasIdmAccessor(idmClient);
        log.debug("CSP settings imported successfully");
        idmAccessor.setTenant(VSPHERE_LOCAL_TENANT);

        // create new SamlService
        SamlServiceFactory factory2 = new DefaultSamlServiceFactory();
        CertificateFactory certFactory = CertificateFactory
                .getInstance("X.509");
        CertPath certPath = certFactory.generateCertPath(idmAccessor
                .getSAMLAuthorityChain());
        SamlService service2 = factory2.createSamlService(
                idmClient.getTenantPrivateKey(VSPHERE_LOCAL_TENANT),
                SignatureAlgorithm.RSA_SHA256, SignatureAlgorithm.RSA_SHA256,
                idmClient.getEntityID(VSPHERE_LOCAL_TENANT), certPath);

        // now call it again with generated signature
        String vcdSignature = "tTUaPscQSmPKkqP9XGgCHZYoH%2FUy2MvZ1eoeP%2B3Y" +
                "nTDLxiuV5glxngtMbOGspo9NbL37lNVjdCUo7qQVDznUNmKpIOGa%2BGwE" +
                "jcgqeS7mBDsYPcICxVHZPYxbIaFCmlTIo125olswe4LuP92lIroe%2B%2F" +
                "DpeNXIGjUAFLHQwlLO7r73cHLH%2BPY2pcYww4X2I7Mhk%2FQ7I3tdMX1O" +
                "eOhqcRpMn8uyOs6JmVbMoVXuTVKyO96LmQUPCQVLmVjDeD%2BZjVALVLbs" +
                "vjWsdFt%2F%2Ff2MEXIQkYmeIM5HxZ5rW0uXRocarUrp8nhgxk%2FEQGhk" +
                "00KYP1xZTCC9JZR6OcbXJZZemBgq%2BA%3D%3D";
        vcdSignature = URLDecoder.decode(vcdSignature, "UTF-8");
        // just call verifySignature method and expect to not throw

        service2.verifySignature(message, vcdSignature); */
    }

    @Test
    public void testDecodeSamlAuthnRequest() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        // create basic SAML Authn request
        String id = "42"; // the answer to life the universe and everything
        int tenantId = 0;
        AuthnRequest authnRequest = SharedUtils.createSamlAuthnRequest(id, tenantId);
        StringBuffer sbRequestUrl = new StringBuffer();
        sbRequestUrl.append(authnRequest.getDestination());

        // encode
        HttpServletRequest request = SharedUtils.buildMockRequestObject(
                authnRequest, null, null, null, sbRequestUrl, null, null, tenantId);

        AuthnRequest decodedAuthnRequest = service
                .decodeSamlAuthnRequest(request);
        assertTrue(decodedAuthnRequest.getID().equals(id));
        assertTrue(decodedAuthnRequest.getVersion().equals(
                SAMLVersion.VERSION_20));
    }

    @Test
    public void testCreateSamlResponse() throws UnmarshallingException {
        Response response = service.createSamlResponse("42", acsUrl,
                OasisNames.REQUESTER, OasisNames.REQUEST_UNSUPPORTED,
                RESPONSE_MESSAGE, null);
        assertNotNull(response);
    }

    @Test
    public void testCreateSamlResponseWithToken() throws SAXException,
            IOException, ParserConfigurationException, UnmarshallingException {
        Document token = readToken();
        Response response = service.createSamlResponse("42", acsUrl,
                OasisNames.REQUESTER, OasisNames.REQUEST_UNSUPPORTED,
                RESPONSE_MESSAGE, token);
        assertNotNull(response);
    }

    @Test
    public void testBuildPostResponseForm() throws UnmarshallingException {
        Response response = service.createSamlResponse("42", acsUrl,
                OasisNames.REQUESTER, OasisNames.REQUEST_UNSUPPORTED,
                RESPONSE_MESSAGE, null);
        String relayState = "{\"dest\":\"d2d25106-44ee-4e36-877e-1d7aa1335dcd\",\"idpId\":53}";
        String formHtml = service.buildPostResponseForm(response, relayState, acsUrl);
        assertNotNull(formHtml);

        // find input attribute for RelayState in html post form, validate the expected RelayState is embedded in the form
        Pattern inputPattern = Pattern.compile("<input(.*?)>", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
        Pattern attributePattern = Pattern.compile("(\\w+)=\"(.*?)\"");
        Matcher inputPatternMatcher = inputPattern.matcher(formHtml);
        boolean relayStateFound = false;
        while (inputPatternMatcher.find()) {
            String attributesStr = inputPatternMatcher.group(1);
            Matcher attributePatternMatcher = attributePattern.matcher(attributesStr);
            while (attributePatternMatcher.find()) {
                if (attributePatternMatcher.group(1).equals("name")) {
                    String key = attributePatternMatcher.group(2).trim();
                    if (key.equals("RelayState")) {
                        attributePatternMatcher.find();
                        if (attributePatternMatcher.group(1).equals("value")) {
                            String value = StringEscapeUtils.unescapeHtml(attributePatternMatcher.group(2).trim());
                            assertEquals(relayState, value);
                            relayStateFound = true;
                            break;
                        }
                    }
                }
            }
        }
        assertTrue(relayStateFound);
    }

    // read token from resource stream
    private Document readToken() throws SAXException, IOException,
            ParserConfigurationException {
        InputStream is = new FileInputStream(SamlServiceTest.class.getResource(
                "/saml-assertion.xml").getFile());
        return SharedUtils.readXml(is);
    }
}
