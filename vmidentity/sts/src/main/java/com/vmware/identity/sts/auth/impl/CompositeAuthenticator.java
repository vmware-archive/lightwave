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

import java.util.Set;

import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;

/**
 * Represents a composite authenticator. It allows caller to authenticate by
 * more than just one mechanisms. Each underlying authenticator uses its
 * specific mechanism and employs specific credentials to prove caller's
 * identity.
 *
 * This implementation picks up the latest (by authentication instant) complete
 * result, if any. If none, it returns the first incomplete result.
 */
final class CompositeAuthenticator implements Authenticator {

   private final Set<Authenticator> authenticators;

   CompositeAuthenticator(Set<Authenticator> authenticators) {
      assert authenticators != null && !authenticators.isEmpty();

      this.authenticators = authenticators;
   }

   @Override
   public Result authenticate(Request req)
      throws AuthenticationFailedException, UnsupportedSecurityTokenException,
      NoSuchIdPException, RequestFailedException {
      assert req != null;

      Result result = null;
      for (Authenticator authenticator : authenticators) {
         final Result lastResult = authenticator.authenticate(req);
         if (lastResult != null) {
            checkPrincipal(result, lastResult);
            result = reconcile(result, lastResult);
         }
      }
      return result;
   }

   private void checkPrincipal(Result result, Result lastResult)
      throws InvalidCredentialsException {
      assert lastResult != null;

      if (result != null && result.completed() && lastResult.completed()) {
         if (!result.getPrincipalId().equals(lastResult.getPrincipalId())) {
            throw new InvalidCredentialsException(
               "Successfully authenticated different principals "
                  + result.getPrincipalId() + " and "
                  + lastResult.getPrincipalId() + " in the current request!");
         }
      }
   }

   private Result reconcile(Result result, Result lastResult) {
      assert lastResult != null;

      return (result == null || (lastResult.completed() && (!result.completed() || lastResult
         .getAuthnInstant().after(result.getAuthnInstant())))) ? lastResult
         : result;
   }
}
