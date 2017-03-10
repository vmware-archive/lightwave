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

package com.vmware.identity.openidconnect.server;

import java.net.URI;
import java.util.Locale;
import java.util.Set;

import javax.servlet.http.Cookie;

import org.apache.commons.lang3.StringEscapeUtils;
import org.apache.commons.lang3.tuple.Pair;
import org.apache.commons.lang3.tuple.Triple;
import org.springframework.context.MessageSource;
import org.springframework.ui.Model;
import org.springframework.web.servlet.ModelAndView;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ResponseTypeValue;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.protocol.AccessToken;
import com.vmware.identity.openidconnect.protocol.AuthenticationErrorResponse;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest;
import com.vmware.identity.openidconnect.protocol.AuthenticationSuccessResponse;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.IDToken;

/**
 * @author Yehia Zayour
 */
public class AuthenticationRequestProcessor {
    private static final long CLIENT_ASSERTION_LIFETIME_MS = 50 * 60 * 1000L; // 50 minutes, to allow for user to get login form, then login later

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(AuthenticationRequestProcessor.class);

    private final TenantInfoRetriever tenantInfoRetriever;
    private final ClientInfoRetriever clientInfoRetriever;
    private final ServerInfoRetriever serverInfoRetriever;
    private final UserInfoRetriever userInfoRetriever;
    private final PersonUserAuthenticator personUserAuthenticator;
    private final SolutionUserAuthenticator solutionUserAuthenticator;

    private final AuthorizationCodeManager authzCodeManager;
    private final SessionManager sessionManager;
    private final MessageSource messageSource;
    private final Model model;
    private final Locale locale;
    private final HttpRequest httpRequest;
    private String tenant;

    private final boolean isAjaxRequest;

    private TenantInfo tenantInfo;
    private ClientInfo clientInfo;
    private AuthenticationRequest authnRequest;

    public AuthenticationRequestProcessor(
            CasIdmClient idmClient,
            AuthorizationCodeManager authzCodeManager,
            SessionManager sessionManager,
            MessageSource messageSource,
            Model model,
            Locale locale,
            HttpRequest httpRequest,
            String tenant) {
        this.tenantInfoRetriever = new TenantInfoRetriever(idmClient);
        this.clientInfoRetriever = new ClientInfoRetriever(idmClient);
        this.serverInfoRetriever = new ServerInfoRetriever(idmClient);
        this.userInfoRetriever = new UserInfoRetriever(idmClient);
        this.personUserAuthenticator = new PersonUserAuthenticator(idmClient);
        this.solutionUserAuthenticator = new SolutionUserAuthenticator(idmClient);

        this.authzCodeManager = authzCodeManager;
        this.sessionManager = sessionManager;
        this.messageSource = messageSource;
        this.model = model;
        this.locale = locale;
        this.httpRequest = httpRequest;
        this.tenant = tenant;

        this.isAjaxRequest = (this.httpRequest.getMethod() == HttpRequest.Method.POST);

        this.tenantInfo = null;
        this.clientInfo = null;
        this.authnRequest = null;
    }

