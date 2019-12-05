/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client.sample;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.net.InetAddress;
import java.net.URL;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.TimeZone;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509TrustManager;
import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.ssl.TrustStrategy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.servlet.ModelAndView;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import org.apache.commons.codec.binary.Base64;

import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.HostRetriever;
import com.vmware.identity.rest.core.client.SimpleHostRetriever;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.CertificateResource;
import com.vmware.identity.rest.idm.client.IdmClient;
import com.vmware.identity.rest.idm.client.RelyingPartyResource;
import com.vmware.identity.rest.idm.data.AssertionConsumerServiceDTO;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.RelyingPartyDTO;
import com.vmware.identity.rest.idm.data.ServiceEndpointDTO;
import com.vmware.identity.rest.idm.data.SignatureAlgorithmDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateGranularity;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;
import com.vmware.identity.websso.client.AssertionConsumerService;
import com.vmware.identity.websso.client.IDPConfiguration;
import com.vmware.identity.websso.client.IDPConfigurationFactory;
import com.vmware.identity.websso.client.MetadataSettings;
import com.vmware.identity.websso.client.SPConfiguration;
import com.vmware.identity.websso.client.SingleLogoutService;
import com.vmware.identity.websso.client.SingleSignOnService;
import com.vmware.identity.wstrust.client.UsernamePasswordCredential;
import com.vmware.identity.wstrust.client.impl.DefaultSecurityTokenServiceFactory;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.identity.wstrust.client.SecurityTokenService;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.ConnectionConfig;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.SSLTrustedManagerConfig;
import com.vmware.identity.wstrust.client.TokenSpec;

/**
 * Component that initializes the library settings and provides some useful
 * static utility methods
 *
 */
public class ComponentUtils {
  private static final Logger logger = LoggerFactory.getLogger(ComponentUtils.class);

    private static final String ORG1 = "Org1"; // sample organization name
    private static final String ORG2 = "Org2"; // sample organization name
    private static final String SIGNATURE_ALGORITHM = "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256"; // or
    private static final String SESSION_COOKIE_NAME = "SSOLIB-SAMPLE-SESSION";
    private static final String SESSION_INDEX_COOKIE_NAME = "SSOLIB-SAMPLE-SESSION-INDEX";
    private static final String KEYSTORE_FILENAME = "sts-store.jks";
    private static final String KEYSTORE_PASSWORD = "Passw0rd$";

    private static ServiceLocator serviceLocator = null;

    /**
     * Configure our sample organizations
     *
     * @param metadataSettings
     * @param serviceProviderFQDN
     * @throws Exception
     */
    public static void populateSettings(
        MetadataSettings metadataSettings,
            String serviceProviderFQDN,
      String tenantAdminUsername,
            String tenantAdminPassword,
            String tenant,
      String spPort
  ) throws Exception {
      String [] orgs = new String [] { ORG1, ORG2 };

    serviceLocator = new ServiceLocator(serviceProviderFQDN, tenant);

    logger.info("Retrieved Service Locator - " + serviceLocator.toString());

    String idpDoc = exportTenantConfiguration(serviceLocator);

      for (String org : orgs) {
      logger.info("Registering service provider for " + org);
      // Org2 is auto-configured to talk to IDP at websso registered to lookupservice address:
      // 1. generate SPConfiguration object and store it
      // 2. get IDPConfiguration XML from websso Metadata
      // 3. store IDPConfiguration
      // 4. store SPConfiguration XML using admin REST API
      SPConfiguration spConfig = generateSPConfiguration(org, spPort);
      metadataSettings.addSPConfiguration(spConfig);

      logger.info(
          String.format(
              "Getting Service Locator for FQDN:%s, Tenant:%s",
              serviceProviderFQDN,
              tenant
          )
      );

      IDPConfiguration idpConfig = getIDPConfigurationFromMetadata(org, idpDoc);
      metadataSettings.addIDPConfiguration(idpConfig);

      exportSPConfigToIDP(serviceLocator, tenantAdminUsername, tenantAdminPassword, spConfig);
    }
    }

    /**
     * Export SP metadata to IDP
     *
     * @param serviceLocator
     * @param tenantAdminUsername
     * @param tenantAdminPassword
     * @param tenant
     * @param metadata
     *
     * @throws Exception
     */
    public static void exportSPConfigToIDP(ServiceLocator serviceLocator, String tenantAdminUsername,
            String tenantAdminPassword, String tenant, String metadata) throws Exception {
        exportSPConfigToIDP(serviceLocator, tenantAdminUsername, tenantAdminPassword,
                getSPConfigurationFromMetadata(tenant, metadata));
    }

