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

import org.apache.commons.codec.binary.Base64;
import org.oasis_open.docs.ws_sx.ws_trust._200512.BinaryExchangeType;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;
import com.vmware.identity.sts.idm.InvalidCredentialsException;
import com.vmware.identity.sts.idm.SystemException;

/**
 * Insert your comment for BETAuthenticator here
 */
final class BETAuthenticator implements Authenticator {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(BETAuthenticator.class);
   // TODO [848547] move WST constants out of the authenticators
   static final String WST_VALUE_TYPE = "http://schemas.xmlsoap.org/ws/2005/02/trust/spnego";
   static final String WST_ENCODING_TYPE = "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary";

   private final com.vmware.identity.sts.idm.Authenticator idmAuth;

   BETAuthenticator(com.vmware.identity.sts.idm.Authenticator idmAuth) {
      assert idmAuth != null;

      this.idmAuth = idmAuth;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   public Result authenticate(Request req)
      throws AuthenticationFailedException, UnsupportedSecurityTokenException,
      NoSuchIdPException, RequestFailedException {

      assert req != null && req.getRst() != null;
      log.trace("Authenticating by BET...");

      final BinaryExchangeType beToken = req.getRst().getBinaryExchange();
      if (beToken == null || beToken.getValue() == null) {
         log.debug("No BET found!");
         return null;
      }

      String context = req.getRst().getContext();
      final GSSResult gssResult;
      try {
         gssResult = idmAuth.authenticate(context, retrieveInitiatorLeg(beToken));
      } catch (InvalidCredentialsException e) {
         throw new com.vmware.identity.sts.InvalidCredentialsException(
            "IDM rejected authentication by GSS", e);
      } catch (SystemException e) {
         throw new RequestFailedException(
            "IDM threw unexpected error during authentication", e);
      }

      final Result result = gssResult.complete() ? new Result(
         gssResult.getPrincipalId(), new Date(), AuthnMethod.KERBEROS, gssResult.getServerLeg())
         : new Result(gssResult.getServerLeg());

      if (result.completed()) {
         log.debug("Authenticated principal: {} at time {}: ",
            result.getPrincipalId(), result.getAuthnInstant());
      } else {
         log.debug(
            "The GSS security context is not yet established. Returning a server leg with length {}",
            result.getServerLeg().length);
      }
      return result;
   }

   private byte[] retrieveInitiatorLeg(BinaryExchangeType beToken)
      throws UnsupportedSecurityTokenException {
      assert beToken != null && beToken.getValue() != null;

      checkSupported(beToken.getValueType(), WST_VALUE_TYPE, "value type");
      checkSupported(beToken.getEncodingType(), WST_ENCODING_TYPE,
         "encoding type");

      // TODO revise the asymmetry in encodings of server (out) and initiator
      // (in) legs. The latter comes base64 encoded while the former goes out in
      // raw form. Is not possible to decode the initiator leg in WST layer?
      return Base64.decodeBase64(beToken.getValue());
   }

   private void checkSupported(String valueType, String supportedType,
      String type) throws UnsupportedSecurityTokenException {
      assert supportedType != null;
      assert type != null;

      if (!supportedType.equals(valueType)) {
         throw new UnsupportedSecurityTokenException(
            "Missing or unsupported BET " + type);
      }
   }
}
