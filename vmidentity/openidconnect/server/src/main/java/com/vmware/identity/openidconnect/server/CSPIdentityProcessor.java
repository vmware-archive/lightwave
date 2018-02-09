/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.nio.charset.UnsupportedCharsetException;
import java.util.Date;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang3.tuple.Pair;
import org.apache.http.HttpEntity;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.entity.ContentType;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;

import com.nimbusds.jose.JWSVerifier;
import com.nimbusds.jose.crypto.RSASSAVerifier;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.DomainType;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.OidcConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.ResponseMode;
import com.vmware.identity.openidconnect.common.ResponseType;
import com.vmware.identity.openidconnect.common.ResponseTypeValue;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.protocol.AccessToken;
import com.vmware.identity.openidconnect.protocol.AuthenticationRequest;
import com.vmware.identity.openidconnect.protocol.AuthenticationSuccessResponse;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.CSPToken;
import com.vmware.identity.openidconnect.protocol.FederationIDPIssuerType;
import com.vmware.identity.openidconnect.protocol.FederationToken;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
import com.vmware.identity.openidconnect.protocol.IDToken;
import com.vmware.identity.openidconnect.protocol.JSONUtils;
import com.vmware.identity.openidconnect.protocol.URIUtils;

import net.minidev.json.JSONObject;

@Controller
public class CSPIdentityProcessor implements FederatedIdentityProcessor {

  private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(CSPIdentityProcessor.class);

  public static final String QUERY_PARAM_ORG_LINK = "orgLink";
  public static final String QUERY_PARAM_CODE = "code";
  public static final String QUERY_PARAM_CLIENT_ID = "client_id";
  public static final String QUERY_PARAM_REDIRECT_URI = "redirect_uri";
  public static final String QUERY_PARAM_STATE = "state";
  public static final String QUERY_PARAM_GRANT_TYPE = "grant_type";

  public static final String ROLE_CSP_ORG_OWNER = "csp:org_owner";
  public static final String CSP_ORG_LINK = "/csp/gateway/am/api/orgs/";

  @Autowired
  private CasIdmClient idmClient;

  @Autowired
  private SessionManager sessionManager;

  private final HashMap<String, FederatedTokenPublicKey> publicKeyLookup;
  private final ReadWriteLock keyLookupLock;
  private final Lock readLockKeyLookup;
  private final Lock writeLockKeyLookup;

  public CSPIdentityProcessor() {
    publicKeyLookup = new HashMap<String, FederatedTokenPublicKey>();
    keyLookupLock = new ReentrantReadWriteLock();
    readLockKeyLookup = keyLookupLock.readLock();
    writeLockKeyLookup = keyLookupLock.writeLock();
  }

    @Override
    public HttpResponse processAuthRequestForFederatedIDP(HttpServletRequest request, String tenant,
            IDPConfig idpConfig) throws Exception {
        HttpRequest httpRequest = HttpRequest.from(request);
        AuthenticationRequest authnRequest = AuthenticationRequest.parse(httpRequest);
        FederationRelayState.Builder builder = new FederationRelayState.Builder(idpConfig.getEntityID(),
                authnRequest.getClientID().getValue(), authnRequest.getRedirectURI().toString());
        final String orgLink = CSP_ORG_LINK + tenant;
        builder.nonce(authnRequest.getNonce().getValue());
        builder.scope(authnRequest.getScope().toString());
        builder.spInitiatedState(authnRequest.getState().getValue());
        builder.responseMode(authnRequest.getResponseMode().getValue());
        builder.responseType(authnRequest.getResponseType().toString());
        final FederationRelayState relayState = builder.build();
        return processRequestPreAuth(request, relayState, orgLink, idpConfig);
  }

  @Override
  public HttpResponse
  processRequest(
      HttpServletRequest request,
      FederationRelayState relayState,
      IDPConfig idpConfig
  ) throws Exception {
    final String orgLink = request.getParameter(QUERY_PARAM_ORG_LINK);
    if (orgLink != null && !orgLink.isEmpty()) {
      return processRequestPreAuth(request, relayState, orgLink, idpConfig);
    }

    final String code = request.getParameter(QUERY_PARAM_CODE);
    if (code != null && !code.isEmpty()) {
      return processRequestAuth(request, relayState, code, idpConfig);
    }

    throw new ServerException(ErrorObject.invalidRequest("Error: Invalid request"));
  }

