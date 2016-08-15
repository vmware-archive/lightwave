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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.idm.IdmPrincipalAttributesExtractorFactory;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;

/**
 * Authentication by cookie (this authenticator wraps another authenticator, for example Kerb or Password).
 * We will fall back to that authenticator if session cookie is not found.
 *
 */
public class AuthnRequestStateCookieWrapper implements AuthenticationFilter<AuthnRequestState> {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(AuthnRequestStateCookieWrapper.class);
    private AuthenticationFilter<AuthnRequestState> authenticator;
    private AuthnRequestStateExternalAuthenticationFilter externalAuthenticator;

    /**
     * Empty constructor
     */
    public AuthnRequestStateCookieWrapper() {}

    /**
     * Initialize embedded authenticator
     * @param authenticator
     */
    public AuthnRequestStateCookieWrapper(AuthenticationFilter<AuthnRequestState> authenticator) {
        this.authenticator = authenticator;
    }

    /**
     * Initialize external authenticator
     * @param authenticator
     */
    public AuthnRequestStateCookieWrapper(AuthnRequestStateExternalAuthenticationFilter extAuthenticator) {
        this.externalAuthenticator = extAuthenticator;
    }

    @Override
    public void preAuthenticate(AuthnRequestState t)
            throws SamlServiceException {
        log.debug("AuthnRequestStateCookieWrapper.preAuthenticate is called");

        // check for session existence first
        Validate.notNull(t);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        SessionManager sessionManager = t.getSessionManager();
        Validate.notNull(sessionManager);

        Validate.notNull(t.getIdmAccessor());
        Session currentSession = Shared.getSession(sessionManager, request, t.getIdmAccessor().getTenant());
        if (currentSession == null) {
            if (this.getAuthenticator() != null) {
                // fall back to stored authenticator
                authenticator.preAuthenticate(t);
            } else {
                // will fall back to login page in authenticate()
            }
        }
    }

    @Override
    public void authenticate(AuthnRequestState t) throws SamlServiceException {
        log.debug("AuthnRequestStateCookieWrapper.authenticate is called");

        // check for session existence first
        Validate.notNull(t, "AuthnRequestState");
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request, "request");
        SessionManager sessionManager = t.getSessionManager();
        Validate.notNull(sessionManager, "sessionManager");
        Session currentSession = Shared.getSession(sessionManager, request,t.getIdmAccessor().getTenant());
        if (currentSession != null) {
            log.debug("Found existing session {}", currentSession);
            // use session data here to determine identity
            PrincipalId principalId = currentSession.getPrincipalId();
            Validate.notNull(principalId, "principalId");
            t.setPrincipalId(principalId);
            t.setAuthnMethod(currentSession.getAuthnMethod());
            t.setSessionId(currentSession.getId());

            //turning off the proxying flag since we will are using existing session for the user.
            if (t.isProxying())
                t.setProxying(false);
        } else if (this.getAuthenticator() != null || t.isProxying()) {
            // fall back to stored authenticator: currently it could be kerbros,pw, external.
            if (t.isProxying() && getExternalAuthenticator() != null)
                getExternalAuthenticator().authenticate(t);
            else if (!t.isProxying() && getAuthenticator() != null) {
                getAuthenticator().authenticate(t);
                t.createSession(null, null);
            }
            else {
                log.error("externel authenticator is not intialized! ");
                throw new SamlServiceException();
            }
        } else {
            // fall back to sending browser the login page.
            ValidationResult vr = new ValidationResult(
                    HttpServletResponse.SC_FOUND, null, null);
            t.setValidationResult(vr);
        }
    }

    @Override
    public PrincipalAttributesExtractorFactory getPrincipalAttributeExtractorFactory(
            String idmHostName) {
        if (this.getAuthenticator() != null) {
            return authenticator.getPrincipalAttributeExtractorFactory(idmHostName);
        } else {
            return new IdmPrincipalAttributesExtractorFactory(idmHostName);
        }
    }

    @Override
    public ConfigExtractorFactory getConfigExtractorFactory(String idmHostName) {
        if (this.getAuthenticator() != null) {
            return authenticator.getConfigExtractorFactory(idmHostName);
        } else {
            return new ConfigExtractorFactoryImpl();
        }
    }

    /**
     * @return the authenticator
     */
    public AuthenticationFilter<AuthnRequestState> getAuthenticator() {
        return authenticator;
    }

    /**
     * @param authenticator the authenticator to set
     */
    public void setAuthenticator(AuthenticationFilter<AuthnRequestState> authenticator) {
        this.authenticator = authenticator;
    }

    @Override
    public void preProcess(AuthnRequestState t) throws SamlServiceException {
        preAuthenticate(t);
    }

    @Override
    public void process(AuthnRequestState t) throws SamlServiceException {
        authenticate(t);
    }

    /**
     * @return the externalAuthenticator
     */
    public AuthnRequestStateExternalAuthenticationFilter getExternalAuthenticator() {
        return externalAuthenticator;
    }

    /**
     * @param externalAuthenticator the externalAuthenticator to set
     */
    public void setExternalAuthenticator(AuthnRequestStateExternalAuthenticationFilter externalAuthenticator) {
        this.externalAuthenticator = externalAuthenticator;
    }
}
