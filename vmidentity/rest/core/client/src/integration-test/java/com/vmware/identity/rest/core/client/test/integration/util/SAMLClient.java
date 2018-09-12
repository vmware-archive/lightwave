package com.vmware.identity.rest.core.client.test.integration.util;

import static com.vmware.identity.rest.core.client.URIFactory.buildURI;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.net.URI;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.SSLContext;
import javax.xml.soap.MessageFactory;
import javax.xml.soap.SOAPBody;
import javax.xml.soap.SOAPException;
import javax.xml.soap.SOAPHeader;
import javax.xml.soap.SOAPMessage;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.codec.binary.Base64;
import org.apache.http.HttpEntity;
import org.apache.http.HttpStatus;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.util.EntityUtils;
import org.w3c.dom.Node;

import com.vmware.identity.rest.core.client.BaseClient;
import com.vmware.identity.rest.core.client.exceptions.ClientException;

public class SAMLClient extends BaseClient {

    private static final String TOKEN_TENANT_URI_STRING = "/sts/STSService/%s";
    private static final String XML_DATE_FORMAT = "yyyy-MM-dd'T'HH:mm:ss.SSS'Z'";
    private static final String RST_FILE = "/soap_request.xml";
    private static final String CREATED_TAG = "wsu:Created";
    private static final String EXPIRES_TAG = "wsu:Expires";
    private static final String USERNAME_TAG = "wsse:Username";
    private static final String PASSWORD_TAG = "wsse:Password";
    private static final String SAML2_ASSERTION_TAG = "saml2:Assertion";
    private static final String SOAP_ACTION_ISSUE = "http://docs.oasis-open.org/ws-sx/ws-trust/200512/RST/Issue";
    private static final String GET_TOKEN_METHOD = "GetSamlToken";
    private static final String REQUEST_CONTENT_TYPE = "text/xml";

    public SAMLClient(String host, HostnameVerifier verifier, SSLContext sslContext) {
        super(host, verifier, sslContext);
    }

    public String getAccessToken(String tenant, String username, String password) throws SOAPException, IOException, ClientException {
        InputStream is = SAMLClient.class.getResourceAsStream(RST_FILE);
        SOAPMessage soapRequest = MessageFactory.newInstance().createMessage(null, is);
        SOAPHeader header = soapRequest.getSOAPHeader();
        SOAPBody body = soapRequest.getSOAPBody();

        DateFormat dateFormat = new SimpleDateFormat(XML_DATE_FORMAT);
        dateFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        final long now = System.currentTimeMillis();
        String createDate = dateFormat.format(new Date(now));
        String expireDate = dateFormat.format(new Date(now + TimeUnit.SECONDS.toMillis(600)));

        Node node = header.getElementsByTagName(CREATED_TAG).item(0);
        node.setTextContent(createDate);
        node = header.getElementsByTagName(EXPIRES_TAG).item(0);
        node.setTextContent(expireDate);
        node = header.getElementsByTagName(USERNAME_TAG).item(0);
        node.setTextContent(username);
        node = header.getElementsByTagName(PASSWORD_TAG).item(0);
        node.setTextContent(password);

        node = body.getElementsByTagName(CREATED_TAG).item(0);
        node.setTextContent(createDate);
        node = body.getElementsByTagName(EXPIRES_TAG).item(0);
        node.setTextContent(expireDate);

        URI uri = buildURI(getHostRetriever(), TOKEN_TENANT_URI_STRING, tenant);
        HttpPost post = new HttpPost(uri);
        post.setHeader("Content-Type", REQUEST_CONTENT_TYPE);
        post.setHeader("SOAPAction", SOAP_ACTION_ISSUE);
        post.setHeader("Method", GET_TOKEN_METHOD);
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        soapRequest.writeTo(out);
        String messageStr = new String(out.toByteArray());

        HttpEntity entity = new StringEntity(messageStr);
        post.setEntity(entity);

        String token;
        try (CloseableHttpResponse response = getClient().execute(post)) {
            int statusCode = response.getStatusLine().getStatusCode();
            if (statusCode == HttpStatus.SC_OK) {
                try {
                    SOAPMessage soapResponse = MessageFactory.newInstance().createMessage(null, response.getEntity().getContent());
                    Node tokenNode = soapResponse.getSOAPBody().getElementsByTagName(SAML2_ASSERTION_TAG).item(0);
                    StringWriter writer = new StringWriter();
                    Transformer transformer = TransformerFactory.newInstance().newTransformer();
                    transformer.transform(new DOMSource(tokenNode), new StreamResult(writer));
                    token = writer.toString();
                } catch (IOException | TransformerFactoryConfigurationError | TransformerException e) {
                    throw new IllegalStateException("An error occurred unmarshalling the response", e);
                }
            } else {
                String error = EntityUtils.toString(response.getEntity());
                throw new IllegalArgumentException("An error (" + statusCode + ") occurred when retrieving the access token from STS. " + error);
            }
        }

        return Base64.encodeBase64String(token.getBytes());
    }
 }