  private HttpResponse
  processRequestPreAuth(
      HttpServletRequest request,
      FederationRelayState relayState,
      String orgLink,
      IDPConfig idpConfig
  ) throws Exception {
    OidcConfig oidcConfig = idpConfig.getOidcConfig();
    URI target = new URI(oidcConfig.getAuthorizeRedirectURI());

    Map<String, String> parameters = new HashMap<String, String>();

    parameters.put(QUERY_PARAM_CLIENT_ID, oidcConfig.getClientId());
    parameters.put(QUERY_PARAM_ORG_LINK, orgLink);
    parameters.put(QUERY_PARAM_REDIRECT_URI, oidcConfig.getRedirectURI()); // match URI registered with ClientID
    parameters.put(QUERY_PARAM_STATE, relayState.getEncodedValue());

    URI redirectTarget = URIUtils.appendQueryParameters(target, parameters);
    return HttpResponse.createRedirectResponse(redirectTarget);
  }

  private HttpResponse
  processRequestAuth(
      HttpServletRequest request,
      FederationRelayState relayState,
      String authenticode,
      IDPConfig idpConfig
  ) throws Exception {
    SessionID session = new SessionID();
    sessionManager.add(session);
    // Get the Token corresponding to the code
    Pair<CSPToken, CSPToken> token = getToken(authenticode, relayState, idpConfig, session);
    // Process the response
    // TODO: Nonce and state handling
    return processResponse(token, idpConfig, relayState, session);
  }

  private Pair<CSPToken, CSPToken>
  getToken( String code, FederationRelayState relayState, IDPConfig idpConfig, SessionID session) throws Exception {
    OidcConfig oidcConfig = idpConfig.getOidcConfig();
    URI target = new URI(oidcConfig.getTokenRedirectURI());
    Map<String, String> parameters = new HashMap<String, String>();
    parameters.put(QUERY_PARAM_REDIRECT_URI, oidcConfig.getRedirectURI());
    parameters.put(QUERY_PARAM_STATE, relayState.getEncodedValue());
    parameters.put(QUERY_PARAM_GRANT_TYPE, "authorization_code");
    parameters.put(QUERY_PARAM_CODE, code);
    Map<String, String> headers = new HashMap<String, String>();
    headers.put(
        "Authorization",
        String.format(
            "Basic %s",
            Base64Utils.encodeToString(
                String.format(
                    "%s:%s",
                    oidcConfig.getClientId(),
                    oidcConfig.getClientSecret()
                )
            )
        )
    );
    return getToken(target, headers, parameters, getPublicKey(idpConfig), session);
  }

  private HttpResponse
  processResponse(Pair<CSPToken, CSPToken> token, IDPConfig idpConfig, FederationRelayState state, SessionID session) throws Exception {
    CSPToken idToken = token.getLeft();
    CSPToken accessToken = token.getRight();
    String tenantName = idToken.getOrgId();
    if (tenantName == null || tenantName.isEmpty()) {
      ErrorObject errorObject = ErrorObject.invalidRequest("Invalid tenant name");
      LoggerUtils.logFailedRequest(logger, errorObject);
      throw new ServerException(errorObject);
    }

    TenantInfo tenantInfo = getTenantInfo(tenantName);
    if (tenantInfo == null) {
      ErrorObject errorObject = ErrorObject.accessDenied(
          String.format("Tenant [%s] does not exist", tenantName)
      );
      LoggerUtils.logFailedRequest(logger, errorObject);
      throw new ServerException(errorObject);
    }

    String systemDomain = getSystemDomainName(tenantName);
    if (systemDomain == null || systemDomain.isEmpty()) {
      throw new ServerException(ErrorObject.serverError("The system domain is invalid"));
    }

    PrincipalId user = new PrincipalId(accessToken.getUsername(), accessToken.getDomain());
    FederatedIdentityProviderInfoRetriever federatedInfoRetriever = new FederatedIdentityProviderInfoRetriever(this.idmClient);
    FederatedIdentityProviderInfo federatedIdpInfo = federatedInfoRetriever.retrieveInfo(state.getIssuer());

    FederatedIdentityProvider federatedIdp = new FederatedIdentityProvider(tenantName, this.idmClient);
    if (!federatedIdp.isFederationUserActive(user)) {
        federatedIdp.provisionFederationUser(state.getIssuer(), user);
    }

    federatedIdp.updateUserGroups(user, accessToken.getPermissions(), federatedIdpInfo.getRoleGroupMappings());

    ClientInfoRetriever clientInfoRetriever = new ClientInfoRetriever(idmClient);
    ClientID clientID = new ClientID(state.getClientId());
    ClientInfo clientInfo = clientInfoRetriever.retrieveClientInfo(systemDomain, clientID);

    Cookie sessionCookie = loggedInSessionCookie(tenantName, session);
    Cookie cspIssuerCookie = CSPIssuerCookie(tenantName, state.getIssuer());
    PersonUser personUser = new PersonUser(user, tenantName);
    sessionManager.update(session, personUser, LoginMethod.PASSWORD, clientInfo);
    HttpResponse httpResponse;
    if (StringUtils.isNotEmpty(state.getSPInitiatedState())) {
        AuthenticationSuccessResponse authnSuccessResponse = processIDTokenResponse(tenantName, state, personUser, session);
        httpResponse = authnSuccessResponse.toHttpResponse();
    } else {
        String redirectURI = String.format("%s/%s", state.getRedirectURI(), tenantName);
        httpResponse = HttpResponse.createRedirectResponse(new URI(redirectURI));
    }
    httpResponse.addCookie(sessionCookie);
    httpResponse.addCookie(cspIssuerCookie);

    return httpResponse;
  }

