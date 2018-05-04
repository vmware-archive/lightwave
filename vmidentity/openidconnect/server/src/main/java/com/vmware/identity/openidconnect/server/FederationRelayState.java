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

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.JSONUtils;

import net.minidev.json.JSONObject;

public class FederationRelayState {

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
  private State state;
  private Nonce nonce; // nonce used for external IDP request
  private String issuer;
  private String redirectURL;
  private String clientId;
  private String spInitiatedNonce;
  private String spInitiatedState;
  private String spInitiatedScope;
  private String spInitiatedResponseMode;
  private String spInitiatedResponseType;

  private FederationRelayState(Builder builder) {
      this.tenant = builder.tenant;
      this.state = builder.state;
      this.nonce = builder.nonce;
      this.issuer = builder.issuer;
      this.clientId = builder.clientId;
      this.redirectURL = builder.redirectURL;
      this.spInitiatedNonce = builder.spInitiatedNonce;
      this.spInitiatedState = builder.spInitiatedState;
      this.spInitiatedScope = builder.spInitiatedScope;
      this.spInitiatedResponseMode = builder.spInitiatedResponseMode;
      this.spInitiatedResponseType = builder.spInitiatedResponseType;
  }

  /**
   * Get the tenant name of the authentication request.
   *
   * @return tenant name
   */
  public String getTenant() {
      return tenant;
  }

  /**
   * Get the state of the delegated authentication request from Lightwave to Federated IDP.
   *
   * @return state per oidc protocol
   */
  public State getState() {
      return state;
  }

  /**
   * Get the nonce of the delegated authentication request from Lightwave to Federated IDP.
   *
   * @return nonce per oidc protocol
   */
  public Nonce getNonce() {
      return nonce;
  }

  /**
   * Get issuer of the Identity Provider.
   *
   * @return issuer per oidc protocol
   */
  public String getIssuer() {
    return issuer;
  }

  /**
   * Get Lightwave OIDC client id.
   *
   * @return client id
   */
  public String getClientId() {
    return clientId;
  }

  /**
   * Get redirect url where the tokens are sent.
   *
   * @return redirect url per oidc protocol
   */
  public String getRedirectURI() {
    return redirectURL;
  }

  /**
   * Get state value from SP initiated request.
   *
   * @return state per oidc protocol
   */
  public String getSPInitiatedState() {
      return spInitiatedState;
  }

  /**
   * Get nonce value from SP initiated request.
   *
   * @return nonce per oidc protocol
   */
  public String getSPInitiatedNonce() {
      return spInitiatedNonce;
  }

  /**
   * Get scope value from SP initiated request.
   *
   * @return scope per oidc protocol
   */
  public String getSPInitiatedScope() {
      return spInitiatedScope;
  }

  /**
   * Get responseType from SP initiated request.
   *
   * @return response type per oidc protocol
   */
  public String getSPInitiatedResponseType() {
      return spInitiatedResponseType;
  }

  /**
   * Get responseMode from SP initiated request.
   *
   * @return responseMode per oidc protocol
   */
  public String getSPInitiatedResponseMode() {
      return spInitiatedResponseMode;
  }

  public static FederationRelayState build(String base64EncodedState) throws ServerException {
    Validate.notEmpty(base64EncodedState, "Encoded state must not be null.");
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
      String clientId = JSONUtils.getString(jsonContent, PARAM_CLIENT_ID);
      if (clientId == null || clientId.isEmpty()) {
        throw new ServerException(ErrorObject.invalidRequest("client id is invalid"));
      }

      Builder builder = new Builder(issuer, clientId, redirectURL);

      if (JSONUtils.hasKey(jsonContent, PARAM_TENANT)) {
          String tenant = JSONUtils.getString(jsonContent, PARAM_TENANT);
          builder.withTenant(tenant);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_NONCE)) {
          String nonce = JSONUtils.getString(jsonContent, PARAM_NONCE);
          builder.withSPInitiatedNonce(nonce);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_STATE)) {
          String state = JSONUtils.getString(jsonContent, PARAM_STATE);
          builder.withSPInitiatedState(state);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_SCOPE)) {
          String scope = JSONUtils.getString(jsonContent, PARAM_SCOPE);
          builder.withSPInitiatedScope(scope);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_RESPONSE_MODE)) {
          String responseMode = JSONUtils.getString(jsonContent, PARAM_RESPONSE_MODE);
          builder.withSPInitiatedResponseMode(responseMode);
      }

      if (JSONUtils.hasKey(jsonContent, PARAM_RESPONSE_TYPE)) {
          String responseType = JSONUtils.getString(jsonContent, PARAM_RESPONSE_TYPE);
          builder.withSPInitiatedResponseType(responseType);
      }

      return builder.build();
    } catch (ParseException | UnsupportedEncodingException e) {
      throw new ServerException(ErrorObject.invalidRequest("Unable to parse relay state json content."), e);
    }
  }

  public static class Builder {
      private String tenant;
      private State state;
      private Nonce nonce;
      private String issuer;
      private String redirectURL;
      private String clientId;
      private String spInitiatedNonce;
      private String spInitiatedState;
      private String spInitiatedScope;
      private String spInitiatedResponseMode;
      private String spInitiatedResponseType;

      public Builder(String issuer, String clientId, String redirectURL) {
          Validate.notEmpty(issuer, "issuer");
          Validate.notEmpty(clientId, "clientId");
          Validate.notEmpty(redirectURL, "redirect url");

          this.issuer = issuer;
          this.clientId = clientId;
          this.redirectURL = redirectURL;
      }

      public Builder withTenant(String tenant) {
          Validate.notEmpty(tenant, "tenant must not be null");
          this.tenant = tenant;
          return this;
      }

      public Builder withState(State state) {
          Validate.notNull(state, "state must not be null.");
          this.state = state;
          return this;
      }

      public Builder withNonce(Nonce nonce) {
          Validate.notNull(nonce, "nonce must not be null.");
          this.nonce = nonce;
          return this;
      }

      public Builder withSPInitiatedNonce(String nonce) {
          this.spInitiatedNonce = nonce;
          return this;
      }

      public Builder withSPInitiatedState(String spInitiatedState) {
          this.spInitiatedState = spInitiatedState;
          return this;
      }

      public Builder withSPInitiatedScope(String scope) {
          this.spInitiatedScope = scope;
          return this;
      }

      public Builder withSPInitiatedResponseMode(String responseMode) {
          this.spInitiatedResponseMode = responseMode;
          return this;
      }

      public Builder withSPInitiatedResponseType(String responseType) {
          this.spInitiatedResponseType = responseType;
          return this;
      }

      public FederationRelayState build() {
          return new FederationRelayState(this);
      }
  }
}