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

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.ParseException;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.JSONUtils;

import net.minidev.json.JSONObject;

public class FederationRelayState {
  private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederationRelayState.class);

  private String issuer;
  private String redirectURL;
  private String clientId;
  private String encodedValue;

  private FederationRelayState(String issuer, String clientId, String redirectURL, String encodedValue) {
    this.issuer = issuer;
    this.clientId = clientId;
    this.redirectURL = redirectURL;
    this.encodedValue = encodedValue;
  }

  public String getIssuer() {
    return issuer;
  }

  public String getClientId() {
    return clientId;
  }

  public String getRedirectURL() {
    return redirectURL;
  }

  public String getEncodedValue() {
    return encodedValue;
  }

  public static FederationRelayState build(String base64EncodedState) throws Exception {
    try {
      String urlDecodedState = URLDecoder.decode(base64EncodedState, "UTF-8");
      JSONObject jsonContent = JSONUtils.parseJSONObject(
          Base64Utils.decodeToString(
              urlDecodedState
          ));
      String issuer = JSONUtils.getString(jsonContent, "issuer");
      if (issuer == null || issuer.isEmpty()) {
        throw new ServerException(ErrorObject.invalidRequest("Issuer is invalid"));
      }
      String redirectURL = JSONUtils.getString(jsonContent, "redirect_uri");
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
      String clientId = JSONUtils.getString(jsonContent, "client_id");
      if (clientId == null || clientId.isEmpty()) {
        throw new ServerException(ErrorObject.invalidRequest("client id is invalid"));
      }
      return new FederationRelayState(issuer, clientId, redirectURL, base64EncodedState);
    } catch (ParseException e) {
      throw new Exception("failed to parse json response", e);
    }
  }
}