    /**
     * Export SP configuration to IDP
     *
     * @param serviceLocator
     * @param tenantAdminUsername
     * @param tenantAdminPassword
     * @param spConfig
     * @throws Exception
     */
    public static void exportSPConfigToIDP(ServiceLocator serviceLocator,
            String tenantAdminUsername, String tenantAdminPassword,
            SPConfiguration spConfig) throws Exception {

        SamlToken samlToken = getBearerTokenByUsernamePassword(serviceLocator, tenantAdminUsername, tenantAdminPassword);
        HostRetriever hostRetriever = new SimpleHostRetriever(serviceLocator.getHost(), true);
        IdmClient client = new IdmClient(hostRetriever, new NoopHostnameVerifier(), new SSLContextBuilder()
                .loadTrustMaterial(null, new SampleTrustStrategy(serviceLocator.getSTSSSLCertificates())).build());

        client.setToken(new AccessToken(Base64.encodeBase64String(samlToken.toXml().getBytes()), AccessToken.Type.SAML));
        RelyingPartyResource rpResource = client.relyingParty();
        List<RelyingPartyDTO> rpDTOList = rpResource.getAll(serviceLocator.getTenant());

        for (RelyingPartyDTO rpDTO : rpDTOList) {
            if (rpDTO.getName().equals(spConfig.getAlias())) {
                rpResource.update(serviceLocator.getTenant(), rpDTO.getName(), getRelyingParty(spConfig));
                return;
            }
        }

        rpResource.register(serviceLocator.getTenant(), getRelyingParty(spConfig));
    }

    /**
     * Create Relying party from SPConfiguration
     *
     * @param spConfig
     * @return
     * @throws CertificateEncodingException
     */
    private static RelyingPartyDTO getRelyingParty(SPConfiguration spConfig) throws CertificateEncodingException {
        // get Relying Party certificate from key store
        X509Certificate cert = spConfig.getSigningCertificate();

        CertificateDTO certDTO;
        certDTO = new CertificateDTO(cert);

        return new RelyingPartyDTO.Builder().withName(spConfig.getAlias()).withUrl(spConfig.getEntityID())
                .withAssertionConsumerServices(generateAssertionConsumerServices(spConfig))
                .withAuthnRequestsSigned(spConfig.isAuthnRequestsSigned()).withCertificate(certDTO)
                .withSignatureAlgorithms(generateSignatureAlgorithms(spConfig))
                .withSingleLogoutServices(generateSingleLogoutServices(spConfig)).build();
    }

    private static List<AssertionConsumerServiceDTO> generateAssertionConsumerServices(SPConfiguration spConfig) {
        List<AssertionConsumerServiceDTO> acsList = new ArrayList<AssertionConsumerServiceDTO>();
        if (spConfig.getAssertionConsumerServices() != null) {
            for (AssertionConsumerService element : spConfig.getAssertionConsumerServices()) {
                AssertionConsumerServiceDTO acs = new AssertionConsumerServiceDTO.Builder()
                        .withName(element.getLocation() + element.getBinding()).withBinding(element.getBinding())
                        .withEndpoint(element.getLocation()).withIndex(element.getIndex()).build();
                acsList.add(acs);
            }
        }

        return acsList;
    }

    private static List<SignatureAlgorithmDTO> generateSignatureAlgorithms(SPConfiguration spConfig) {
        return Arrays.asList(new SignatureAlgorithmDTO.Builder().withMaxKeySize(1024).withMinKeySize(256)
                .withPriority(1).build());
    }

    private static List<ServiceEndpointDTO> generateSingleLogoutServices(SPConfiguration spConfig) {

        List<ServiceEndpointDTO> acsList = new ArrayList<ServiceEndpointDTO>();

        if (spConfig.getSingleLogoutServices() != null) {
            for (SingleLogoutService element : spConfig.getSingleLogoutServices()) {
                ServiceEndpointDTO acs = new ServiceEndpointDTO.Builder()
                        .withName(element.getLocation() + element.getBinding()).withEndpoint(element.getLocation())
                        .withBinding(element.getBinding()).build();

                acsList.add(acs);
            }
        }

        return acsList;
    }

    /**
     * Get Metadata from websso
     *
     * @return
     * @throws Exception
     */
    private static String exportTenantConfiguration(
            ServiceLocator serviceLocator) throws Exception {

        URL metadataURL = serviceLocator.getWebssoUrl();

        String webssoSSLAlias = "webssoSSL";
        KeyStore keyStore = getKeyStore();
        if (!keyStore.containsAlias(webssoSSLAlias))
            keyStore.setCertificateEntry(webssoSSLAlias, serviceLocator.getWebssoSSLCertificates());

        HttpsURLConnection conn = (HttpsURLConnection) metadataURL.openConnection();

        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory
                .getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        SSLContext ctx = SSLContext.getInstance("SSL");
        ctx.init(null, trustManagerFactory.getTrustManagers(), null);
        SSLSocketFactory sslFactory = ctx.getSocketFactory();
        conn.setSSLSocketFactory(sslFactory);

        BufferedReader reader = new BufferedReader(new InputStreamReader(
                conn.getInputStream()));
        String inputLine;
        StringBuffer str = new StringBuffer();
        while ((inputLine = reader.readLine()) != null) {
            str.append(inputLine);
            str.append(System.lineSeparator());

        }
        reader.close();

        return str.toString();

    }