    private AuthenticationSuccessResponse processIDTokenResponse(String tenant, FederationRelayState state,
            PersonUser personUser, SessionID sessionId) throws Exception {
        ServerInfoRetriever serverInfoRetriever = new ServerInfoRetriever(idmClient);
        UserInfoRetriever userInfoRetriever = new UserInfoRetriever(idmClient);
        Scope scope = Scope.parse(state.getScope());
        Set<ResourceServerInfo> resourceServerInfos = serverInfoRetriever.retrieveResourceServerInfos(tenant, scope);
        UserInfo userInfo = userInfoRetriever.retrieveUserInfo(personUser, scope, resourceServerInfos);
        TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(idmClient);
        TenantInfo tenantInfo = tenantInfoRetriever.retrieveTenantInfo(tenant);

        TokenIssuer tokenIssuer = new TokenIssuer(personUser, (SolutionUser) null, userInfo, tenantInfo, scope,
                new Nonce(state.getNonce()), new ClientID(state.getClientId()), sessionId);

        IDToken idToken = tokenIssuer.issueIDToken();
        AccessToken accessToken = null;
        if (ResponseType.parse(state.getResponseType()).contains(ResponseTypeValue.ACCESS_TOKEN)) {
            accessToken = tokenIssuer.issueAccessToken();
        }

        return new AuthenticationSuccessResponse(ResponseMode.parse(state.getResponseMode()),
                new URI(state.getRedirectURI()), State.parse(state.getSPInitiatedState()), false,
                (AuthorizationCode) null, idToken, accessToken);
    }

  private String getSystemDomainName(String tenantName) throws Exception {
    EnumSet<DomainType> domains = EnumSet.of(DomainType.SYSTEM_DOMAIN);
    Iterator<IIdentityStoreData> iter = this.idmClient.getProviders(tenantName, domains).iterator();
    return iter.next().getName();
  }

  private FederatedTokenPublicKey getPublicKey(IDPConfig idpConfig) throws Exception {
    String entityID = idpConfig.getEntityID();
    readLockKeyLookup.lock();
    FederatedTokenPublicKey key = null;
    try {
      key = publicKeyLookup.get(entityID);
    } finally {
      readLockKeyLookup.unlock();
    }
    if (key == null) {
      String jwkUri = idpConfig.getOidcConfig().getJwksURI();
      key = FederatedTokenPublicKeyFactory.build(jwkUri, FederationIDPIssuerType.CSP);
      writeLockKeyLookup.lock();
      try {
        publicKeyLookup.put(entityID, key);
      } finally {
        writeLockKeyLookup.unlock();
      }
    }
    return key;
  }

  private Cookie loggedInSessionCookie(String tenantName, SessionID sessionId) {
    boolean isSecure=false;
    Cookie cookie = new Cookie(SessionManager.getSessionCookieName(tenantName), sessionId.getValue());
    cookie.setPath(Endpoints.BASE);
    cookie.setSecure(isSecure);
    cookie.setHttpOnly(true);
    return cookie;
  }

  private Cookie CSPIssuerCookie(String tenantName, String issuer) {
      boolean isSecure=false;
      Cookie cookie = new Cookie(SessionManager.getExternalIdpIssuerCookieName(tenantName), issuer);
      cookie.setPath(Endpoints.BASE);
      cookie.setSecure(isSecure);
      cookie.setHttpOnly(true);
      return cookie;
  }

  private TenantInfo getTenantInfo(String tenantName) {
    try {
      TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(this.idmClient);
      return tenantInfoRetriever.retrieveTenantInfo(tenantName);
    } catch (Exception e) {
      return null;
    }
  }

  private void validateIssuer(FederationToken token, String expectedIssuer) throws Exception {
      String issuer = token.getIssuer();
      if (issuer == null || !issuer.equalsIgnoreCase(expectedIssuer)) {
        ErrorObject errorObject = ErrorObject.accessDenied("Issuer does not match");
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }
  }

  private void validateExpiration(FederationToken token) throws Exception {
      Date expiryTime = token.getExpirationTime();
      Date now = new Date();
      if (now.after(expiryTime)) {
        ErrorObject errorObject = ErrorObject.accessDenied("token expired");
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }
  }

