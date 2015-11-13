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
package com.vmware.identity.saml.impl;

import com.vmware.identity.saml.RenewException;
import com.vmware.identity.saml.SamlTokenSpec.RenewSpec;

/**
 * Immutable entity that determines token renew-ability processing issue and
 * renew requests.
 */
final class RenewRestrictor {

   private final boolean tokenOwner;
   private final int initialCount;

   /**
    * Creates an instance specific to token requester, either token owner or
    * token delegate.
    *
    * @param tokenOwner
    *           whether the requester is token owner
    * @param initialCount
    *           initial renew count
    */
   RenewRestrictor(boolean tokenOwner, int initialCount) {
      assert initialCount >= 0;
      this.tokenOwner = tokenOwner;
      this.initialCount = initialCount;
   }

   /**
    * Determines whether the request could be processed and if yes, calculates
    * the new renew counter.
    *
    * @param spec
    *           renew specification, required
    * @return number of renews left, not negative
    * @throws RenewException
    *            if renew request cannot be satisfied when the token is not
    *            renew-able.
    */
   int processRequest(RenewSpec spec) {
      assert spec != null;

      int result = tokenOwner ? initialCount : spec.getRemainingRenewables();
      if (!tokenOwner && spec.isRenew()) {
         result--;
      }

      if (result < 0) {
         throw new RenewException("Unable to renew non-renewable token");
      }

      result = spec.isRenewable() ? result : 0;
      assert result >= 0;
      return result;
   }
}
