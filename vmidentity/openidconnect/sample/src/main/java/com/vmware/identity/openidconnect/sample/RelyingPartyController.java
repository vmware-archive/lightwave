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
import java.net.Inet4Address;
import java.net.URI;
import java.net.UnknownHostException;
import java.security.KeyStore;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import net.minidev.json.JSONObject;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

import com.nimbusds.jose.jwk.JWKSet;
import com.nimbusds.jose.jwk.RSAKey;
import com.nimbusds.jwt.ReadOnlyJWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.client.AuthenticationCodeResponse;
import com.vmware.identity.openidconnect.client.AuthenticationFrameworkHelper;
import com.vmware.identity.openidconnect.client.AuthenticationResponse;
import com.vmware.identity.openidconnect.client.AuthenticationTokensResponse;
import com.vmware.identity.openidconnect.client.AuthorizationCodeGrant;
import com.vmware.identity.openidconnect.client.ClientConfig;
import com.vmware.identity.openidconnect.client.ClientInformation;
import com.vmware.identity.openidconnect.client.ConnectionConfig;
import com.vmware.identity.openidconnect.client.HighAvailabilityConfig;
import com.vmware.identity.openidconnect.client.HolderOfKeyConfig;
import com.vmware.identity.openidconnect.client.IDToken;
import com.vmware.identity.openidconnect.client.ListenerHelper;
import com.vmware.identity.openidconnect.client.Nonce;
import com.vmware.identity.openidconnect.client.OIDCClient;
import com.vmware.identity.openidconnect.client.OIDCTokens;
import com.vmware.identity.openidconnect.client.ProviderMetadata;
import com.vmware.identity.openidconnect.client.RefreshToken;
import com.vmware.identity.openidconnect.client.RefreshTokenGrant;
import com.vmware.identity.openidconnect.client.ResourceServerAccessToken;
import com.vmware.identity.openidconnect.client.ResponseMode;
import com.vmware.identity.openidconnect.client.ResponseType;
import com.vmware.identity.openidconnect.client.ResponseValue;
import com.vmware.identity.openidconnect.client.SessionID;
import com.vmware.identity.openidconnect.client.State;
import com.vmware.identity.openidconnect.client.TokenSpec;
import com.vmware.identity.openidconnect.client.TokenType;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.provider.VecsLoadStoreParameter;

/**
 * @author Yehia Zayour
 * @author Jun Sun
 */
@Controller
public class RelyingPartyController {
    private static final String SESSION_COOKIE_NAME = "rp_session_id";

    private static boolean highAvailabilityEnabled;
    private static String tenantName;
    private static String localHostWithPort;
    private static ProviderMetadata providerMetadata;
    private static ClientInformation clientInformation;
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
    SessionManager sessionManager;

    @Autowired
    AuthnRequestStateRepository authnStateRepo;

    @Autowired
    LogoutRequestStateRepository logoutStateRepo;

    @Autowired
    RelyingPartyConfig relyingPartyConfig;

    @RequestMapping(value = "/", method = RequestMethod.GET)
    public String root(
            HttpServletRequest request,
            HttpServletResponse response,
            ModelMap modelMap) throws Exception {
        installIfNeeded();

        String sessionId = getCookieValue(request.getCookies(), SESSION_COOKIE_NAME, null);
        OIDCTokens tokens = (sessionId != null) ? this.sessionManager.get(sessionId) : null;

        if (tokens != null) {
            modelMap.addAttribute("loggedIn", true);
            modelMap.addAttribute("displayText", String.format("welcome %s, you are now logged in!", getDisplayName(tokens.getIdToken())));
            modelMap.addAttribute("clientId", clientInformation.getClientId().getValue());
            modelMap.addAttribute("id_token", tokens.getIdToken().serialize());
            modelMap.addAttribute("access_token", tokens.getAccessToken().getValue());
            modelMap.addAttribute("refresh_token", (tokens.getRefreshToken() == null) ? null : tokens.getRefreshToken().getValue());
        } else {
            modelMap.addAttribute("loggedIn", false);
            modelMap.addAttribute("displayText", "");
            modelMap.addAttribute("clientId", clientInformation.getClientId().getValue());
        }

        return "resource";
    }

