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
package com.vmware.identity.sts.auth.impl;

import java.util.Date;

import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.AttributedString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UsernameTokenType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.idm.InvalidCredentialsException;
import com.vmware.identity.sts.idm.LockedUserAccountException;
import com.vmware.identity.sts.idm.PasswordExpiredException;
import com.vmware.identity.sts.idm.SystemException;

/**
 * Insert your comment for UNTAuthenticator here
 */
final class UNTAuthenticator implements Authenticator {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(UNTAuthenticator.class);

   private final com.vmware.identity.sts.idm.Authenticator idmAuth;

   UNTAuthenticator(com.vmware.identity.sts.idm.Authenticator idmAuth) {
      assert idmAuth != null;

      this.idmAuth = idmAuth;
   }

   @Override
   public Result authenticate(Request req)
      throws AuthenticationFailedException, UnsupportedSecurityTokenException,
      NoSuchIdPException, RequestFailedException {
      assert req != null;
      log.trace("Authenticating by UNT...");

      final UsernameTokenType usernameToken = retrieveUNT(req);
      if (usernameToken == null) {
         log.debug("No UNT found!");
         return null;
      }

      final String upn = extractUPN(usernameToken);
      assert upn != null;

      final String password = extractPassword(usernameToken);
      assert password != null;

      log.trace("Starting to authenticate principal: " + upn);
      final PrincipalId authenticatedPrincipal;
      try {
         authenticatedPrincipal = idmAuth.authenticate(upn, password);
      } catch (LockedUserAccountException e) {
         throw new com.vmware.identity.sts.InvalidCredentialsException(
            e.getMessage(), e, true);
      } catch (PasswordExpiredException e) {
         throw new com.vmware.identity.sts.InvalidCredentialsException(
            e.getMessage(), e, true);
      } catch (InvalidCredentialsException e) {
         throw new com.vmware.identity.sts.InvalidCredentialsException(
            "IDM rejected authentication by UPN", e);
      } catch (SystemException e) {
         throw new RequestFailedException(e);
      }

      final Date authenticationTime = new Date();
      log.debug("Authenticated principal: {} at time: {}",
         authenticatedPrincipal, authenticationTime);

      return new Result(authenticatedPrincipal, authenticationTime,
         Result.AuthnMethod.PASSWORD);
   }

   private UsernameTokenType retrieveUNT(Request req) {
      assert req != null && req.getHeader() != null;

      return req.getHeader().getUsernameToken() != null ? req.getHeader()
         .getUsernameToken() : null;
   }

   private String extractUPN(UsernameTokenType usernameToken)
      throws UnsupportedSecurityTokenException {
      assert usernameToken != null;

      return extractNotNullValue(usernameToken.getUsername(),
         "No user name found in UNT");
   }

   private String extractPassword(UsernameTokenType usernameToken)
      throws UnsupportedSecurityTokenException {
      assert usernameToken != null;

      return extractNotNullValue(usernameToken.getPassword(),
         "No password found in UNT");
   }

   private String extractNotNullValue(AttributedString valueContainer,
      String excMsg) throws UnsupportedSecurityTokenException {
      if (valueContainer == null || valueContainer.getValue() == null) {
         throw new UnsupportedSecurityTokenException(excMsg);
      }
      return valueContainer.getValue();
   }
}