    public Pair<ModelAndView, HttpResponse> process() {
        // parse authn request (and if that fails, see if you can still construct an error response off of the partially parsed request)
        ClientID clientId;
        URI redirectUri;
        AuthenticationErrorResponse parseErrorResponse;
        try {
            this.authnRequest = AuthenticationRequest.parse(this.httpRequest);
            clientId = this.authnRequest.getClientID();
            redirectUri = this.authnRequest.getRedirectURI();
            parseErrorResponse = null;
        } catch (AuthenticationRequest.ParseException e) {
            if (e.getClientID() != null && e.getRedirectURI() != null && e.createAuthenticationErrorResponse(this.isAjaxRequest) != null) {
                clientId = e.getClientID();
                redirectUri = e.getRedirectURI();
                parseErrorResponse = e.createAuthenticationErrorResponse(this.isAjaxRequest);
            } else {
                LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
                return Pair.of((ModelAndView) null, HttpResponse.createErrorResponse(e.getErrorObject()));
            }
        }

        // check that tenant, client, and redirect_uri are registered (if not, return error to browser, not client)
        try {
            this.tenantInfo = this.tenantInfoRetriever.retrieveTenantInfo(this.tenant);
            this.tenant = this.tenantInfo.getName(); // use tenant name as it appears in directory
            this.clientInfo = this.clientInfoRetriever.retrieveClientInfo(this.tenant, clientId);
            if (!this.clientInfo.getRedirectURIs().contains(redirectUri)) {
                throw new ServerException(ErrorObject.invalidRequest("unregistered redirect_uri"));
            }
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            return Pair.of((ModelAndView) null, HttpResponse.createErrorResponse(e.getErrorObject()));
        }

        // if tenant, client, and redirect_uri are registered, we can return authn error response to the client instead of browser
        if (parseErrorResponse != null) {
            LoggerUtils.logFailedRequest(logger, parseErrorResponse.getErrorObject());
            return Pair.of((ModelAndView) null, parseErrorResponse.toHttpResponse());
        }

        // authenticate client
        try {
            authenticateClient();
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            return Pair.of((ModelAndView) null, authnErrorResponse(e).toHttpResponse());
        }

        // process login information (username/password or session), if failed, return error message to browser so javascript can consume it
        LoginProcessor p = new LoginProcessor(
                this.personUserAuthenticator,
                this.sessionManager,
                this.messageSource,
                this.locale,
                this.httpRequest,
                this.tenant);
        Triple<PersonUser, SessionID, LoginMethod> loginResult;
        try {
            loginResult = p.process();
        } catch (LoginProcessor.LoginException e) {
            LoggerUtils.logFailedRequest(logger, e.getErrorObject(), e);
            return Pair.of((ModelAndView) null, e.toHttpResponse());
        }
        PersonUser personUser   = loginResult.getLeft();
        SessionID sessionId     = loginResult.getMiddle();
        LoginMethod loginMethod = loginResult.getRight();

        // if no person user, return login form
        if (personUser == null) {
            try {
                ModelAndView loginForm = generateLoginForm();
                logger.info("login form generated");
                return Pair.of(loginForm, (HttpResponse) null);
            } catch (ServerException e) {
                LoggerUtils.logFailedRequest(logger, e);
                return Pair.of((ModelAndView) null, authnErrorResponse(e).toHttpResponse());
            }
        }

        // we have a person user, process authn request for this person user
        try {
            AuthenticationSuccessResponse authnSuccessResponse = (this.authnRequest.getResponseType().contains(ResponseTypeValue.AUTHORIZATION_CODE)) ?
                    processAuthzCodeResponse(personUser, sessionId) :
                    processIDTokenResponse(personUser, sessionId);
            if (loginMethod == null) {
                this.sessionManager.update(sessionId, this.clientInfo);
            } else {
                this.sessionManager.add(sessionId, personUser, loginMethod, this.clientInfo);
            }
            HttpResponse httpResponse = authnSuccessResponse.toHttpResponse();
            httpResponse.addCookie(loggedInSessionCookie(sessionId));
            logger.info(
                    "subject [{}] response_type [{}] response_mode [{}] login_method [{}]",
                    personUser.getSubject().getValue(),
                    this.authnRequest.getResponseType().toString(),
                    this.authnRequest.getResponseMode().getValue(),
                    (loginMethod == null) ? "session" : loginMethod.getValue());
            return Pair.of((ModelAndView) null, httpResponse);
        } catch (ServerException e) {
            LoggerUtils.logFailedRequest(logger, e);
            return Pair.of((ModelAndView) null, authnErrorResponse(e).toHttpResponse());
        }
    }

    private AuthenticationSuccessResponse processAuthzCodeResponse(PersonUser personUser, SessionID sessionId) {
        AuthorizationCode authzCode = new AuthorizationCode();

        this.authzCodeManager.add(
                authzCode,
                personUser,
                sessionId,
                this.authnRequest);

        return new AuthenticationSuccessResponse(
                this.authnRequest.getResponseMode(),
                this.authnRequest.getRedirectURI(),
                this.authnRequest.getState(),
                this.isAjaxRequest,
                authzCode,
                (IDToken) null,
                (AccessToken) null);
    }

    private AuthenticationSuccessResponse processIDTokenResponse(PersonUser personUser, SessionID sessionId) throws ServerException {
        Set<ResourceServerInfo> resourceServerInfos = this.serverInfoRetriever.retrieveResourceServerInfos(this.tenant, this.authnRequest.getScope());
        UserInfo userInfo = this.userInfoRetriever.retrieveUserInfo(personUser, this.authnRequest.getScope(), resourceServerInfos);

        TokenIssuer tokenIssuer = new TokenIssuer(
                personUser,
                (SolutionUser) null,
                userInfo,
                this.tenantInfo,
                this.authnRequest.getScope(),
                this.authnRequest.getNonce(),
                this.authnRequest.getClientID(),
                sessionId);

        IDToken idToken = tokenIssuer.issueIDToken();
        AccessToken accessToken = null;
        if (this.authnRequest.getResponseType().contains(ResponseTypeValue.ACCESS_TOKEN)) {
            accessToken = tokenIssuer.issueAccessToken();
        }

        return new AuthenticationSuccessResponse(
                this.authnRequest.getResponseMode(),
                this.authnRequest.getRedirectURI(),
                this.authnRequest.getState(),
                this.isAjaxRequest,
                (AuthorizationCode) null,
                idToken,
                accessToken);
    }