  private Pair<CSPToken, CSPToken>
  getToken(
      URI target,
      Map<String, String> headers,
      Map<String, String> parameters,
      FederatedTokenPublicKey key,
      SessionID session
  ) throws Exception {
    if (key == null) {
      ErrorObject errorObject = ErrorObject.accessDenied("Invalid public key");
      LoggerUtils.logFailedRequest(logger, errorObject);
      throw new ServerException(errorObject);
    }

    HttpRequest request = HttpRequest.createPostRequest(target, parameters);
    HttpRequestBase httpRequestBase = request.toHttpTask();
    for (Entry<String, String> entry : headers.entrySet()) {
      httpRequestBase.setHeader(entry.getKey(), entry.getValue());
    }

    try(CloseableHttpClient client = HttpClients.createDefault();
        CloseableHttpResponse response = client.execute(httpRequestBase);) {
      int statusCodeInt = response.getStatusLine().getStatusCode();
      StatusCode statusCode;
      try {
        statusCode = StatusCode.parse(statusCodeInt);
        if (statusCode != StatusCode.OK) {
          ErrorObject errorObject =
              ErrorObject.serverError(
                  String.format(
                      "Failed to retrieve token from [%s] with error content [%s]",
                      target.toString(), EntityUtils.toString(response.getEntity())
                  )
              );
          LoggerUtils.logFailedRequest(logger, errorObject);
          throw new ServerException(errorObject);
        }
      } catch (ParseException e) {
        ErrorObject errorObject = ErrorObject.serverError(
            String.format(
                "Failed to parse status code. %s:%s",
                e.getClass().getName(),
                e.getMessage()
            )
        );
        LoggerUtils.logFailedRequest(logger, errorObject, e);
        throw new ServerException(errorObject);
      }
      JSONObject jsonContent = null;
      HttpEntity httpEntity = response.getEntity();
      if (httpEntity == null) {
        ErrorObject errorObject = ErrorObject.serverError("Failed to find http entity");
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }

      ContentType contentType;
      try {
        contentType = ContentType.get(httpEntity);
      } catch (UnsupportedCharsetException | org.apache.http.ParseException e) {
        ErrorObject errorObject = ErrorObject.serverError(
            String.format(
                "Error in setting content type in HTTP response. %s:%s",
                e.getClass().getName(),
                e.getMessage()
            )
        );
        LoggerUtils.logFailedRequest(logger, errorObject, e);
        throw new ServerException(errorObject);
      }
      // TODO: Request CSP to include charset
      Charset charset = contentType.getCharset();
      if (charset != null && !StandardCharsets.UTF_8.equals(charset)) {
        ErrorObject errorObject = ErrorObject.serverError(
            String.format(
                "unsupported charset %s",
                charset
            )
        );
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }
      if (!ContentType.APPLICATION_JSON.getMimeType().equalsIgnoreCase(contentType.getMimeType())) {
        ErrorObject errorObject = ErrorObject.serverError(
            String.format(
                "unsupported mime type %s",
                contentType.getMimeType()
            )
        );
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }
      String content = EntityUtils.toString(httpEntity);
      try {
        jsonContent = JSONUtils.parseJSONObject(content);
      } catch (ParseException e) {
        ErrorObject errorObject = ErrorObject.serverError(
            String.format(
                "failed to parse json response. %s:%s",
                e.getClass().getName(),
                e.getMessage()
            )
        );
        LoggerUtils.logFailedRequest(logger, errorObject, e);
        throw new ServerException(errorObject);
      }
      JWSVerifier verifier = new RSASSAVerifier(key.getPublicKey());
      String idTokenString = JSONUtils.getString(jsonContent, "id_token");
      SignedJWT idTokenJWT = SignedJWT.parse(idTokenString);
      boolean idTokenVerified = idTokenJWT.verify(verifier);
      if (!idTokenVerified) {
        ErrorObject errorObject = ErrorObject.invalidGrant("Error: Unverifiable ID Token");
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }

      String accessTokenString = JSONUtils.getString(jsonContent, "access_token");
      SignedJWT accessTokenJWT = SignedJWT.parse(accessTokenString);
      boolean accessTokenVerified = accessTokenJWT.verify(verifier);
      if (!accessTokenVerified) {
        ErrorObject errorObject = ErrorObject.invalidGrant("Error: Unverifiable Access Token");
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }

      // store the external jwt token content in the current session
      this.sessionManager.setExternalJWTContent(session, content);
      CSPToken idToken = new CSPToken(TokenClass.ID_TOKEN, idTokenJWT);
      CSPToken accessToken = new CSPToken(TokenClass.ACCESS_TOKEN, idTokenJWT);

      validateIssuer(idToken, key.getIssuer());
      validateExpiration(idToken);

      return Pair.of(idToken, accessToken);
    }
  }
}