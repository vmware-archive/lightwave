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
import java.security.KeyFactory;
import java.security.interfaces.RSAPublicKey;
import java.security.spec.X509EncodedKeySpec;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;

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
import com.vmware.identity.idm.PersonDetail;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.HttpRequest;
import com.vmware.identity.openidconnect.protocol.HttpResponse;
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

  @Autowired
  private CasIdmClient idmClient;

  @Autowired
  private AuthorizationCodeManager authzCodeManager;

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
    FederatedToken token = getToken(authenticode, relayState, idpConfig, session);
    // Process the response
    // TODO: Nonce and state handling
    return processResponse(token, idpConfig, relayState, session);
  }

  private FederatedToken
  getToken(String code, FederationRelayState relayState, IDPConfig idpConfig, SessionID session) throws Exception {
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
  processResponse(FederatedToken token, IDPConfig idpConfig, FederationRelayState state, SessionID session) throws Exception {
    String tenantName = token.getOrgId();
    if (tenantName == null || tenantName.isEmpty()) {
      ErrorObject errorObject = ErrorObject.invalidRequest("Invalid tenant name");
      LoggerUtils.logFailedRequest(logger, errorObject);
      throw new ServerException(errorObject);
    }
    String emailAddress = token.getEmailAddress();
    if (emailAddress == null || emailAddress.isEmpty()) {
      ErrorObject errorObject = ErrorObject.invalidRequest("Invalid email address");
      LoggerUtils.logFailedRequest(logger, errorObject);
      throw new ServerException(errorObject);
    }

    TenantInfo tenantInfo = getTenantInfo(tenantName);
    if (tenantInfo == null) {
      Set<String> permissions = token.getPermissions();
      // Create the tenant only if the incoming user has an Organization Admin Role
      if (permissions.contains(ROLE_CSP_ORG_OWNER)) {
        String entityId = state.getIssuer();
        createTenant(tenantName, entityId, idpConfig);
      } else {
        ErrorObject errorObject = ErrorObject.accessDenied(
            String.format("Tenant [%s] does not exist", tenantName)
        );
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }
    }

    String systemDomain = getSystemDomainName(tenantName);
    if (systemDomain == null || systemDomain.isEmpty()) {
      throw new ServerException(ErrorObject.serverError("The system domain is invalid"));
    }

    PrincipalId user = buildPrincipal(emailAddress, tenantName);
    if (!isActive(tenantName, user)) {
      String entityId = state.getIssuer();
      this.idmClient.registerUpnSuffix(tenantName, systemDomain, user.getDomain());
      // generate a unique user name in order not to conflict with local users
      String userName = user.getName() + "-" + user.getDomain();
      this.idmClient.addJitUser(
          tenantName,
          userName,
          new PersonDetail.Builder()
              .userPrincipalName(user.getUPN())
              .description("A JIT user account created for federated IDP.").build(),
          entityId,
          emailAddress
      );
      // If the user is a service owner, add the user to the Administrators group
      Set<String> permissions = token.getPermissions();
      if (permissions.contains(ROLE_CSP_ORG_OWNER)) {
          this.idmClient.addUserToGroup(tenantName, user, "Administrators");
      }
    }

    ClientInfoRetriever clientInfoRetriever = new ClientInfoRetriever(idmClient);
    ClientID clientID = new ClientID(state.getClientId());
    ClientInfo clientInfo = clientInfoRetriever.retrieveClientInfo(systemDomain, clientID);

    Cookie sessionCookie = loggedInSessionCookie(tenantName, session);
    Cookie cspIssuerCookie = CSPIssuerCookie(tenantName, state.getIssuer());
    sessionManager.update(session, new PersonUser(user, tenantName), LoginMethod.PASSWORD, clientInfo);

    String redirectURI = String.format("%s/%s", state.getRedirectURL(), tenantName);
    HttpResponse httpResponse = HttpResponse.createRedirectResponse(new URI(redirectURI));
    httpResponse.addCookie(sessionCookie);
    httpResponse.addCookie(cspIssuerCookie);

    return httpResponse;
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
      String publicKeyURL = idpConfig.getOidcConfig().getJwksURI();
      key = CSPTokenPublicKey.build(publicKeyURL);
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

  private boolean isActive(String tenantName, PrincipalId user) {
    try {
      return this.idmClient.isActive(tenantName, user);
    } catch (Exception ex) {
      return false;
    }
  }

  private PrincipalId buildPrincipal(String upn, String tenant) throws Exception {
    int pos = upn.indexOf("@");
    if (pos > 0) {
      String upnSuffix = upn.substring(pos + 1);
      // subject upn is the same as the upn is external token attribute
      return new PrincipalId(upn.substring(0, pos), upnSuffix);
    } else {
      return new PrincipalId(upn, tenant);
    }
  }

  private TenantInfo getTenantInfo(String tenantName) {
    try {
      TenantInfoRetriever tenantInfoRetriever = new TenantInfoRetriever(this.idmClient);
      return tenantInfoRetriever.retrieveTenantInfo(tenantName);
    } catch (Exception e) {
      return null;
    }
  }

  private TenantInfo createTenant(String tenantName, String entityId, IDPConfig idpConfigSystem) throws Exception {
    Tenant tenant = new Tenant(tenantName);
    this.idmClient.addTenant(
        tenant,
        "Administrator",
        this.idmClient.generatePassword(idmClient.getSystemTenant()).toCharArray()
    );
    this.idmClient.setTenantCredentials(tenantName);
    IDPConfig idpConfig = new IDPConfig(
        entityId,
        IDPConfig.IDP_PROTOCOL_OAUTH_2_0
    );
    idpConfig.setOidcConfig(idpConfigSystem.getOidcConfig());
    idpConfig.setJitAttribute(true);
    this.idmClient.setExternalIdpConfig(tenantName, idpConfig);
    return getTenantInfo(tenantName);
  }


  public FederatedToken
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
                      "Failed to retrieve token from [%s]",
                      target.toString()
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
      FederatedToken token = new FederatedToken(idTokenJWT, accessTokenJWT);

      token.validateIssuer(key.getIssuer());
      token.validateExpiration();

      return token;
    }
  }

  static class CSPTokenPublicKey implements FederatedTokenPublicKey {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(CSPTokenPublicKey.class);

    private String _issuer;
    private RSAPublicKey _publicKey;

    public CSPTokenPublicKey(String issuer, RSAPublicKey publicKey) {
      _issuer = issuer;
      _publicKey = publicKey;
    }

    @Override
    public String getIssuer() {
      return _issuer;
    }

    @Override
    public RSAPublicKey getPublicKey() throws Exception {
      return _publicKey;
    }

    public static FederatedTokenPublicKey build(String publicKeyURL) throws Exception {
      URI target = new URI(publicKeyURL);
      HttpRequest request = HttpRequest.createGetRequest(target);
      HttpRequestBase httpRequestBase = request.toHttpTask();

      try (CloseableHttpClient client = HttpClients.createDefault();
           CloseableHttpResponse response = client.execute(httpRequestBase);) {
        int statusCodeInt = response.getStatusLine().getStatusCode();
        StatusCode statusCode;
        try {
          statusCode = StatusCode.parse(statusCodeInt);
          if (statusCode != StatusCode.OK) {
            ErrorObject errorObject =
                ErrorObject.serverError(
                    String.format(
                        "Failed to retrieve public key from [%s]",
                        publicKeyURL
                    )
                );
            LoggerUtils.logFailedRequest(logger, errorObject);
            throw new ServerException(errorObject);
          }
        } catch (ParseException e) {
          ErrorObject errorObject =
              ErrorObject.serverError(
                String.format(
                    "failed to parse status code. %s:%s",
                      e.getClass().getName(),
                      e.getMessage()
                )
              );
          throw new ServerException(errorObject);
        }

        HttpEntity httpEntity = response.getEntity();
        if (httpEntity == null) {
          ErrorObject errorObject =
              ErrorObject.serverError(
                      "failed to retrieve public key"
              );
          throw new ServerException(errorObject);
        }

        ContentType contentType;
        try {
          contentType = ContentType.get(httpEntity);
        } catch (UnsupportedCharsetException | org.apache.http.ParseException e) {
          ErrorObject errorObject =
              ErrorObject.serverError(
                  "Error in setting content type in HTTP response"
              );
          throw new ServerException(errorObject);
        }

        Charset charset = contentType.getCharset();
        if (charset != null && !StandardCharsets.UTF_8.equals(charset)) {
          ErrorObject errorObject =
              ErrorObject.serverError(
                  String.format("unsupported charset: %s", charset)
              );
          throw new ServerException(errorObject);
        }

        if (!ContentType.APPLICATION_JSON.getMimeType().equalsIgnoreCase(contentType.getMimeType())) {
          ErrorObject errorObject =
              ErrorObject.serverError(
                  String.format(
                      "unsupported mime type: %s",
                      contentType.getMimeType()
                  )
              );
          throw new ServerException(errorObject);
        }

        try {
          String content = EntityUtils.toString(httpEntity);
          JSONObject jsonContent = JSONUtils.parseJSONObject(content);
          // Get the Issuer
          String issuer = JSONUtils.getString(jsonContent, "issuer");
          if (issuer == null || issuer.isEmpty()) {
            ErrorObject errorObject = ErrorObject.serverError(
                "Error: Invalid Issuer found for public key"
            );
            throw new ServerException(errorObject);
          }
          // Get the Algorithm
          String alg = JSONUtils.getString(jsonContent, "alg");
          if (alg == null || (!alg.equals("RSA") && !alg.equals("SHA256withRSA"))) {
            ErrorObject errorObject = ErrorObject.serverError(
                String.format(
                    "Error: No Handler for enclosed Key's Algorithm (%s)",
                    alg
                )
            );
            throw new ServerException(errorObject);
          }
          // Get the key material
          String val = JSONUtils.getString(jsonContent, "value");
          if (val == null || val.isEmpty()) {
            ErrorObject errorObject = ErrorObject.serverError(
                    "Error: Invalid key material for public key"
            );
            throw new ServerException(errorObject);
          }
          val = val.replaceAll("-----BEGIN PUBLIC KEY-----", "")
                   .replaceAll("-----END PUBLIC KEY-----", "")
                   .replaceAll("[\n\r]", "")
                   .trim();
          byte[] decoded = Base64Utils.decodeToBytes(val);
          X509EncodedKeySpec spec = new X509EncodedKeySpec(decoded);
          KeyFactory kf = KeyFactory.getInstance("RSA");
          return new CSPTokenPublicKey(issuer, (RSAPublicKey) kf.generatePublic(spec));
        } catch (ParseException e) {
          ErrorObject errorObject = ErrorObject.serverError(
              String.format(
                  "failed to get public key from federatd IDP. %s:%s",
                  e.getClass().getName(),
                  e.getMessage()
              )
          );
          LoggerUtils.logFailedRequest(logger, errorObject, e);
          throw new ServerException(errorObject);
        }
      }
    }
  }
}