    /**
     * Get admin SAML token from STS by username/password
     *
     * @return
     * @throws Exception
     */
    private static SamlToken getBearerTokenByUsernamePassword(
            ServiceLocator serviceLocator, String tenantAdminUsername,String tenantAdminPassword) throws Exception {

        URL url = serviceLocator.getSTSURL();


        X509Certificate[] trustedCertificates = getTrustedCertificates(serviceLocator);
        X509Certificate[] sslCerts = serviceLocator.getAdminSSLCertificates();

        KeyStore ks = KeyStore.getInstance(KeyStore.getDefaultType());
        ks.load(null, null);
        int iCert = 1;
        for (X509Certificate cert : sslCerts) {
          ks.setCertificateEntry(String.format("Cert-%d", iCert++), cert);
        }

        SSLTrustedManagerConfig trustMgr = new SSLTrustedManagerConfig(ks);

        ConnectionConfig connConfig = new ConnectionConfig(url, trustMgr);

        SecurityTokenServiceConfig stsConfig = new SecurityTokenServiceConfig(
                                                      connConfig,
                                                      trustedCertificates,
                                         null,
                                       null);

        SecurityTokenService sts = DefaultSecurityTokenServiceFactory.getSecurityTokenService(stsConfig);


        TokenSpec.Builder tokenSpecBuilder = new TokenSpec.Builder(600);
        TokenSpec tokenSpec = tokenSpecBuilder.createTokenSpec();

        SamlToken samlToken = sts.acquireToken(
                                    new UsernamePasswordCredential(
                                          tenantAdminUsername,
                                          tenantAdminPassword
                                    ),
                                    tokenSpec
                              );

        return samlToken;
    }

