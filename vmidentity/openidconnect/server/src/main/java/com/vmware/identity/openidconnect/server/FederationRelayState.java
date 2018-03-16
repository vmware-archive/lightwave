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
import java.net.URISyntaxException;
import java.net.URLDecoder;
import java.net.URLEncoder;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang3.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.JSONUtils;

import net.minidev.json.JSONObject;

public class FederationRelayState {
  private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederationRelayState.class);

  private static final String PARAM_TENANT = "tenant";
  private static final String PARAM_ISSUER = "issuer";
  private static final String PARAM_CLIENT_ID = "client_id";
  private static final String PARAM_REDIRECT_URI = "redirect_uri";
  private static final String PARAM_STATE = "state";
  private static final String PARAM_NONCE = "nonce";
  private static final String PARAM_SCOPE = "scope";
  private static final String PARAM_RESPONSE_TYPE = "response_type";
  private static final String PARAM_RESPONSE_MODE = "response_mode";

  private String tenant;
  private String issuer;
  private String redirectURL;
  private String clientId;
  private String nonce;
  private String spInitiatedState;
  private String scope;
  private String responseMode;
  private String responseType;
  private String encodedValue;

  private FederationRelayState(Builder builder) {
      this.tenant = builder.tenant;
      this.issuer = builder.issuer;
      this.clientId = builder.clientId;
      this.redirectURL = builder.redirectURL;
      this.encodedValue = builder.encodedValue;
      this.nonce = builder.nonce;
      this.spInitiatedState = builder.spInitiatedState;
      this.scope = builder.scope;
      this.responseMode = builder.responseMode;
      this.responseType = builder.responseType;
  }

  public String getTenant() {
      return tenant;
  }

  public String getIssuer() {
    return issuer;
  }

  public String getClientId() {
    return clientId;
  }

  public String getRedirectURI() {
    return redirectURL;
  }

  public String getEncodedValue() {
    return encodedValue;
  }

  public String getSPInitiatedState() {
      return spInitiatedState;
  }

  public String getNonce() {
      return nonce;
  }

  public String getScope() {
      return scope;
  }

  public String getResponseType() {
      return responseType;
  }

  public String getResponseMode() {
      return responseMode;
  }

  public static FederationRelayState build(String base64EncodedState) throws Exception {
    try {
      String urlDecodedState = URLDecoder.decode(base64EncodedState, "UTF-8");
      JSONObject jsonContent = JSONUtils.parseJSONObject(
              Base64Utils.decodeToString(
                  urlDecodedState
              ));

      String issuer = JSONUtils.getString(jsonContent, PARAM_ISSUER);
      if (issuer == null || issuer.isEmpty()) {
        throw new ServerException(ErrorObject.invalidRequest("Issuer is invalid"));
      }
      String redirectURL = JSONUtils.getString(jsonContent, PARAM_REDIRECT_URI);
      if (redirectURL == null || redirectURL.isEmpty()) {
        throw new ServerException(ErrorObject.invalidRequest("client redirect uri is invalid"));
      }
      try {
        URI testRedirectURI = new URI(redirectURL);
      } catch (URISyntaxException e) {
        ErrorObject errorObject = ErrorObject.invalidRequest(
                                      String.format(
                                          "client redirect uri is invalid. %s: %s",
                                          e.getClass().getName(),
                                          e.getMessage()
                                      )
                                  );
        LoggerUtils.logFailedRequest(logger, errorObject, e);
        throw new ServerException(errorObject);
      }
      String clientId = JSONUtils.getString(jsonContent, PARAM_CLIENT_ID);
      if (clientId == null || clientId.isEmpty()) {
        throw new ServerException(ErrorObject.invalidRequest("client id is invalid"));
      }

      Builder builder = new Builder(issuer, clientId, redirectURL);

      if (JSONUtils.hasKey(jsonContent, PARAM_TENANT)) {
          String tenant = JSONUtils.getString(jsonContent, PARAM_TENANT);
          builder.tenant(tenant);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_NONCE)) {
          String nonce = JSONUtils.getString(jsonContent, PARAM_NONCE);
          builder.nonce(nonce);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_STATE)) {
          String state = JSONUtils.getString(jsonContent, PARAM_STATE);
          builder.spInitiatedState(state);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_SCOPE)) {
          String scope = JSONUtils.getString(jsonContent, PARAM_SCOPE);
          builder.scope(scope);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_RESPONSE_MODE)) {
          String responseMode = JSONUtils.getString(jsonContent, PARAM_RESPONSE_MODE);
          builder.responseMode(responseMode);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_RESPONSE_TYPE)) {
          String responseType = JSONUtils.getString(jsonContent, PARAM_RESPONSE_TYPE);
          builder.responseType(responseType);
      }

      builder.encodedValue(base64EncodedState);
      return new FederationRelayState(builder);
    } catch (ParseException e) {
      throw new Exception("failed to parse json response", e);
    }
  }

  public static class Builder {
      private String tenant;
      private String issuer;
      private String redirectURL;
      private String clientId;
      private String nonce;
      private String spInitiatedState;
      private String scope;
      private String responseMode;
      private String responseType;
      private String encodedValue;

      public Builder(String issuer, String clientId, String redirectURL) {
          Validate.notEmpty(issuer, "issuer");
          Validate.notEmpty(clientId, "clientId");
          Validate.notEmpty(redirectURL, "redirect url");

          this.issuer = issuer;
          this.clientId = clientId;
          this.redirectURL = redirectURL;
      }

      public Builder tenant(String tenant) {
          this.tenant = tenant;
          return this;
      }

      public Builder nonce(String nonce) {
          this.nonce = nonce;
          return this;
      }

      public Builder spInitiatedState(String spInitiatedState) {
          this.spInitiatedState = spInitiatedState;
          return this;
      }

      public Builder scope(String scope) {
          this.scope = scope;
          return this;
      }

      public Builder responseMode(String responseMode) {
          this.responseMode = responseMode;
          return this;
      }

      public Builder responseType(String responseType) {
          this.responseType = responseType;
          return this;
      }

      public Builder encodedValue(String encodedValue) {
          this.encodedValue = encodedValue;
          return this;
      }

      public FederationRelayState build() throws Exception {
          JSONObject json = new JSONObject();

          json.put(PARAM_ISSUER, issuer);
          json.put(PARAM_CLIENT_ID, clientId);
          json.put(PARAM_REDIRECT_URI, redirectURL);

          if(StringUtils.isNotEmpty(tenant)) {
              json.put(PARAM_TENANT, tenant);
          }

          if(StringUtils.isNotEmpty(nonce)) {
              json.put(PARAM_NONCE, nonce);
          }

          if (StringUtils.isNotEmpty(scope)) {
              json.put(PARAM_SCOPE, scope);
          }

          if (StringUtils.isNotEmpty(spInitiatedState)) {
              json.put(PARAM_STATE, spInitiatedState);
          }

          if (StringUtils.isNotEmpty(responseMode)) {
              json.put(PARAM_RESPONSE_MODE, responseMode);
          }

          if (StringUtils.isNotEmpty(responseType)) {
              json.put(PARAM_RESPONSE_TYPE, responseType);
          }

          if (StringUtils.isEmpty(encodedValue)) {
              encodedValue = URLEncoder.encode(Base64Utils.encodeToString(json.toJSONString()), "UTF-8");
          }

          return new FederationRelayState(this);
      }
  }
}