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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.TransformerFactoryConfigurationError;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.apache.commons.lang.Validate;
import org.opensaml.Configuration;
import org.opensaml.DefaultBootstrap;
import org.opensaml.common.SAMLVersion;
import org.opensaml.xml.ConfigurationException;
import org.opensaml.xml.schema.XSString;
import org.opensaml.xml.schema.impl.XSStringMarshaller;
import org.opensaml.xml.schema.impl.XSStringUnmarshaller;
import org.opensaml.xml.util.Base64;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import com.vmware.identity.saml.ext.DelegableType;
import com.vmware.identity.saml.ext.RenewableType;
import com.vmware.identity.saml.ext.impl.DelegableTypeBuilder;
import com.vmware.identity.saml.ext.impl.DelegableTypeMarshaller;
import com.vmware.identity.saml.ext.impl.DelegableTypeUnmarshaller;
import com.vmware.identity.saml.ext.impl.RenewableTypeBuilder;
import com.vmware.identity.saml.ext.impl.RenewableTypeMarshaller;
import com.vmware.identity.saml.ext.impl.RenewableTypeUnmarshaller;
import com.vmware.identity.saml.ext.impl.XSNonTrimmingStringBuilder;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;

/**
 * Shared constants and code go here
 *
 */
public final class Shared {
    public static final String IDM_HOSTNAME = "localhost"; // where IDM server
                                                           // is
    public static final String PASSWORD_ENTRY = "passwordEntry=1";
    public static final String PASSWORD_SUPPLIED = "passwordSupplied=1";

    public static final String REQUEST_AUTH_HEADER = "CastleAuthorization";
    public static final String KERB_AUTH_PREFIX = "Negotiate";
    public static final String PASSWORD_AUTH_PREFIX = "Basic";
    public static final String TLSCLIENT_AUTH_PREFIX = "TLSClient";
    public static final String RSAAM_AUTH_PREFIX = "RSAAM";
    public static final String RELYINGPARTY_ENTITYID = "RelyingPartyEntityId";
    public static final String IDP_SELECTION_HEADER = "CastleIDPSelection";

    public static final String RESPONSE_ERROR_HEADER = "CastleError";

    public static final String RESPONSE_AUTH_HEADER = "CastleAuthorization";
    public static final String SESSION_COOKIE_NAME = "CastleSession";
    public static final String LOGOUT_SESSION_COOKIE_NAME = "CastleLoggedOut";
    public static final String TENANT_IDP_COOKIE_NAME = "CastleIDPId";
    public static final int TOKEN_LIFETIME_MINUTES = 15;
    public static final int SESSION_LIFETIME_MINUTES = 480;
    public static final int NOTBEFORE_ADJUSTMENT_SECONDS = 150;

    public static final SAMLVersion REQUIRED_SAML_VERSION = SAMLVersion.VERSION_20;

    public static final String SAML_REQUEST_PARAMETER = "SAMLRequest";
    public static final String SAML_RESPONSE_PARAMETER = "SAMLResponse";
    public static final String RELAY_STATE_PARAMETER = "RelayState";
    public static final String SIGNATURE_ALGORITHM_PARAMETER = "SigAlg";
    public static final String SIGNATURE_PARAMETER = "Signature";

    public static final String HTML_CONTENT_TYPE = "text/html";
    public static final String METADATA_CONTENT_TYPE = "application/samlmetadata+xml";

    public final static String ssoCACEndpoint = "/websso/SAML2/SSOCAC";
    public final static String ssoEndpoint = "/websso/SAML2/SSO";

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
     * Base64 encode byte array
     *
     * @param bytesToEncode
     * @return
     */
    public static String encodeBytes(byte[] bytesToEncode) {
        String retval = Base64.encodeBytes(bytesToEncode,
                Base64.DONT_BREAK_LINES);
        return retval;
    }

    /**
     * Encode a string
     *
     * @param stringToEncode
     * @return
     */
    public static String encodeString(String stringToEncode) throws UnsupportedEncodingException {
        return Shared.encodeBytes(stringToEncode.getBytes("UTF-8"));
    }

