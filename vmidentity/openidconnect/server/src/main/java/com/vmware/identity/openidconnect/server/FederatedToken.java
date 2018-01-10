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

import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.openidconnect.common.ErrorObject;
import com.vmware.identity.openidconnect.common.TokenClass;
import com.vmware.identity.openidconnect.protocol.JWTUtils;

public class FederatedToken {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(FederatedToken.class);

    private static final String CLAIM_CONTEXT_NAME = "context_name";
    private static final String CLAIM_USER_NAME = "username";
    private static final String CLAIM_EMAIL = "email";
    private static final String CLAIM_PERMS = "perms";

    private final SignedJWT idToken;
    private final SignedJWT accessToken;

    FederatedToken(SignedJWT idToken, SignedJWT accessToken) {
        this.idToken = idToken;
        this.accessToken = accessToken;
    }

    public SignedJWT getIdToken() {
        return idToken;
    }

    public SignedJWT getAccessToken() {
        return accessToken;
    }

    public String getOrgId() throws Exception {
      JWTClaimsSet idTokenClaimsSet = idToken.getJWTClaimsSet();
      Map<String, Object> idTokenClaims = idTokenClaimsSet.getClaims();
      return (String) idTokenClaims.get(CLAIM_CONTEXT_NAME);
    }

    public String getUserId() throws Exception {
      return getClaimValue(CLAIM_USER_NAME);
    }

    public String getEmailAddress() throws Exception {
      return getClaimValue(CLAIM_EMAIL);
    }

    public Set<String> getPermissions() throws Exception {
      JWTClaimsSet accessTokenClaimsSet = accessToken.getJWTClaimsSet();
      if (accessTokenClaimsSet.getClaims().containsKey(CLAIM_PERMS)) {
        String[] permissions = JWTUtils.getStringArray(accessTokenClaimsSet, TokenClass.ACCESS_TOKEN, CLAIM_PERMS);
        return new HashSet<>(Arrays.asList(permissions));
      } else {
        return Collections.emptySet();
      }
    }

    public String getClaimValue(String claim) throws Exception {
      JWTClaimsSet idTokenClaimsSet = idToken.getJWTClaimsSet();
      Map<String, Object> idTokenClaims = idTokenClaimsSet.getClaims();
      return (String) idTokenClaims.get(claim);
    }

    public String getIssuer() throws Exception {
      return idToken.getJWTClaimsSet().getIssuer();
    }

    public Date getExpirationTime() throws Exception {
      return idToken.getJWTClaimsSet().getExpirationTime();
    }

    public void validateIssuer(String expectedIssuer) throws Exception {
      String issuer = getIssuer();
      if (issuer == null || !issuer.equalsIgnoreCase(expectedIssuer)) {
        ErrorObject errorObject = ErrorObject.accessDenied("Issuer does not match");
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }
    }

    public void validateExpiration() throws Exception {
      Date expiryTime = getExpirationTime();
      Date now = new Date();
      if (now.after(expiryTime)) {
        ErrorObject errorObject = ErrorObject.accessDenied("token expired");
        LoggerUtils.logFailedRequest(logger, errorObject);
        throw new ServerException(errorObject);
      }
    }
  }
