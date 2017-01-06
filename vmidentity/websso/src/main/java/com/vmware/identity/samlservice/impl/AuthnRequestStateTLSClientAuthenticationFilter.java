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
package com.vmware.identity.samlservice.impl;

import java.io.ByteArrayInputStream;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.AuthnRequest;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opensaml.xml.util.Base64;

import com.vmware.identity.SecurityRequestWrapper;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.idm.IdmPrincipalAttributesExtractorFactory;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.WebSSOError;


public class AuthnRequestStateTLSClientAuthenticationFilter implements
AuthenticationFilter<AuthnRequestState> {
    private static final Logger log = LoggerFactory
            .getLogger(AuthnRequestStateTLSClientAuthenticationFilter.class);

    private static final String clientCertHeader = "X-SSL-Client-Certificate";
    private static final String clientCertAttributeName = "javax.servlet.request.X509Certificate";

    @Override
    public void preAuthenticate(AuthnRequestState t) throws SamlServiceException {
        log.debug("AuthnRequestStateTLSClientAuthenticationFilter.preAuthenticate is called");

        Validate.notNull(t);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        IdmAccessor accessor = t.getIdmAccessor();
        Validate.notNull(accessor);

        // then check if required auth header is present
        if (request.getParameter(Shared.REQUEST_AUTH_PARAM) == null) {
            // authentication not possible
            log.debug(Shared.REQUEST_AUTH_PARAM + " is missing, requesting "
                            + Shared.TLSCLIENT_AUTH_PREFIX);
            t.setWwwAuthenticate(Shared.TLSCLIENT_AUTH_PREFIX);
            ValidationResult vr = new ValidationResult(HttpServletResponse.SC_UNAUTHORIZED,
                            WebSSOError.BAD_REQUEST, null);
            t.setValidationResult(vr);
            throw new SamlServiceException();
        }

        // check if logout cookie is present
        Cookie[] cookies = request.getCookies();
        String logoutCookieName = Shared.getLogoutCookieName(accessor.getTenant());
        if (cookies != null && cookies.length > 0) {
            for (Cookie cookie : cookies) {
                if (cookie.getName().equalsIgnoreCase(logoutCookieName)) {
                    ValidationResult vr = new ValidationResult(HttpServletResponse.SC_BAD_REQUEST,
                            WebSSOError.UNAUTHORIZED, WebSSOError.LOGGED_OUT_TLS_SESSION);
                    t.setValidationResult(vr);
                    throw new SamlServiceException();
                }
            }
        }
    }

	    @Override
    public void authenticate(AuthnRequestState t) throws SamlServiceException {
        log.debug("AuthnRequestStateTLSClientAuthenticationFilter.authenticate is called");

        Validate.notNull(t);
        IdmAccessor accessor = t.getIdmAccessor();
        Validate.notNull(accessor);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        AuthnRequest authnRequest = t.getAuthnRequest();
        Validate.notNull(authnRequest);

        PrincipalId principalId = null;


        X509Certificate certChain[] = null;

        //Get from the custom header first.
        String certStr = request
                        .getHeader(AuthnRequestStateTLSClientAuthenticationFilter.clientCertHeader);

        if (certStr != null && certStr.length() > 0
                        && request.getAuthType() == SecurityRequestWrapper.VMWARE_CLIENT_CERT_AUTH) {
            ByteArrayInputStream bais = null;
            CertificateFactory cf;
            try {
                cf = CertificateFactory.getInstance("X.509");
                bais = new ByteArrayInputStream(Base64.decode(certStr));
                X509Certificate cert = (X509Certificate) cf.generateCertificate(bais);
                certChain = new X509Certificate[] { cert };
            } catch (CertificateException e1) {
                log.error("Error reading client certificate from http header. ", e1);
                ValidationResult vr = new ValidationResult(HttpServletResponse.SC_UNAUTHORIZED,
                                WebSSOError.UNAUTHORIZED, WebSSOError.INVALID_CREDENTIAL);
                t.setValidationResult(vr);
                throw new SamlServiceException("Client Certificate error.", e1);
            }
        }

        // Get from standard place of sl client cert location
        if (certChain == null || certChain.length == 0) {
            certChain = (X509Certificate[]) request
                            .getAttribute(AuthnRequestStateTLSClientAuthenticationFilter.clientCertAttributeName);
        }

        if (certChain == null || certChain.length == 0) {
            ValidationResult vr = new ValidationResult(HttpServletResponse.SC_UNAUTHORIZED,
                            WebSSOError.UNAUTHORIZED, WebSSOError.NO_CLIENT_CERT);
            t.setValidationResult(vr);

        } else {
            try {
                // extract auth data
                String certAuthParam = request.getParameter(Shared.REQUEST_AUTH_PARAM);
                Validate.notNull(certAuthParam);
                certAuthParam = certAuthParam.replace(Shared.TLSCLIENT_AUTH_PREFIX, "").trim();

                Validate.notNull(certAuthParam);
                byte[] decodedAuthData = Base64.decode(certAuthParam);

                String unp = new String(decodedAuthData, "UTF-8");
                int idx = unp.indexOf(':');
                String hintAndDomain = idx > 0 ? unp.substring(0, idx) : null;

                principalId = accessor.authenticate(certChain,hintAndDomain);
                Validate.notNull(principalId, "principalId");
            } catch (Exception ex) {
                // could not authenticate with the certificate
                ValidationResult vr = new ValidationResult(HttpServletResponse.SC_UNAUTHORIZED,
                                WebSSOError.UNAUTHORIZED, WebSSOError.INVALID_CREDENTIAL);
                t.setValidationResult(vr);
            }
        }

        if (principalId != null) {
            t.setPrincipalId(principalId);
            t.setAuthnMethod(AuthnMethod.TLSCLIENT);
        }
    }

    @Override
	public PrincipalAttributesExtractorFactory getPrincipalAttributeExtractorFactory(
            String idmHostName) {
        return new IdmPrincipalAttributesExtractorFactory(idmHostName);
    }

    @Override
	public ConfigExtractorFactory getConfigExtractorFactory(String idmHostName) {
        return new ConfigExtractorFactoryImpl();
    }

    @Override
	public void preProcess(AuthnRequestState t) throws SamlServiceException {
        preAuthenticate(t);
    }

    @Override
	public void process(AuthnRequestState t) throws SamlServiceException {
        authenticate(t);
    }


}