    private static X509Certificate[] getTrustedCertificates(
            ServiceLocator serviceLocator) throws Exception {
        List<X509Certificate> x509Certs = new ArrayList<X509Certificate>();
        HostRetriever hostRetriever = new SimpleHostRetriever(serviceLocator.getHost(), true);

        IdmClient client = new IdmClient(
        hostRetriever, new NoopHostnameVerifier(), new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {
            @Override
            public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                return true;
            }
        }).build());

        client.setToken(new AccessToken(null, AccessToken.Type.SAML));
        CertificateResource certificateResource = client.certificate();
        List<CertificateChainDTO> chains = certificateResource.get(serviceLocator.getTenant(), CertificateScope.TENANT, CertificateGranularity.CHAIN);

        if (chains != null) {
            for (CertificateChainDTO certificateChainDTO : chains) {
                for (CertificateDTO certificateDTO : certificateChainDTO.getCertificates()) {
                    x509Certs.add(certificateDTO.getX509Certificate());
                }

            }
        }

        return x509Certs.toArray(new X509Certificate[x509Certs.size()]);
    }

    static KeyStore getKeyStore() throws KeyStoreException,
            NoSuchAlgorithmException, CertificateException, IOException {
        KeyStore ks = null;

        ks = KeyStore.getInstance(KeyStore.getDefaultType());
        InputStream is = ComponentUtils.class.getClassLoader().getResourceAsStream(KEYSTORE_FILENAME);

        char[] stsKeystorePassword = KEYSTORE_PASSWORD.toCharArray();

        ks.load(is, stsKeystorePassword);

        return ks;
    }

    /**
     * Create DOM from String
     *
     * @param strXML
     * @return
     * @throws ParserConfigurationException
     * @throws SAXException
     * @throws IOException
     */
    private static final Document createDOM(String strXML)
            throws ParserConfigurationException, SAXException, IOException {

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
        dbf.setValidating(true);
        dbf.setIgnoringComments(false);
        dbf.setIgnoringElementContentWhitespace(true);
        dbf.setNamespaceAware(true);

        DocumentBuilder db = dbf.newDocumentBuilder();
        db = dbf.newDocumentBuilder();
        db.setEntityResolver(new NullResolver());

        InputSource sourceXML = new InputSource(new StringReader(strXML));
        Document xmlDoc = db.parse(sourceXML);
        return xmlDoc;
    }

    /**
     * @param orgName
     * @return
     * @throws Exception
     * @throws KeyStoreException
     * @throws FileNotFoundException
     * @throws IOException
     * @throws NoSuchAlgorithmException
     * @throws CertificateException
     * @throws UnrecoverableKeyException
     */
    private static SPConfiguration generateSPConfiguration(String orgName, String spPort)
            throws Exception, KeyStoreException, FileNotFoundException,
            IOException, NoSuchAlgorithmException, CertificateException,
            UnrecoverableKeyException {
        SPConfiguration spConfig;
        String entityID;
        PrivateKey signingPrivateKey;
        X509Certificate certificate;
        List<String> nameIDFormats;
        List<AssertionConsumerService> assertionConsumerServices;
        List<SingleLogoutService> singleLogoutServices;

        logger.info("Registering service provider configuration");

        // set endpoints
        if(spPort != null && !spPort.isEmpty() && !spPort.equals("443")) {
          entityID = String.format("https://%s:%s/ssolib-sample/SsoClient/Metadata/%s", getHostName(), spPort, orgName);
        } else {
            entityID = String.format("https://%s/ssolib-sample/SsoClient/Metadata/%s", getHostName(), orgName);
        }

        logger.info("Registering entity ID - " + entityID);

    assertionConsumerServices = new ArrayList<AssertionConsumerService>();
    assertionConsumerServices.add(new AssertionConsumerService(entityID.replace("SsoClient/Metadata",
            "SsoClient/SSO"), true, SAMLNames.HTTP_POST_BINDING, 0));

    singleLogoutServices = new ArrayList<SingleLogoutService>();
    singleLogoutServices.add(new SingleLogoutService(entityID.replace("SsoClient/Metadata", "SsoClient/SLO"),
            SAMLNames.HTTP_REDIRECT_BINDING));
    singleLogoutServices.add(new SingleLogoutService(entityID.replace("SsoClient/Metadata", "SsoClient/SLO"),
            SAMLNames.SOAP_BINDING));

    // set nameID formats
    nameIDFormats = new ArrayList<String>();
    nameIDFormats.add(SAMLNames.IDFORMAT_VAL_PERSIST);
    nameIDFormats.add(SAMLNames.IDFORMAT_VAL_EMAILADD);

    // set keys
    KeyStore ks = getKeyStore();

    String stsAlias = "stskey";
    Key key = ks.getKey(stsAlias, KEYSTORE_PASSWORD.toCharArray());
    signingPrivateKey = (PrivateKey) key;

    // read certificate
    certificate = (X509Certificate) ks.getCertificate(stsAlias);

    // create configuration object and store it
    spConfig = new SPConfiguration(orgName, entityID, true, signingPrivateKey, certificate, SIGNATURE_ALGORITHM,
            nameIDFormats, assertionConsumerServices, singleLogoutServices);
    return spConfig;
    }

    /**
     * Create session cookie for the user
     *
     * @param response
     * @param userIdentity
     * @param sessionIndex
     */
    public static void setSessionCookie(HttpServletResponse response,
            String userIdentity, String sessionIndex) {
        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, userIdentity);
        sessionCookie.setPath("/"); // apply cookie to all pages on the server
        response.addCookie(sessionCookie);
        Cookie sessionIndexCookie = new Cookie(SESSION_INDEX_COOKIE_NAME, sessionIndex);
        sessionIndexCookie.setPath("/"); // apply cookie to all pages on the
                                         // server
        response.addCookie(sessionIndexCookie);
    }

    /**
     * Return user identity from the cookie
     *
     * @param request
     * @return
     */
    public static String getSessionCookieValue(HttpServletRequest request) {
        return getCookieValue(request.getCookies(), SESSION_COOKIE_NAME, null);
    }

    /**
     * Return user session index from the cookie
     *
     * @param request
     * @return
     */
    public static String getSessionIndexCookieValue(HttpServletRequest request) {
        return getCookieValue(request.getCookies(), SESSION_INDEX_COOKIE_NAME,
                null);
    }

     /**
     * Utility method to convert SAML Metadata into an SP Configuration object
     *
     * @param alias
     *            Alias (or tenant name) to use
     * @param metadata
     *            SAML metadata as a string
     * @return
     * @throws Exception
     */
    public static SPConfiguration getSPConfigurationFromMetadata(String alias, String metadata) throws Exception {
        // build a document from string and call import
        InputStream is = new ByteArrayInputStream(metadata.getBytes());

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        DocumentBuilder docBuilder;
        docBuilder = factory.newDocumentBuilder();
        Document doc = docBuilder.parse(is);

        SPConfiguration retval = getSPConfigurationFromMetadata(doc);
        retval.setAlias(alias);
        return retval;
    }

    private static SPConfiguration getSPConfigurationFromMetadata(Document metadata) throws Exception {
        NodeList entities = metadata.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_METADATA,
                SAMLNames.ENTITIESDESCRIPTOR);
        Element entitiesEle = (Element) entities.item(0);

        String name = entitiesEle.getAttribute(SAMLNames.NAME);

        NodeList entity = entitiesEle.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_METADATA, SAMLNames.ENTDESCRIPTOR);
        Element entityEl = ((Element) entity.item(0));
        String entityId = entityEl.getAttribute(SAMLNames.ENTID);

        NodeList spList = entityEl.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_METADATA, SAMLNames.SPSSODESCRIPTOR);
        if (spList.getLength() == 0) {
            throw new Exception("SAML medadata error: file " + "does not have an sp descriptor!");
        }

        Element spSSOEle = (Element) spList.item(0);

        Collection<Certificate> certificates = getCertificates(spSSOEle);

        List<String> nameIDFormats = parseNameIDFormats(spSSOEle);
        List<AssertionConsumerService> ssoServices = parseAssertionConsumerService(spSSOEle);
        List<SingleLogoutService> sloServices = parseSloServices(spSSOEle);

        boolean authnRequestsSigned = Boolean.parseBoolean(spSSOEle.getAttribute(SAMLNames.AUTHNREQUESTSIGNED));
        X509Certificate signingCertificate = null;
        if (certificates != null && certificates.size() > 0)
            signingCertificate = (X509Certificate) (((ArrayList<Certificate>) certificates).get(0));
        return new SPConfiguration(name, entityId, authnRequestsSigned, null, signingCertificate, SIGNATURE_ALGORITHM,
                nameIDFormats, ssoServices, sloServices);
    }

    private static Collection<Certificate> getCertificates(Element spSSOEle) throws Exception, CertificateException {
        NodeList keyDescriptorList = spSSOEle.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_METADATA,
                SAMLNames.KEYDESCRIPTOR);
        if (keyDescriptorList.getLength() == 0) {
            throw new Exception("SAML metadata error: no signing key for SP");
        }

        Element keyDescriptorEle = null;
        // iterate through keys, to find the signing one
        for (int i = 0; i < keyDescriptorList.getLength(); i++) {
            if (((Element) keyDescriptorList.item(i)).getAttribute(SAMLNames.USE).equals(SAMLNames.SIGNING)) {
                keyDescriptorEle = ((Element) keyDescriptorList.item(i));
                break;
            }
        }
        // "use" attribute should be "signing" ignore other types.
        if (keyDescriptorEle == null) {
            throw new Exception("SAML metadata error: no signing key for SP");
        }
        // Get Keyinfo. no need to check availability since it is
        // schema-enforced.
        NodeList keyInfoList = keyDescriptorEle.getElementsByTagNameNS(SAMLNames.NS_NAME_SAML_DIGTALSIG,
                SAMLNames.KEYINFO);
        Element keyInfoEle = (Element) keyInfoList.item(0);

        Collection<Certificate> tenantCertificates = parseCertificates(keyInfoEle);
        return tenantCertificates;
    }

    private static List<AssertionConsumerService> parseAssertionConsumerService(Element idpSSOEle) {
        NodeList nodes = idpSSOEle.getElementsByTagName(SAMLNames.ASSERTIONCONSUMERSERVICE);
        if (nodes.getLength() == 0) {
            return null;
        }

        List<AssertionConsumerService> acsServices = new ArrayList<AssertionConsumerService>();
        for (int i = 0; i < nodes.getLength(); i++) {
            Element ele = (Element) nodes.item(i);
            acsServices.add(new AssertionConsumerService(ele.getAttribute(SAMLNames.LOCATION), Boolean.parseBoolean(ele
                    .getAttribute(SAMLNames.ISDEFAULT)), ele.getAttribute(SAMLNames.BINDING), Integer.parseInt(ele
                    .getAttribute(SAMLNames.INDEX))));
        }

        if (acsServices.size() == 0) {
            return null;
        }
        return acsServices;
    }

    /**
     * Utility method to convert SAML Metadata into an IDP Configuration object
     *
     * @param alias
     *            Alias (or tenant name) to use
     * @param metadata
     *            SAML metadata as a string
     * @return
     * @throws Exception
     */
    public static IDPConfiguration getIDPConfigurationFromMetadata(
            String alias, String metadata) throws Exception {
        // build a document from string and call import
        InputStream is = new ByteArrayInputStream(metadata.getBytes());

        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        DocumentBuilder docBuilder;
        docBuilder = factory.newDocumentBuilder();
        Document doc;
        doc = docBuilder.parse(is);

        IDPConfiguration retval = getIDPConfigurationFromMetadataDocument(doc);
        retval.setAlias(alias);
        return retval;
    }

    /**
     * Utility method to convert SP Configuration object into SAML Metadata
     * string
     *
     * @param spConfiguration
     *            SP Configuration to use
     * @return
     * @throws Exception
     */
    public static String getMetadataFromSPConfiguration(
            SPConfiguration spConfiguration) throws Exception {
        Document doc = getMetadataDocumentFromSPConfiguration(spConfiguration);
        return getStringFromDocument(doc);
    }

    /**
     * Helper method to create error view out of exception object
     *
     * @param e
     * @return
     */
    public static ModelAndView getErrorView(Exception e) {
        ModelAndView model = new ModelAndView("error");
        model.addObject("Data", e.getMessage() + "\n\n" + getStackTrace(e));
        return model;
    }

    /**
     * Return exception stack trace as a string
     *
     * @param throwable
     * @return
     */
    public static String getStackTrace(Throwable throwable) {
        Writer writer = new StringWriter();
        PrintWriter printWriter = new PrintWriter(writer);
        throwable.printStackTrace(printWriter);
        return writer.toString();
    }

    /**
     * Create XML Document from SPConfiguration object - helper method
     *
     * @param spConfiguration
     * @return
     * @throws ParserConfigurationException
     * @throws CertificateEncodingException
     */
    private static Document getMetadataDocumentFromSPConfiguration(
            SPConfiguration spConfiguration)
            throws ParserConfigurationException, CertificateEncodingException {
        DocumentBuilder docBuilder = DocumentBuilderFactory.newInstance()
                .newDocumentBuilder();
        Document doc = docBuilder.newDocument();

        Element root = createEntitiesDescriptor(doc, spConfiguration);
        doc.appendChild(root);

        // Export SP
        Element entEle = createSPEntityDescriptor(doc, spConfiguration);
        root.appendChild(entEle);

        return doc;
    }

    /**
     * Helper method which creates SP Entity Descriptor
     *
     * @param doc
     * @param spConfiguration
     * @return
     * @throws CertificateEncodingException
     */
    private static Element createSPEntityDescriptor(Document doc,
            SPConfiguration spConfiguration)
            throws CertificateEncodingException {

        Element entEle = doc.createElementNS(null, SAMLNames.ENTDESCRIPTOR);
        entEle.setAttribute(SAMLNames.ENTID, spConfiguration.getEntityID());

        // SPSSODescriptor
        Element spSSO = createSPSSODescriptor(doc, spConfiguration);
        entEle.appendChild(spSSO);

        return entEle;
    }

    /**
     * Helper method which creates SPSSODescriptor
     *
     * @param doc
     * @param spConfiguration
     * @return
     * @throws CertificateEncodingException
     */
    private static Element createSPSSODescriptor(Document doc,
            SPConfiguration spConfiguration)
            throws CertificateEncodingException {
        Element spssoEle = doc.createElement(SAMLNames.SPSSODESCRIPTOR);
        spssoEle.setAttribute(SAMLNames.AUTHNREQUESTSIGNED, new Boolean(
                spConfiguration.isAuthnRequestsSigned()).toString());
        spssoEle.setAttribute(SAMLNames.PSE, SAMLNames.REQUIREDPROTOCAL);

        Element keyD = createSPKeyDescriptor(doc, spConfiguration);
        if (keyD != null) {
            spssoEle.appendChild(keyD);
        }

        // export single logout services.
        Collection<SingleLogoutService> sloList = spConfiguration
                .getSingleLogoutServices();
        for (SingleLogoutService slo : sloList) {
            Element sloEle = createSingleLogoutService(doc, slo);
            if (sloEle != null) {
                spssoEle.appendChild(sloEle);
            }
        }

        // export name id formats.
        Collection<String> nameIDFormats = spConfiguration.getNameIDFormats();
        for (String nameIDFormat : nameIDFormats) {
            Element nidEle = createNameIDFormat(doc, nameIDFormat);
            if (nidEle != null) {
                spssoEle.appendChild(nidEle);
            }
        }

        // export assertion consumer services.
        Collection<AssertionConsumerService> acsList = spConfiguration
                .getAssertionConsumerServices();
        Iterator<AssertionConsumerService> it = acsList.iterator();
        while (it.hasNext()) {
            AssertionConsumerService acs = it.next();
            Element acsEle = createAssertionConsumerService(doc, acs);
            if (acsEle != null) {
                spssoEle.appendChild(acsEle);
            }
        }
        return spssoEle;
    }

    /**
     * Helper method to create KeyDescriptor element from SPConfiguration
     *
     * @param doc
     * @param spConfiguration
     * @return
     * @throws CertificateEncodingException
     */
    private static Element createSPKeyDescriptor(Document doc,
            SPConfiguration spConfiguration)
            throws CertificateEncodingException {
        Element keyD = null;

        X509Certificate cert = spConfiguration.getSigningCertificate();
        if (cert == null) {
            return null;
        }
        keyD = doc.createElement(SAMLNames.KEYDESCRIPTOR);
        keyD.setAttribute(SAMLNames.NS_NAME_SAML_DS,
                SAMLNames.NS_NAME_SAML_DIGTALSIG);
        keyD.setAttribute(SAMLNames.USE, SAMLNames.SIGNING);

        Element keyInfoEle = doc.createElement(SAMLNames.DS_KEYINFO);
        Element x509DataEle = doc.createElement(SAMLNames.DS_X509DATA);
        Element x509CertEle = createCertificate(doc, cert);
        x509DataEle.appendChild(x509CertEle);
        keyInfoEle.appendChild(x509DataEle);
        keyD.appendChild(keyInfoEle);

        return keyD;
    }

    /**
     * Helper method to create Certificate node
     *
     * @param doc
     * @param cert
     * @return
     * @throws CertificateEncodingException
     */
    private static Element createCertificate(Document doc, X509Certificate cert)
            throws CertificateEncodingException {
        Element x509CertificateEle = doc
                .createElement(SAMLNames.DS_X509CERTIFICATE);
        String base64Str = Base64.encodeBase64String(cert.getEncoded());
        Node certText = doc.createTextNode(base64Str);

        x509CertificateEle.appendChild(certText);
        return x509CertificateEle;
    }

    /**
     * Helper method to create NameIDFormat element
     *
     * @param doc
     * @param nameIDFormat
     * @return
     */
    private static Element createNameIDFormat(Document doc, String nameIDFormat) {
        Element nidEle = doc.createElement(SAMLNames.NAMEIDFORMAT);
        nidEle.appendChild(doc.createTextNode(nameIDFormat));

        return nidEle;
    }

    /**
     * Helper method to create AssertionConsumerService element
     *
     * @param doc
     * @param service
     * @return
     */
    private static Element createAssertionConsumerService(Document doc,
            AssertionConsumerService service) {
        if (service == null) {
            return null;
        }
        Element acsEle = doc.createElement(SAMLNames.ASSERTIONCONSUMERSERVICE);
        String loc = service.getLocation();
        acsEle.setAttribute(SAMLNames.BINDING, service.getBinding());
        acsEle.setAttribute(SAMLNames.LOCATION, loc);
        acsEle.setAttribute(SAMLNames.INDEX,
                Integer.toString(service.getIndex()));

        if (service.isDefault()) {
            acsEle.setAttribute(SAMLNames.ISDEFAULT, SAMLNames.TRUE);
        }
        return acsEle;
    }

    /**
     * Helper method to create SingleLogoutService element
     *
     * @param doc
     * @param service
     * @return
     */
    private static Element createSingleLogoutService(Document doc,
            SingleLogoutService service) {
        if (service == null) {
            return null;
        }
        Element sloEle = doc.createElement(SAMLNames.SINGLELOGOUTSERVICE);
        String loc = service.getLocation();
        String rspLoc = service.getResponseLocation();
        sloEle.setAttribute(SAMLNames.BINDING, service.getBinding());
        sloEle.setAttribute(SAMLNames.LOCATION, loc);
        sloEle.setAttribute(SAMLNames.RESPONSE_LOCATION, rspLoc);

        return sloEle;
    }

    /**
     * Method which creates EntitiesDescriptor element
     *
     * @param doc
     * @param spConfiguration
     * @return
     */
    private static Element createEntitiesDescriptor(Document doc,
            SPConfiguration spConfiguration) {
        Element entitiesEle = doc.createElementNS(null,
                SAMLNames.ENTITIESDESCRIPTOR);
        entitiesEle.setAttribute(SAMLNames.XMLNS,
                SAMLNames.NS_NAME_SAML_METADATA);
        entitiesEle.setAttribute(SAMLNames.NS_NAME_SAML_SAML,
                SAMLNames.NS_VAL_SAML_SAML);

        entitiesEle.setAttribute(SAMLNames.NAME, spConfiguration.getAlias());

        return entitiesEle;
    }

    /**
     * Utility method to convert SAML Metadata document into an IDP
     * Configuration object
     *
     * @param tenantDoc
     * @return
     * @throws Exception
     */
    private static IDPConfiguration getIDPConfigurationFromMetadataDocument(
            Document tenantDoc) throws Exception {
        NodeList entNodes = tenantDoc.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA, SAMLNames.ENTDESCRIPTOR);
        Element entityEle = (Element) entNodes.item(0);
        if (isExpired(entityEle)) {
            throw new Exception("Document has expired!");
        }
        return importIDPEntity(entityEle);
    }

    /**
     * Convert IDP Entity element into IDPConfiguration object - helper method
     *
     * @param entity
     * @return
     * @throws Exception
     */
    private static IDPConfiguration importIDPEntity(Element entity)
            throws Exception {
        // Set entity id and issuerName
        String entityID = entity.getAttribute(SAMLNames.ENTID);

        NodeList idpList = entity.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_METADATA, SAMLNames.IDPSSODESCRIPTOR);
        if (idpList.getLength() == 0) {
            throw new Exception("SAML medadata error: file "
                    + "does not have a idp or sp descriptor!");
        }

        Element idpSSOEle = (Element) idpList.item(0);
        return importIDPSSODescriptor(idpSSOEle, entityID);
    }

    /**
     * Convert IDPSSODescriptor into IDPConfiguration object
     *
     * @param idpSSOEle
     * @param entityID
     * @return
     * @throws Exception
     */
    private static IDPConfiguration importIDPSSODescriptor(Element idpSSOEle,
            String entityID) throws Exception {
         Collection<Certificate> tenantCertificates = getCertificates(idpSSOEle);// parseCertificates(keyInfoEle);
            List<String> nameIDFormats = parseNameIDFormats(idpSSOEle);
            List<SingleSignOnService> ssoServices = parseSsoServices(idpSSOEle);
            List<SingleLogoutService> sloServices = parseSloServices(idpSSOEle);

            IDPConfiguration retval = IDPConfigurationFactory.createAffinitizedIDPConfiguration(entityID, entityID,
                    (X509Certificate) tenantCertificates.toArray()[0], nameIDFormats, ssoServices, sloServices);


            return retval;
    }

    /**
     * Read certificates from metadata - helper method
     *
     * @param keyInfoEle
     * @return
     * @throws CertificateException
     */
    private static Collection<Certificate> parseCertificates(Element keyInfoEle)
            throws CertificateException {
        // expecting one certificate chain at least.
        NodeList nodes = keyInfoEle.getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_DIGTALSIG, SAMLNames.X509DATA);
        if (nodes.getLength() == 0) {
            return null;
        }
        // expecting one or more certificate in the chain
        NodeList certList = ((Element) nodes.item(0)).getElementsByTagNameNS(
                SAMLNames.NS_NAME_SAML_DIGTALSIG, SAMLNames.X509CERTIFICATE);
        if (certList.getLength() == 0) {
            return null;
        }

        List<Certificate> certificates = new ArrayList<Certificate>();
        for (int i = 0; i < certList.getLength(); i++) {

            Element x509Ele = (Element) certList.item(0);

            if (x509Ele != null) {

                byte[] certDecoded = Base64.decodeBase64(x509Ele.getTextContent());
                CertificateFactory cf = CertificateFactory.getInstance("X.509");
                X509Certificate c = (X509Certificate) cf
                        .generateCertificate(new ByteArrayInputStream(
                                certDecoded));
                certificates.add(c);
            }
        }

        if (certificates.size() == 0) {
            return null;
        }
        return certificates;
    }

    /**
     * Read NameIDFormats from metadata
     *
     * @param idpSSOEle
     * @return
     */
    private static List<String> parseNameIDFormats(Element idpSSOEle) {
        NodeList nodes = idpSSOEle.getElementsByTagName(SAMLNames.NAMEIDFORMAT);
        if (nodes.getLength() == 0) {
            return null;
        }

        List<String> nameIdFormats = new ArrayList<String>();
        for (int i = 0; i < nodes.getLength(); i++) {

            nameIdFormats.add(nodes.item(i).getTextContent());
        }

        if (nameIdFormats.size() == 0) {
            return null;
        }
        return nameIdFormats;
    }

    /**
     * Read Single Logout Services from metadata
     *
     * @param idpSSOEle
     * @return
     */
    private static List<SingleLogoutService> parseSloServices(Element idpSSOEle) {
        List<SingleLogoutService> sloServices = new ArrayList<SingleLogoutService>();
        NodeList nodes = idpSSOEle
                .getElementsByTagName(SAMLNames.SINGLELOGOUTSERVICE);
        for (int i = 0; i < nodes.getLength(); i++) {
            Element ele = (Element) nodes.item(i);
            sloServices.add(new SingleLogoutService(ele
                    .getAttribute(SAMLNames.LOCATION), ele
                    .getAttribute(SAMLNames.BINDING)));
        }
        return sloServices;
    }

    /**
     * Read Single SignOn Services from metadata
     *
     * @param idpSSOEle
     * @return
     */
    private static List<SingleSignOnService> parseSsoServices(Element idpSSOEle) {
        NodeList nodes = idpSSOEle
                .getElementsByTagName(SAMLNames.SINGLESIGNONSERVICE);
        if (nodes.getLength() == 0) {
            return null;
        }

        List<SingleSignOnService> ssoServices = new ArrayList<SingleSignOnService>();
        for (int i = 0; i < nodes.getLength(); i++) {
            Element ele = (Element) nodes.item(i);
            ssoServices.add(new SingleSignOnService(ele
                    .getAttribute(SAMLNames.LOCATION), ele
                    .getAttribute(SAMLNames.BINDING)));
        }

        if (ssoServices.size() == 0) {
            return null;
        }
        return ssoServices;
    }

    /**
     * Check if the element has expired - helper method
     *
     * @param entityEle
     * @return
     * @throws ParseException
     */
    private static boolean isExpired(Element entityEle) throws ParseException {
        String expDateStr = entityEle.getAttribute(SAMLNames.VALIDUNTIL);
        if (expDateStr.isEmpty()) {
            return false;
        }

        TimeZone timeZone = TimeZone.getTimeZone(SAMLNames.UTC);
        Calendar cal = Calendar.getInstance(timeZone);
        Date curDate = cal.getTime();

        SimpleDateFormat df = new SimpleDateFormat(SAMLNames.DATE_FORMAT);
        df.setTimeZone(timeZone);

        Date expDate = df.parse(expDateStr);
        if (expDate.before(curDate)) {
            return true;
        }
        return false;
    }

    /**
     * Helper method to get local host name
     *
     * @return
     * @throws Exception
     */
    public static String getHostName() throws Exception {
        return InetAddress.getLocalHost().getCanonicalHostName();
    }

    /**
     * Helper method to get host name by IP
     *
     * @return
     * @throws Exception
     */
    public static String getHostName(String ipAddress) throws Exception {
        return InetAddress.getByName(ipAddress).getCanonicalHostName();
    }

    /**
     * Method to convert Document to String
     *
     * @param doc
     * @return
     */
    private static String getStringFromDocument(Document doc) {
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            Element e = doc.getDocumentElement();
            e.normalize();
            prettyPrint(e, baos);
            return baos.toString();
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Return a cookie value or default
     *
     * @param cookies
     * @param cookieName
     * @param defaultValue
     * @return
     */
    private static String getCookieValue(Cookie[] cookies, String cookieName,
            String defaultValue) {
        if (cookies != null) {
            for (int i = 0; i < cookies.length; i++) {
                Cookie cookie = cookies[i];
                if (cookieName.equals(cookie.getName())) {
                    return (cookie.getValue());
                }
            }
        }
        return (defaultValue);
    }

    /**
     * Print out XML node to a stream
     *
     * @param xml
     * @param out
     * @throws TransformerConfigurationException
     * @throws TransformerFactoryConfigurationError
     * @throws TransformerException
     * @throws UnsupportedEncodingException
     */
    private static final void prettyPrint(Node xml, OutputStream out)
            throws Exception {
        TransformerFactory tFactory = TransformerFactory.newInstance();
        // tFactory.setAttribute("indent-number", 4);
        Transformer tf = tFactory.newTransformer();
        tf.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
        tf.setOutputProperty(OutputKeys.ENCODING, "UTF-8");
        tf.setOutputProperty(OutputKeys.INDENT, "yes");
        tf.setOutputProperty(OutputKeys.METHOD, "xml");
        tf.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "5");
        StreamResult result = new StreamResult(new OutputStreamWriter(out,
                "UTF-8"));
        tf.transform(new DOMSource(xml), result);
    }

    /**
     * Send HTTP response
     *
     * @param response
     * @param contentType
     * @param str
     * @throws IOException
     */
    public static void sendResponse(HttpServletResponse response,
            String contentType, String str) throws IOException {
        response.setContentType(contentType);
        PrintWriter out = response.getWriter();
        out.println(str);
        out.close();
    }
}

    class SampleTrustStrategy implements TrustStrategy {
        private static final String DEFAULT_TRUST_MANAGER_ALGO = "PKIX";
        private X509TrustManager certTrustManager = null;

        public SampleTrustStrategy(X509Certificate[] trustedCerts) throws KeyStoreException, NoSuchAlgorithmException,
                CertificateException, IOException {
            String stsSSLAlias = "stsSSL";
            KeyStore keyStore = ComponentUtils.getKeyStore();
            for (int i = 0; i < trustedCerts.length; i++) {
                if (!keyStore.containsAlias(stsSSLAlias + i))
                    keyStore.setCertificateEntry(stsSSLAlias + i, trustedCerts[i]);
            }

            TrustManagerFactory factory;

            factory = TrustManagerFactory.getInstance(DEFAULT_TRUST_MANAGER_ALGO);
            factory.init(keyStore);

            for (TrustManager trustManager : factory.getTrustManagers()) {
                if (trustManager instanceof X509TrustManager) {
                    certTrustManager = (X509TrustManager) trustManager;
                    break;
                }
            }
        }

        @Override
        public boolean isTrusted(X509Certificate[] certs, String authType) throws CertificateException {
            if (certTrustManager != null)
                certTrustManager.checkServerTrusted(certs, authType);

            return true;
        }

    }

    class NullResolver implements EntityResolver {

        @Override
        public InputSource resolveEntity(String publicId, String systemId)
                throws SAXException, IOException {
            return new InputSource(new StringReader(""));
        }
    }