    @RequestMapping(value = "/login_authz_code_flow_form_response", method = RequestMethod.POST)
    public void loginAuthzCodeFlow(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        loginAuthzCodeFlow(request, response, ResponseMode.FORM_POST, new URI(redirectEndpointUrlAuthzCodeFlowFormResponse));
    }

    @RequestMapping(value = "/login_authz_code_flow_query_response", method = RequestMethod.POST)
    public void loginAuthzCodeFlowQueryResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        loginAuthzCodeFlow(request, response, ResponseMode.QUERY, new URI(redirectEndpointUrlAuthzCodeFlowQueryResponse));
    }

    private void loginAuthzCodeFlow(
            HttpServletRequest request,
            HttpServletResponse response,
            ResponseMode responseMode,
            URI redirectUri) throws Exception {
        installIfNeeded();

        State state = new State(UUID.randomUUID().toString());
        Nonce nonce = new Nonce(UUID.randomUUID().toString());
        this.authnStateRepo.add(state, nonce);

        Set<ResponseValue> responseValueSet = new HashSet<ResponseValue>();
        responseValueSet.add(ResponseValue.CODE);
        TokenSpec tokenSpec = new TokenSpec.Builder(TokenType.HOK).
                refreshToken(true).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList(resourceServers)).build();
        URI authenticationRequestURI = client.buildAuthenticationRequestURI(
                redirectUri,
                new ResponseType(responseValueSet),
                responseMode,
                tokenSpec,
                state,
                nonce);
        response.sendRedirect(authenticationRequestURI.toString());
    }

    @RequestMapping(value = "/login_implicit_flow_form_response", method = RequestMethod.POST)
    public void loginImplicitFlow(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        loginImplicitFlow(request, response, ResponseMode.FORM_POST, new URI(redirectEndpointUrlImplicitFlowFormResponse));
    }

    @RequestMapping(value = "/login_implicit_flow_fragment_response", method = RequestMethod.POST)
    public void loginImplicitFlowFragmentResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        loginImplicitFlow(request, response, ResponseMode.FRAGMENT, new URI(redirectEndpointUrlImplicitFlowFragmentResponse));
    }

    private void loginImplicitFlow(
            HttpServletRequest request,
            HttpServletResponse response,
            ResponseMode responseMode,
            URI redirectUri) throws Exception {
        installIfNeeded();

        State state = new State(UUID.randomUUID().toString());
        Nonce nonce = new Nonce(UUID.randomUUID().toString());
        this.authnStateRepo.add(state, nonce);

        Set<ResponseValue> responseValueSet = new HashSet<ResponseValue>();
        responseValueSet.add(ResponseValue.ID_TOKEN);
        responseValueSet.add(ResponseValue.TOKEN);
        TokenSpec tokenSpec = new TokenSpec.Builder(TokenType.BEARER).
                refreshToken(false).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList(resourceServers)).build();
        URI authenticationRequestURI = client.buildAuthenticationRequestURI(
                redirectUri,
                new ResponseType(responseValueSet),
                responseMode,
                tokenSpec,
                state,
                nonce);
        response.sendRedirect(authenticationRequestURI.toString());
    }

    @RequestMapping(value = "/redirect_authz_code_flow_form_response", method = RequestMethod.POST)
    public void redirectAuthzCodeFlow(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        redirectAuthzCodeFlow(request, response, new URI(redirectEndpointUrlAuthzCodeFlowFormResponse));
    }

    @RequestMapping(value = "/redirect_authz_code_flow_query_response", method = RequestMethod.GET)
    public void redirectAuthzCodeFlowQueryResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        redirectAuthzCodeFlow(request, response, new URI(redirectEndpointUrlAuthzCodeFlowQueryResponse));
    }

    private void redirectAuthzCodeFlow(
            HttpServletRequest request,
            HttpServletResponse response,
            URI redirectUri) throws Exception {
        Map<String, String> parameterMap = new HashMap<String, String>();
        for (String key : request.getParameterMap().keySet()) {
            parameterMap.put(key, request.getParameterMap().get(key)[0]);
        }
        AuthenticationResponse authnResponse = ListenerHelper.parseAuthenticationResponse(parameterMap, clientConfig);
        if (!(authnResponse instanceof AuthenticationCodeResponse)) {
            throw new IllegalStateException("Invalid authentication response");
        }
        State state = ((AuthenticationCodeResponse) authnResponse).getState();
        if (!this.authnStateRepo.contains(state)) {
            throw new IllegalStateException("we never used this state: " + state.toString());
        }

        AuthorizationCodeGrant codeGrant = new AuthorizationCodeGrant(
                ((AuthenticationCodeResponse) authnResponse).getAuthorizationCode(),
                redirectUri);
        TokenSpec tokenSpec = new TokenSpec.Builder(TokenType.HOK).
                refreshToken(true).
                idTokenGroups(true).
                accessTokenGroups(true).
                resouceServers(Arrays.asList(resourceServers)).build();
        OIDCTokens oidcTokens = client.acquireTokens(codeGrant, tokenSpec);
        validateTokenResponse(oidcTokens, TokenType.HOK);

        if (!oidcTokens.getIdToken().getNonce().equals(this.authnStateRepo.getNonce(state))) {
            throw new IllegalStateException("nonce in request and id token not matching: " + state.toString());
        }

        // build an access token for resource server
        ResourceServerAccessToken accessToken = ResourceServerAccessToken.build(
                oidcTokens.getAccessToken().getValue(),
                providerRSAPublicKey,
                resourceServers[0],
                providerMetadata.getIssuer());

        // check the access token can read claims
        if (!accessToken.getAudience().contains(resourceServers[0])) {
            throw new IllegalStateException("Audience should be contained in the resource server version access token.");
        }

        RefreshToken refreshToken = oidcTokens.getRefreshToken();
        if (refreshToken == null) {
            throw new IllegalStateException("refresh_token is null");
        }

        OIDCTokens oidcTokensNotUsed = client.acquireTokens(new RefreshTokenGrant(refreshToken), tokenSpec);
        validateTokenResponse(oidcTokensNotUsed, TokenType.HOK);

        String sessionId = UUID.randomUUID().toString();
        this.sessionManager.add(sessionId, oidcTokens);

        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, sessionId);
        sessionCookie.setPath("/openidconnect-sample-rp");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        response.addCookie(sessionCookie);

        response.sendRedirect(rootUrl);
    }

    @RequestMapping(value = "/redirect_implicit_flow_form_response", method = RequestMethod.POST)
    public void redirectImplicitFlow(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        Map<String, String> parameterMap = new HashMap<String, String>();
        for (String key : request.getParameterMap().keySet()) {
            parameterMap.put(key, request.getParameterMap().get(key)[0]);
        }
        AuthenticationResponse authnResponse = ListenerHelper.parseAuthenticationResponse(parameterMap, clientConfig);
        if (!(authnResponse instanceof AuthenticationTokensResponse)) {
            throw new IllegalStateException("Invalid authentication response");
        }
        State state = ((AuthenticationTokensResponse) authnResponse).getState();
        if (!this.authnStateRepo.contains(state)) {
            throw new IllegalStateException("we never used this state: " + state.toString());
        }

        OIDCTokens oidcTokens = ((AuthenticationTokensResponse) authnResponse).getOidcTokens();
        validateTokenResponse(oidcTokens, TokenType.BEARER);

        if (!oidcTokens.getIdToken().getNonce().equals(this.authnStateRepo.getNonce(state))) {
            throw new IllegalStateException("nonce in request and id token not matching: " + state.toString());
        }

        ResourceServerAccessToken accessToken = ResourceServerAccessToken.build(
                oidcTokens.getAccessToken().getValue(),
                providerRSAPublicKey,
                resourceServers[0],
                providerMetadata.getIssuer());

        if (!accessToken.getAudience().contains(resourceServers[0])) {
            throw new IllegalStateException("Audience should be contained in the resource server version access token.");
        }

        String sessionId = UUID.randomUUID().toString();
        this.sessionManager.add(sessionId, oidcTokens);

        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, sessionId);
        sessionCookie.setPath("/openidconnect-sample-rp");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        response.addCookie(sessionCookie);

        response.sendRedirect(rootUrl);
    }

    @RequestMapping(value = "/redirect_implicit_flow_fragment_response", method = RequestMethod.GET)
    public void redirectImplicitFlowFragmentResponse(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
    }

    @RequestMapping(value = "/logout", method = RequestMethod.POST)
    public void logoutButtonClick(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        String sessionId = getCookieValue(request.getCookies(), SESSION_COOKIE_NAME, null);
        if (sessionId == null) {
            response.sendRedirect(rootUrl);
            return;
        }

        OIDCTokens tokens = this.sessionManager.get(sessionId);
        this.sessionManager.remove(sessionId);

        State logoutState = new State(UUID.randomUUID().toString());
        this.logoutStateRepo.add(logoutState, tokens.getIdToken().getSubject());

        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, "");
        sessionCookie.setPath("/openidconnect-sample-rp");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        sessionCookie.setMaxAge(0);
        response.addCookie(sessionCookie);

        URI logoutRequestURI = client.buildLogoutRequestURI(
                new URI(postLogoutRedirectUrl),
                tokens.getIdToken(),
                logoutState);
        response.sendRedirect(logoutRequestURI.toString());
    }

    @RequestMapping(value = "/logout", method = RequestMethod.GET)
    public void logoutSlo(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        Map<String, String> parameterMap = new HashMap<String, String>();
        for (String key : request.getParameterMap().keySet()) {
            parameterMap.put(key, request.getParameterMap().get(key)[0]);
        }
        String sessionId = getCookieValue(request.getCookies(), SESSION_COOKIE_NAME, null);
        if (sessionId == null) {
            return;
        }

        SessionID sid = ListenerHelper.parseSLORequest(parameterMap);
        if (sid == null) {
            throw new IllegalStateException("sid parameter expected");
        }
        OIDCTokens tokens = this.sessionManager.get(sessionId);
        String sidIdTokenClaim = (String) tokens.getIdToken().getClaim("sid");
        if (!sid.getValue().equals(sidIdTokenClaim)) {
            String error = String.format("sid parameter [%s] does not match sid in id_token [%s]", sid.getValue(), sidIdTokenClaim);
            throw new IllegalStateException(error);
        }
        this.sessionManager.remove(sessionId);

        Cookie sessionCookie = new Cookie(SESSION_COOKIE_NAME, "");
        sessionCookie.setPath("/openidconnect-sample-rp");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        sessionCookie.setMaxAge(0);
        response.addCookie(sessionCookie);
    }

    // this is also being used as the logout_uri according to "OpenID Connect HTTP-Based Logout 1.0 - draft 00"
    @RequestMapping(value = "/post-logout-redirect", method = RequestMethod.GET)
    public void postLogoutRedirect(
            HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        Map<String, String> parameterMap = new HashMap<String, String>();
        for (String key : request.getParameterMap().keySet()) {
            parameterMap.put(key, request.getParameterMap().get(key)[0]);
        }
        State logoutState = ListenerHelper.parseLogoutResponse(parameterMap);
        if (logoutState == null || !this.logoutStateRepo.contains(logoutState)) {
            throw new IllegalStateException("invalid logoutState: " + logoutState);
        }
        this.logoutStateRepo.remove(logoutState);
        response.sendRedirect(rootUrl);
    }

    private static String getDisplayName(IDToken idToken) {
        String result;

        String subject    = idToken.getSubject();
        String givenName  = (String) idToken.getClaim("given_name");
        String familyName = (String) idToken.getClaim("family_name");

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

    private void installIfNeeded() throws Exception {
        if (isInstalled) {
            return;
        }

        // create and load keyStore if VECS is not enabled.
        boolean vecsKeyStoreEnabled = ("true").equals(this.relyingPartyConfig.getRpVecsEnabled());
        if (vecsKeyStoreEnabled) {
            this.keyStore = KeyStore.getInstance("VKS");
            this.keyStore.load(new VecsLoadStoreParameter("TRUSTED_ROOTS"));
        } else {
            this.keyStore = KeyStore.getInstance("JKS");
            this.keyStore.load(null, null);
            String domainControllerFQDN = this.relyingPartyConfig.getOpFQDN();
            int domainControllerPort = Integer.parseInt(this.relyingPartyConfig.getOpListeningPort());
            AuthenticationFrameworkHelper authenticationFrameworkHelper = new AuthenticationFrameworkHelper(
                    domainControllerFQDN,
                    domainControllerPort);
            authenticationFrameworkHelper.populateSSLCertificates(this.keyStore);
        }

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
        postLogoutRedirectUrl = String.format("https://%s/openidconnect-sample-rp/post-logout-redirect", localHostWithPort);
        redirectEndpointUrlAuthzCodeFlowFormResponse    = String.format("https://%s/openidconnect-sample-rp/redirect_authz_code_flow_form_response", localHostWithPort);
        redirectEndpointUrlAuthzCodeFlowQueryResponse   = String.format("https://%s/openidconnect-sample-rp/redirect_authz_code_flow_query_response", localHostWithPort);
        redirectEndpointUrlImplicitFlowFormResponse     = String.format("https://%s/openidconnect-sample-rp/redirect_implicit_flow_form_response", localHostWithPort);
        redirectEndpointUrlImplicitFlowFragmentResponse = String.format("https://%s/openidconnect-sample-rp/redirect_implicit_flow_fragment_response", localHostWithPort);
        logoutUrl = String.format("https://%s/openidconnect-sample-rp/logout", localHostWithPort);
        rootUrl = String.format("https://%s/openidconnect-sample-rp/", localHostWithPort);

        File regInfoDir = new File(this.relyingPartyConfig.getRegInfoDir());
        File rpInfoFile = new File(this.relyingPartyConfig.getRpInfoFile());
        File opMetadataFile = new File(this.relyingPartyConfig.getOpMetadataFile());
        File opPublickeyFile = new File(this.relyingPartyConfig.getOpPublickeyFile());
        File rpPrivatekeyFile = new File(this.relyingPartyConfig.getRpPrivatekeyFile());
        File rpCertificateFile = new File(this.relyingPartyConfig.getRpCertificateFile());

        // install when at least one of the files is missing
        if (!rpInfoFile.exists() ||
                !opMetadataFile.exists() ||
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

        clientInformation = (ClientInformation) RelyingPartyInstaller.readObject(this.relyingPartyConfig.getRpInfoFile());
        providerMetadata = (ProviderMetadata) RelyingPartyInstaller.readObject(this.relyingPartyConfig.getOpMetadataFile());
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
                clientInformation.getClientId(),
                holderOfKeyConfig,
                highAvailabilityEnabled ? new HighAvailabilityConfig(tenantName) : null);

        client = new OIDCClient(clientConfig);

        isInstalled = true;
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

    private void validateTokenResponse(OIDCTokens tokens, TokenType expectedTokenType) throws Exception {
        SignedJWT signedJwt;

        signedJwt = SignedJWT.parse(tokens.getIdToken().serialize());
        validateIdToken(signedJwt.getJWTClaimsSet(), expectedTokenType);

        signedJwt = SignedJWT.parse(tokens.getAccessToken().getValue());
        validateAccessToken(signedJwt.getJWTClaimsSet(), expectedTokenType);

        if (tokens.getRefreshToken() != null) {
            signedJwt = SignedJWT.parse(tokens.getRefreshToken().getValue());
            validateRefreshToken(signedJwt.getJWTClaimsSet(), expectedTokenType);
        }
    }

    private static void validateIdToken(ReadOnlyJWTClaimsSet claimsSet, TokenType expectedTokenType) throws Exception {
        validateToken(claimsSet, TokenClass.ID_TOKEN, expectedTokenType);

        String error = null;

        String sessionId = claimsSet.getStringClaim("sid");
        if (sessionId == null) {
            error = "missing sid claim";
        }

        if (error != null) {
            throw new IllegalStateException("id_token validation error: " + error);
        }
    }

    private static void validateAccessToken(ReadOnlyJWTClaimsSet claimsSet, TokenType expectedTokenType) throws Exception {
        validateToken(claimsSet, TokenClass.ACCESS_TOKEN, expectedTokenType);

        String error = null;

        if (error == null && !claimsSet.getAudience().contains("rs_admin_server")) {
            error = "audience does not contain rs_admin_server";
        }

        if (error == null && !claimsSet.getAudience().contains("rs_some_other_resource_server")) {
            error = "audience does not contain rs_some_other_resource_server";
        }

        String adminServerRole = claimsSet.getStringClaim("admin_server_role");
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

    private static void validateRefreshToken(ReadOnlyJWTClaimsSet claimsSet, TokenType expectedTokenType) throws Exception {
        validateToken(claimsSet, TokenClass.REFRESH_TOKEN, expectedTokenType);
    }

    private static void validateToken(ReadOnlyJWTClaimsSet claimsSet, TokenClass tokenClass, TokenType expectedTokenType) throws Exception {
        String error = null;

        if (!claimsSet.getAudience().contains(clientInformation.getClientId().getValue())) {
            error = "audience does not contain expected client_id";
        }

        if (error == null && !clientInformation.getClientId().getValue().equals(claimsSet.getStringClaim("client_id"))) {
            error = "incorrect client_id";
        }

        if (error == null && !tenantName.equals(claimsSet.getStringClaim("tenant"))) {
            error = "incorrect tenant";
        }

        Date now = new Date();
        if (error == null && now.after(claimsSet.getExpirationTime())) {
            error = "expired jwt";
        }

        if (error == null &&
                (tokenClass == TokenClass.ID_TOKEN || tokenClass == TokenClass.ACCESS_TOKEN) &&
                claimsSet.getClaim("groups") == null) {
            error = "missing groups claim";
        }

        if (error == null && !tokenClass.getName().equals(claimsSet.getStringClaim("token_class"))) {
            error = "incorrect token_class";
        }

        if (error == null && !expectedTokenType.getValue().equals(claimsSet.getStringClaim("token_type"))) {
            error = "incorrect token_type";
        }

        if (expectedTokenType == TokenType.HOK) {
            Object hotk = claimsSet.getClaim("hotk");
            if (error == null && hotk == null) {
                error = "missing hotk claim";
            }

            if (error == null) {
                JWKSet jwkSet = JWKSet.parse((JSONObject) hotk);
                RSAKey rsaKey = (RSAKey) jwkSet.getKeys().get(0);
                RSAPublicKey rsaPublicKey = rsaKey.toRSAPublicKey();
                if (rsaPublicKey == null) {
                    error = "could not extract an RSA 256 public key out of hotk";
                } else if (!rsaPublicKey.equals(clientCertificate.getPublicKey())) {
                    error = "hotk is not equal to the public key we sent";
                }
            }

            if (error == null && claimsSet.getStringClaim("act_as") == null) {
                error = "missing act_as claim";
            }
        }

        if (error != null) {
            throw new IllegalStateException(String.format("%s validation error: %s", tokenClass.getName(), error));
        }
    }
}
