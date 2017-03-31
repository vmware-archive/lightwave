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

package com.vmware.identity.openidconnect.sample;

import java.io.File;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.URI;
import java.net.UnknownHostException;
import java.security.KeyStore;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.Date;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang3.StringUtils;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.ssl.TrustStrategy;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.vmware.identity.openidconnect.client.AuthenticationCodeResponse;
import com.vmware.identity.openidconnect.client.AuthenticationTokensResponse;
import com.vmware.identity.openidconnect.client.ClientAuthenticationMethod;
import com.vmware.identity.openidconnect.client.ClientConfig;
import com.vmware.identity.openidconnect.client.ConnectionConfig;
import com.vmware.identity.openidconnect.client.GroupMembershipType;
import com.vmware.identity.openidconnect.client.HighAvailabilityConfig;
import com.vmware.identity.openidconnect.client.HolderOfKeyConfig;
import com.vmware.identity.openidconnect.client.IDToken;
import com.vmware.identity.openidconnect.client.ListenerHelper;
import com.vmware.identity.openidconnect.client.MetadataHelper;
import com.vmware.identity.openidconnect.client.OIDCClient;
import com.vmware.identity.openidconnect.client.OIDCClientException;
import com.vmware.identity.openidconnect.client.OIDCServerException;
import com.vmware.identity.openidconnect.client.OIDCTokens;
import com.vmware.identity.openidconnect.client.ResourceServerAccessToken;
import com.vmware.identity.openidconnect.client.SSLConnectionException;
import com.vmware.identity.openidconnect.client.TokenSpec;
import com.vmware.identity.openidconnect.client.TokenValidationException;
import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ProviderMetadata;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.ResponseType;
import com.vmware.identity.openidconnect.common.ResponseTypeValue;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.TokenType;
import com.vmware.identity.openidconnect.protocol.ServerIssuedToken;
import com.vmware.identity.rest.afd.client.AfdClient;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.provider.VecsLoadStoreParameter;

/**
 * @author Yehia Zayour
 * @author Jun Sun
 */
@Controller
public class RelyingPartyController {
    private static final String SESSION_COOKIE_NAME = "rp_session_id";
    private static final long CLOCK_TOLERANCE_SECONDS = 10 * 60L; // 10 mins

    private static boolean highAvailabilityEnabled;
    private static String tenantName;
    private static String localHostWithPort;
    private static ProviderMetadata providerMetadata;
    private static String clientId;
    private static RSAPublicKey providerRSAPublicKey;
    private static X509Certificate clientCertificate;
    private static String postLogoutRedirectUrl;
    private static String redirectEndpointUrlAuthzCodeFlowFormResponse;
    private static String redirectEndpointUrlAuthzCodeFlowQueryResponse;
    private static String redirectEndpointUrlImplicitFlowFormResponse;
    private static String redirectEndpointUrlImplicitFlowFragmentResponse;
    private static String logoutUrl;
    private static String rootUrl;

    private static boolean isInstalled;

    private static OIDCClient client;
    private static ClientConfig clientConfig;
    private static String[] resourceServers = {"rs_admin_server", "rs_some_other_resource_server"};

    private KeyStore keyStore;

    @Autowired
    private SessionManager sessionManager;

    @Autowired
    private AuthenticationRequestTracker authnRequestTracker;

    @Autowired
    private LogoutRequestTracker logoutRequestTracker;

    @Autowired
    private RelyingPartyConfig relyingPartyConfig;

    @RequestMapping(value = "/", method = RequestMethod.GET)
    public String root(
            HttpServletRequest request,
            HttpServletResponse response,
            ModelMap modelMap) {
        installIfNeeded();

        SessionID sessionId = getSessionID(request);
        OIDCTokens tokens = (sessionId != null) ? this.sessionManager.get(sessionId) : null;

        if (tokens != null) {
            modelMap.addAttribute("loggedIn", true);
            modelMap.addAttribute("displayText", String.format("welcome %s, you are now logged in!", getDisplayName(tokens.getIDToken())));
            modelMap.addAttribute("clientId", clientId);
            modelMap.addAttribute("id_token", tokens.getIDToken().serialize());
            modelMap.addAttribute("access_token", tokens.getAccessToken().getValue());
            modelMap.addAttribute("refresh_token", (tokens.getRefreshToken() == null) ? null : tokens.getRefreshToken().getValue());
        } else {
            modelMap.addAttribute("loggedIn", false);
            modelMap.addAttribute("displayText", "");
            modelMap.addAttribute("clientId", clientId);
        }

        return "resource";
    }