    /**
     * Decode a string
     *
     * @param stringToDecode
     * @return
     */
    public static String decodeString(String stringToDecode) throws UnsupportedEncodingException {
        return new String(Base64.decode(stringToDecode), "UTF-8");
    }

    /**
     * Utility method to get complete request URL
     *
     * @param req
     * @return
     */
    public static String getUrl(HttpServletRequest req) {
        String reqUrl = req.getRequestURL().toString();
        String queryString = req.getQueryString(); // d=789
        if (queryString != null) {
            reqUrl += "?" + queryString;
        }
        return reqUrl;
    }

    /**
     * Utility method to check if session cookie is present
     *
     * @param req
     * @param sessionManager
     * @param tenant
     * @return
     */
    public static boolean hasSessionCookie(HttpServletRequest req,
            SessionManager sessionManager, String tenant) {
        boolean retval = false;

        if (req != null) {
            String sessionId = getCookieValue(req.getCookies(),
                    Shared.getTenantSessionCookieName(tenant), null);
            // the reason for session manager check is it possible for someone to
            // log out the user from a different process
            // (browser gets a cookie, and test tool sends a logout, or bad guy guesses session id and logs user out)
            // In that scenario, browser still has the cookie which will affect login page logic unless
            // we check server side session as well.
            if (sessionId != null && sessionManager != null) {
               Session session = sessionManager.get(sessionId);
               if (session != null && session.isValid())
                   retval = true;
            }
        }

        return retval;
    }

    /**
     * Return default tenant name
     *
     * @return
     */
    public static String getDefaultTenant() {
        DefaultIdmAccessorFactory factory = new DefaultIdmAccessorFactory();
        Validate.notNull(factory);
        IdmAccessor accessor = factory.getIdmAccessor();
        Validate.notNull(accessor);
        accessor.setDefaultTenant();
        return accessor.getTenant();
    }

    /**
     * Method to convert Document to String
     *
     * @param doc
     * @return
     */
    public static String getStringFromDocument(Document doc) {
        try {
            return getStringFromElement(doc.getDocumentElement());
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Method to convert Element to String
     *
     * @param e
     * @return
     */
    public static String getStringFromElement(Element e) {
        try {
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            e.normalize();
            prettyPrint(e, baos);
            return baos.toString("UTF-8");
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Send HTTP response
     * @param response
     * @param contentType
     * @param str
     * @throws IOException
     */
    public static void sendResponse(HttpServletResponse response, String contentType, String str)
            throws IOException {
        response.setContentType(contentType);
        PrintWriter out = response.getWriter();
        out.println(str);
        out.close();
    }

    /**
     * Bootstrap OpenSAML
     * @throws ConfigurationException
     */
    public static void bootstrap() throws ConfigurationException {
        DefaultBootstrap.bootstrap();
        org.opensaml.xml.Configuration.registerObjectProvider(XSString.TYPE_NAME,
                new XSNonTrimmingStringBuilder(),
                new XSStringMarshaller(),
                new XSStringUnmarshaller()
             );
        Configuration.registerObjectProvider(RenewableType.TYPE_NAME,
           new RenewableTypeBuilder(),
           new RenewableTypeMarshaller(),
           new RenewableTypeUnmarshaller());
        Configuration.registerObjectProvider(DelegableType.TYPE_NAME,
                new DelegableTypeBuilder(),
                new DelegableTypeMarshaller(),
                new DelegableTypeUnmarshaller());
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
     * Return a cookie value or default
     * @param cookies
     * @param cookieName
     * @param defaultValue
     * @return
     */
    public static String getCookieValue(Cookie[] cookies, String cookieName,
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
     * Return a logout cookie name
     * @param nameSuffix
     * @return */
    public static String getLogoutCookieName(String nameSuffix)
    {
        return Shared.LOGOUT_SESSION_COOKIE_NAME + nameSuffix;
    }

    /**
     * Return a cookie name
     * @param nameSuffix
     * @return */
    public static String getTenantSessionCookieName(String nameSuffix)
    {
        return Shared.SESSION_COOKIE_NAME+nameSuffix;
    }

    public static String getTenantIDPCookieName(String nameSuffix)
    {
        return Shared.TENANT_IDP_COOKIE_NAME + nameSuffix;
    }
}