    private void authenticateClient() throws ServerException {
        if (this.clientInfo.getCertSubjectDN() == null) {
            return; // no client authentication since no cert was specified at client registration time
        }

        if (this.authnRequest.getClientAssertion() == null) {
            throw new ServerException(ErrorObject.invalidClient("client_assertion parameter is required since client has registered a cert"));
        }

        long clientAssertionLifetimeMs = (this.clientInfo.getAuthnRequestClientAssertionLifetimeMS() > 0) ?
                this.clientInfo.getAuthnRequestClientAssertionLifetimeMS() :
                CLIENT_ASSERTION_LIFETIME_MS;

        this.solutionUserAuthenticator.authenticateByClientAssertion(
                this.authnRequest.getClientAssertion(),
                clientAssertionLifetimeMs,
                Endpoints.normalizeAuthenticationRequestURI(this.httpRequest.getURI()),
                this.tenantInfo,
                this.clientInfo);
    }

    private AuthenticationErrorResponse authnErrorResponse(ServerException e) {
        return new AuthenticationErrorResponse(
                this.authnRequest.getResponseMode(),
                this.authnRequest.getRedirectURI(),
                this.authnRequest.getState(),
                this.isAjaxRequest,
                e.getErrorObject());
    }

    private Cookie loggedInSessionCookie(SessionID sessionId) {
        Cookie cookie = new Cookie(SessionManager.getSessionCookieName(this.tenant), sessionId.getValue());
        cookie.setPath(Endpoints.BASE);
        cookie.setSecure(true);
        cookie.setHttpOnly(true);
        return cookie;
    }

    private String localize(String key) {
        return this.messageSource.getMessage(key, (Object[]) null, this.locale);
    }

    private ModelAndView generateLoginForm() throws ServerException {
        AuthorizationServerInfo authzServerInfo = this.serverInfoRetriever.retrieveAuthorizationServerInfo();

        this.model.addAttribute("spn",                          StringEscapeUtils.escapeEcmaScript(authzServerInfo.getServicePrincipalName()));

        this.model.addAttribute("protocol",                     "openidconnect");
        this.model.addAttribute("cac_endpoint",                 Endpoints.BASE + Endpoints.authenticationCacForLoginForm(this.httpRequest.getURI()));
        this.model.addAttribute("sso_endpoint",                 Endpoints.BASE + Endpoints.AUTHENTICATION);
        this.model.addAttribute("responseMode",                 this.authnRequest.getResponseMode().getValue());

        this.model.addAttribute("tenant",                       this.tenant);
        this.model.addAttribute("tenant_brandname",             StringEscapeUtils.escapeEcmaScript(this.tenantInfo.getBrandName()));
        this.model.addAttribute("tenant_logonbanner_title",     StringEscapeUtils.escapeEcmaScript(this.tenantInfo.getLogonBannerTitle()));
        this.model.addAttribute("tenant_logonbanner_content",   StringEscapeUtils.escapeEcmaScript(this.tenantInfo.getLogonBannerContent()));
        this.model.addAttribute("enable_logonbanner_checkbox",  this.tenantInfo.getLogonBannerEnableCheckbox());

        this.model.addAttribute("username_placeholder",         localize("LoginForm.UserName.Placeholder"));
        this.model.addAttribute("username",                     localize("LoginForm.UserName"));
        this.model.addAttribute("password",                     localize("LoginForm.Password"));
        this.model.addAttribute("passcode",                     localize("LoginForm.Passcode"));
        this.model.addAttribute("submit",                       localize("LoginForm.Submit"));
        this.model.addAttribute("error",                        localize("LoginForm.Error"));
        this.model.addAttribute("login",                        localize("LoginForm.Login"));
        this.model.addAttribute("help",                         localize("LoginForm.Help"));
        this.model.addAttribute("winSession",                   localize("LoginForm.WinSession"));
        this.model.addAttribute("downloadCIP",                  localize("LoginForm.DownloadCIP"));
        this.model.addAttribute("unsupportedBrowserWarning",    localize("LoginForm.UnsupportedBrowserWarning"));
        this.model.addAttribute("smartcard",                    localize("LoginForm.Smartcard"));
        this.model.addAttribute("iAgreeTo",                     localize("LoginForm.IAgreeTo"));
        this.model.addAttribute("logonBannerAlertMessage",      localize("LoginForm.LogonBannerAlertMessage"));
        this.model.addAttribute("rsaam",                        localize("LoginForm.RsaSecurID"));

        this.model.addAttribute("enable_password_auth",         this.tenantInfo.getAuthnPolicy().isPasswordAuthnEnabled());
        this.model.addAttribute("enable_windows_auth",          this.tenantInfo.getAuthnPolicy().isWindowsAuthnEnabled());
        this.model.addAttribute("enable_tlsclient_auth",        this.tenantInfo.getAuthnPolicy().isClientCertAuthnEnabled());
        this.model.addAttribute("enable_rsaam_auth",            this.tenantInfo.getAuthnPolicy().isSecurIDAuthnEnabled());

        if (this.tenantInfo.getAuthnPolicy().isSecurIDAuthnEnabled()) {
            this.model.addAttribute("rsaam_reminder", StringEscapeUtils.escapeEcmaScript(this.tenantInfo.getAuthnPolicy().getSecurIDLoginGuide()));
        }

        return new ModelAndView("unpentry");
    }
}