    @RequestMapping(value = "/login_authz_code_flow_form_response", method = RequestMethod.POST)
    public void loginAuthzCodeFlowFormResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException {
        login(request, response, ResponseType.authorizationCode(), ResponseMode.FORM_POST, URI.create(redirectEndpointUrlAuthzCodeFlowFormResponse));
    }

    @RequestMapping(value = "/login_authz_code_flow_query_response", method = RequestMethod.POST)
    public void loginAuthzCodeFlowQueryResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException {
        login(request, response, ResponseType.authorizationCode(), ResponseMode.QUERY, URI.create(redirectEndpointUrlAuthzCodeFlowQueryResponse));
    }

    private void login(
            HttpServletRequest request,
            HttpServletResponse response,
            ResponseType responseType,
            ResponseMode responseMode,
            URI redirectUri) throws OIDCClientException {
        installIfNeeded();

        State state = new State();
        Nonce nonce = new Nonce();
        this.authnRequestTracker.add(state, nonce);

        boolean requestRefreshToken = responseType.contains(ResponseTypeValue.AUTHORIZATION_CODE);

        TokenSpec tokenSpec = new TokenSpec.Builder().
                refreshToken(requestRefreshToken).
                idTokenGroups(GroupMembershipType.FULL).
                accessTokenGroups(GroupMembershipType.FILTERED).
                resourceServers(Arrays.asList(resourceServers)).build();
        URI authenticationRequestURI = client.buildAuthenticationRequestURI(
                redirectUri,
                responseType,
                responseMode,
                tokenSpec,
                state,
                nonce);
        sendRedirect(response, authenticationRequestURI);
    }

    @RequestMapping(value = "/login_implicit_flow_form_response", method = RequestMethod.POST)
    public void loginImplicitFlowFormResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException {
        login(request, response, ResponseType.idTokenAccessToken(), ResponseMode.FORM_POST, URI.create(redirectEndpointUrlImplicitFlowFormResponse));
    }

    @RequestMapping(value = "/login_implicit_flow_fragment_response", method = RequestMethod.POST)
    public void loginImplicitFlowFragmentResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException {
        login(request, response, ResponseType.idTokenAccessToken(), ResponseMode.FRAGMENT, URI.create(redirectEndpointUrlImplicitFlowFragmentResponse));
    }

    @RequestMapping(value = "/redirect_authz_code_flow_form_response", method = RequestMethod.POST)
    public void redirectAuthzCodeFlowFormResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        redirectAuthzCodeFlow(request, response, URI.create(redirectEndpointUrlAuthzCodeFlowFormResponse));
    }

    @RequestMapping(value = "/redirect_authz_code_flow_query_response", method = RequestMethod.GET)
    public void redirectAuthzCodeFlowQueryResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        redirectAuthzCodeFlow(request, response, URI.create(redirectEndpointUrlAuthzCodeFlowQueryResponse));
    }

    private void redirectAuthzCodeFlow(
            HttpServletRequest request,
            HttpServletResponse response,
            URI redirectUri) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        AuthenticationCodeResponse authnCodeResponse = ListenerHelper.parseAuthenticationCodeResponse(request);
        State state = authnCodeResponse.getState();
        AuthorizationCode authzCode = authnCodeResponse.getAuthorizationCode();

        Nonce nonce = this.authnRequestTracker.remove(state);
        assert nonce != null;

        OIDCTokens tokens = client.acquireTokensByAuthorizationCode(authzCode, redirectUri);
        validateTokenResponse(tokens, TokenType.HOK);

        assert Objects.equals(tokens.getIDToken().getNonce(), nonce);

        ResourceServerAccessToken.build(
                tokens.getAccessToken().getValue(),
                providerRSAPublicKey,
                providerMetadata.getIssuer(),
                resourceServers[0],
                CLOCK_TOLERANCE_SECONDS);

        OIDCTokens tokensNotUsed = client.acquireTokensByRefreshToken(tokens.getRefreshToken());
        validateTokenResponse(tokensNotUsed, TokenType.HOK);

        SessionID sessionId = new SessionID();
        this.sessionManager.add(sessionId, tokens);

        response.addCookie(loginSessionCookie(sessionId));
        sendRedirect(response, rootUrl);
    }

    @RequestMapping(value = "/redirect_implicit_flow_form_response", method = RequestMethod.POST)
    public void redirectImplicitFlowFormResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException, OIDCServerException, TokenValidationException {
        AuthenticationTokensResponse authnTokensResponse = ListenerHelper.parseAuthenticationTokensResponse(
                request,
                clientConfig.getConnectionConfig().getProviderPublicKey(),
                clientConfig.getConnectionConfig().getIssuer(),
                clientConfig.getClientId(),
                clientConfig.getClockToleranceInSeconds());
        State state = authnTokensResponse.getState();
        OIDCTokens tokens = authnTokensResponse.getTokens();

        Nonce nonce = this.authnRequestTracker.remove(state);
        assert nonce != null;

        validateTokenResponse(tokens, TokenType.BEARER);
        assert Objects.equals(tokens.getIDToken().getNonce(), nonce);

        ResourceServerAccessToken.build(
                tokens.getAccessToken().getValue(),
                providerRSAPublicKey,
                providerMetadata.getIssuer(),
                resourceServers[0],
                CLOCK_TOLERANCE_SECONDS);

        SessionID sessionId = new SessionID();
        this.sessionManager.add(sessionId, tokens);

        response.addCookie(loginSessionCookie(sessionId));
        sendRedirect(response, rootUrl);
    }

    @RequestMapping(value = "/redirect_implicit_flow_fragment_response", method = RequestMethod.GET)
    public void redirectImplicitFlowFragmentResponse(
            HttpServletRequest request,
            HttpServletResponse response) {
    }

    @RequestMapping(value = "/logout_redirect", method = RequestMethod.POST)
    public void logoutUsingRedirect(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException {
        SessionID sessionId = getSessionID(request);
        if (sessionId == null) {
            sendRedirect(response, rootUrl);
            return;
        }

        OIDCTokens tokens = this.sessionManager.remove(sessionId);
        assert tokens != null;

        State logoutState = new State();
        this.logoutRequestTracker.add(logoutState, tokens.getIDToken());

        URI logoutRequestURI = client.buildLogoutRequestURI(
                URI.create(postLogoutRedirectUrl),
                tokens.getIDToken(),
                logoutState);

        response.addCookie(logoutSessionCookie());
        sendRedirect(response, logoutRequestURI);
    }

    @RequestMapping(value = "/logout_form_post", method = RequestMethod.POST)
    public void logoutUsingFormPost(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException {
        SessionID sessionId = getSessionID(request);
        if (sessionId == null) {
            sendRedirect(response, rootUrl);
            return;
        }

        OIDCTokens tokens = this.sessionManager.remove(sessionId);
        assert tokens != null;

        State logoutState = new State();
        this.logoutRequestTracker.add(logoutState, tokens.getIDToken());

        String logoutRequestForm = client.buildLogoutRequestHtmlForm(
                URI.create(postLogoutRedirectUrl),
                tokens.getIDToken(),
                logoutState);

        response.addCookie(logoutSessionCookie());
        try {
            response.getWriter().write(logoutRequestForm);
        } catch (IOException e) {
            throw new IllegalStateException(e);
        }
    }

    @RequestMapping(value = "/logout_slo", method = RequestMethod.GET)
    public void logoutSlo(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException {
        SessionID sessionId = getSessionID(request);
        if (sessionId == null) {
            return;
        }

        OIDCTokens tokens = this.sessionManager.remove(sessionId);
        assert tokens != null;

        SessionID sloSid = ListenerHelper.parseSLORequest(request);
        assert Objects.equals(sloSid, tokens.getIDToken().getSessionID());

        response.addCookie(logoutSessionCookie());
    }

    @RequestMapping(value = "/post_logout_redirect", method = RequestMethod.GET)
    public void postLogoutRedirect(
            HttpServletRequest request,
            HttpServletResponse response) throws OIDCClientException, OIDCServerException {
        State logoutState = ListenerHelper.parseLogoutResponse(request);
        IDToken idToken = this.logoutRequestTracker.remove(logoutState);
        assert idToken != null;
        sendRedirect(response, rootUrl);
    }

    private static Cookie loginSessionCookie(SessionID sessionId) {
        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, sessionId.getValue());
        sessionCookie.setPath("/openidconnect-sample-rp");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        return sessionCookie;
    }

    private static Cookie logoutSessionCookie() {
        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, "");
        sessionCookie.setPath("/openidconnect-sample-rp");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        sessionCookie.setMaxAge(0);
        return sessionCookie;
    }

    private static String getDisplayName(IDToken idToken) {
        String result;

        String subject    = idToken.getSubject().getValue();
        String givenName  = idToken.getGivenName();
        String familyName = idToken.getFamilyName();

        if (StringUtils.isNotBlank(givenName)) {
            if (StringUtils.isNotBlank(familyName)) {
                result = String.format("%s %s", givenName, familyName);
            } else {
                result = givenName;
            }
        } else {
            result = subject;
        }

        return result;
    }

    private static SessionID getSessionID(HttpServletRequest request) {
        String sessionIdString = getCookieValue(request.getCookies(), SESSION_COOKIE_NAME, null);
        return (sessionIdString != null) ? new SessionID(sessionIdString) : null;
    }

    private static String getCookieValue(
            Cookie[] cookies,
            String cookieName,
            String defaultValue) {
        if (cookies != null) {
            for (int i = 0; i < cookies.length; i++) {
                Cookie cookie = cookies[i];
                if (cookieName.equals(cookie.getName())) {
                    return (cookie.getValue());
                }
            }
        }
        return defaultValue;
    }

    private void validateTokenResponse(OIDCTokens tokens, TokenType expectedTokenType) {
        com.vmware.identity.openidconnect.protocol.IDToken idToken;
        com.vmware.identity.openidconnect.protocol.AccessToken accessToken;
        com.vmware.identity.openidconnect.protocol.RefreshToken refreshToken;
        try {
            idToken = com.vmware.identity.openidconnect.protocol.IDToken.parse(tokens.getIDToken().serialize());
            accessToken = com.vmware.identity.openidconnect.protocol.AccessToken.parse(tokens.getAccessToken().getValue());
            refreshToken = tokens.getRefreshToken() == null ? null : com.vmware.identity.openidconnect.protocol.RefreshToken.parse(tokens.getRefreshToken().getValue());
        } catch (ParseException e) {
            throw new IllegalStateException(e);
        }
        validateIdToken(idToken, expectedTokenType);
        validateAccessToken(accessToken, expectedTokenType);
        if (refreshToken != null) {
            validateRefreshToken(refreshToken, expectedTokenType);
        }
    }

    private static void validateIdToken(com.vmware.identity.openidconnect.protocol.IDToken idToken, TokenType expectedTokenType) {
        validateToken(idToken, expectedTokenType);

        String error = null;

        if (idToken.getGroups() == null) {
            error = "missing groups claim";
        }

        if (error == null && idToken.getSessionID() == null) {
            error = "missing sid claim";
        }

        if (error != null) {
            throw new IllegalStateException("id_token validation error: " + error);
        }
    }

    private static void validateAccessToken(com.vmware.identity.openidconnect.protocol.AccessToken accessToken, TokenType expectedTokenType) {
        validateToken(accessToken, expectedTokenType);

        String error = null;

        if (accessToken.getGroups() == null) {
            error = "missing groups claim";
        }

        if (error == null && !accessToken.getAudience().contains("rs_admin_server")) {
            error = "audience does not contain rs_admin_server";
        }

        if (error == null && !accessToken.getAudience().contains("rs_some_other_resource_server")) {
            error = "audience does not contain rs_some_other_resource_server";
        }

        String adminServerRole = accessToken.getAdminServerRole();
        Set<String> roles = new HashSet<String>();
        roles.add("Administrator");
        roles.add("ConfigurationUser");
        roles.add("RegularUser");
        roles.add("GuestUser");
        if (error == null && (adminServerRole == null || !roles.contains(adminServerRole))) {
            error = "unexpected admin_server_role value: " + adminServerRole;
        }

        if (error != null) {
            throw new IllegalStateException("access_token validation error: " + error);
        }
    }

    private static void validateRefreshToken(com.vmware.identity.openidconnect.protocol.RefreshToken refreshToken, TokenType expectedTokenType) {
        validateToken(refreshToken, expectedTokenType);
    }

    private static void validateToken(ServerIssuedToken token, TokenType expectedTokenType) {
        String error = null;

        if (!token.getAudience().contains(clientId)) {
            error = "audience does not contain expected client_id";
        }

        if (error == null && !clientId.equals(token.getClientID().getValue())) {
            error = "incorrect client_id";
        }

        Date now = new Date();
        Date adjustedExpirationTime = new Date(token.getExpirationTime().getTime() + CLOCK_TOLERANCE_SECONDS * 1000L);
        if (error == null && now.after(adjustedExpirationTime)) {
            error = "expired jwt";
        }

        if (error == null && token.getTokenType() != expectedTokenType) {
            error = "incorrect token_type";
        }

        if (expectedTokenType == TokenType.HOK) {
            if (error == null && token.getHolderOfKey() == null) {
                error = "missing hotk claim";
            }

            if (error == null) {
                RSAPublicKey rsaPublicKey = token.getHolderOfKey();
                if (rsaPublicKey == null) {
                    error = "could not extract an RSA 256 public key out of hotk";
                } else if (!rsaPublicKey.equals(clientCertificate.getPublicKey())) {
                    error = "hotk is not equal to the public key we sent";
                }
            }

            if (error == null && token.getActAs() == null) {
                error = "missing act_as claim";
            }
        }

        if (error != null) {
            throw new IllegalStateException(String.format("%s validation error: %s", token.getTokenClass().getValue(), error));
        }
    }

    private static void sendRedirect(HttpServletResponse response, URI target) {
        sendRedirect(response, target.toString());
    }

    private static void sendRedirect(HttpServletResponse response, String target) {
        try {
            response.sendRedirect(target);
        } catch (IOException e) {
            throw new IllegalStateException(e);
        }
    }

    private void installIfNeeded() {
        try {
            installIfNeededInternal();
        } catch (Exception e) {
            throw new IllegalStateException(e);
        }
    }

    private void installIfNeededInternal() throws Exception {
        if (isInstalled) {
            return;
        }

        String domainControllerFQDN = this.relyingPartyConfig.getOpFQDN();
        int domainControllerPort = Integer.parseInt(this.relyingPartyConfig.getOpListeningPort());

        // create and load keyStore if VECS is not enabled.
        boolean vecsKeyStoreEnabled = ("true").equals(this.relyingPartyConfig.getRpVecsEnabled());
        if (vecsKeyStoreEnabled) {
            this.keyStore = KeyStore.getInstance("VKS");
            this.keyStore.load(new VecsLoadStoreParameter("TRUSTED_ROOTS"));
        } else {
            this.keyStore = KeyStore.getInstance("JKS");
            this.keyStore.load(null, null);
            // create REST afd client to populate SSL certificates
            populateSSLCertificates(
                    domainControllerFQDN,
                    domainControllerPort);
        }

        MetadataHelper metadataHelper = new MetadataHelper.Builder(domainControllerFQDN).
                domainControllerPort(domainControllerPort).
                tenant(this.relyingPartyConfig.getTenant()).
                keyStore(this.keyStore).
                build();

        providerMetadata = metadataHelper.getProviderMetadata();

        // read RP listening port if its file exists
        File rpListeningPortFile = new File(this.relyingPartyConfig.getRpListeningPortFile());
        String rpListeningPort;
        if (rpListeningPortFile.exists()) {
            rpListeningPort = (String) RelyingPartyInstaller.readObject(this.relyingPartyConfig.getRpListeningPortFile());
        } else {
            rpListeningPort = this.relyingPartyConfig.getRpListeningPort();
        }
        try {
            localHostWithPort = Inet4Address.getLocalHost().getHostAddress() + ":" + rpListeningPort;
        } catch (UnknownHostException e) {
            throw new IllegalStateException("could not retrieve local hostname");
        }
        postLogoutRedirectUrl = String.format("https://%s/openidconnect-sample-rp/post_logout_redirect", localHostWithPort);
        redirectEndpointUrlAuthzCodeFlowFormResponse    = String.format("https://%s/openidconnect-sample-rp/redirect_authz_code_flow_form_response", localHostWithPort);
        redirectEndpointUrlAuthzCodeFlowQueryResponse   = String.format("https://%s/openidconnect-sample-rp/redirect_authz_code_flow_query_response", localHostWithPort);
        redirectEndpointUrlImplicitFlowFormResponse     = String.format("https://%s/openidconnect-sample-rp/redirect_implicit_flow_form_response", localHostWithPort);
        redirectEndpointUrlImplicitFlowFragmentResponse = String.format("https://%s/openidconnect-sample-rp/redirect_implicit_flow_fragment_response", localHostWithPort);
        logoutUrl = String.format("https://%s/openidconnect-sample-rp/logout_slo", localHostWithPort);
        rootUrl = String.format("https://%s/openidconnect-sample-rp/", localHostWithPort);

        File regInfoDir = new File(this.relyingPartyConfig.getRegInfoDir());
        File rpInfoFile = new File(this.relyingPartyConfig.getRpInfoFile());
        File opPublickeyFile = new File(this.relyingPartyConfig.getOpPublickeyFile());
        File rpPrivatekeyFile = new File(this.relyingPartyConfig.getRpPrivatekeyFile());
        File rpCertificateFile = new File(this.relyingPartyConfig.getRpCertificateFile());

        // install when at least one of the files is missing
        if (!rpInfoFile.exists() ||
                !opPublickeyFile.exists() ||
                !rpPrivatekeyFile.exists() ||
                !rpCertificateFile.exists()) {
            if (!regInfoDir.exists()) {
                regInfoDir.mkdir();
            }

            RelyingPartyInstaller oidcInstaller = new RelyingPartyInstaller(
                    this.relyingPartyConfig,
                    this.keyStore);
            oidcInstaller.install(
                    new String[]{
                            redirectEndpointUrlAuthzCodeFlowFormResponse,
                            redirectEndpointUrlAuthzCodeFlowQueryResponse,
                            redirectEndpointUrlImplicitFlowFormResponse,
                            redirectEndpointUrlImplicitFlowFragmentResponse},
                    new String[]{postLogoutRedirectUrl},
                    logoutUrl);
        }

        clientId = (String) RelyingPartyInstaller.readObject(this.relyingPartyConfig.getRpInfoFile());
        providerRSAPublicKey = (RSAPublicKey) RelyingPartyInstaller.loadPublicKey(this.relyingPartyConfig.getOpPublickeyFile(), "RSA");
        RSAPrivateKey clientPrivateKey = (RSAPrivateKey) RelyingPartyInstaller.loadPrivateKey(this.relyingPartyConfig.getRpPrivatekeyFile(), "RSA");
        clientCertificate = (X509Certificate) RelyingPartyInstaller.readObject(this.relyingPartyConfig.getRpCertificateFile());

        highAvailabilityEnabled = ("true").equals(this.relyingPartyConfig.getRpHaEnabled());
        tenantName = this.relyingPartyConfig.getTenant();

        // create OIDCClient object to help client get tokens
        ConnectionConfig connectionConfig = new ConnectionConfig(
                providerMetadata,
                providerRSAPublicKey,
                this.keyStore);

        HolderOfKeyConfig holderOfKeyConfig = new HolderOfKeyConfig(clientPrivateKey, clientCertificate);

        clientConfig = new ClientConfig(
                connectionConfig,
                new ClientID(clientId),
                holderOfKeyConfig,
                highAvailabilityEnabled ? new HighAvailabilityConfig(tenantName) : null,
                CLOCK_TOLERANCE_SECONDS,
                ClientAuthenticationMethod.PRIVATE_KEY_JWT);

        client = new OIDCClient(clientConfig);

        isInstalled = true;
    }

    private void populateSSLCertificates(String domainControllerFQDN, int domainControllerPort) throws Exception {
        AfdClient afdClient =
                new AfdClient(domainControllerFQDN, domainControllerPort, NoopHostnameVerifier.INSTANCE, new SSLContextBuilder()
                        .loadTrustMaterial(null, new TrustStrategy() {
                            @Override
                            public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                                return true;
                            }
                        }).build());

        List<CertificateDTO> certs = afdClient.vecs().getSSLCertificates();
        int index = 1;
        for (CertificateDTO cert : certs) {
            this.keyStore.setCertificateEntry(String.format("VecsSSLCert%d", index), cert.getX509Certificate());
            index++;
        }
    }
}
