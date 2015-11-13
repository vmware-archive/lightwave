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

import java.util.List;

import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;

/**
 * This will represent the info which will be embedded into the issued token.
 */
final class DelegationInfo {

   private final int delegationCount;
   private final List<TokenDelegate> delegationChain;

   /**
    * @param delegationChain
    *           the list of current entities in delegation chain, not null but
    *           possibly empty.
    * @param delegationCount
    *           number of times the issued token can be delegated yet.
    *           Non-negative.
    */
   public DelegationInfo(List<TokenDelegate> delegationChain,
      int delegationCount) {
      assert delegationChain != null;
      assert delegationCount >= 0;

      this.delegationChain = delegationChain;
      this.delegationCount = delegationCount;
   }

   /**
    * @return delegationCount of the issued token. Non-negative.
    */
   public int getDelegationCount() {
      return delegationCount;
   }

   /**
    * @return the delegation chain to be included in the issued token, not null.
    */
   public List<TokenDelegate> getDelegationChain() {
      return delegationChain;
   }

   /**
    * @return if the issued token will be delegable.
    */
   public boolean isDelegableToken() {
      return delegationCount > 0;
   }

   /**
    * Gets the last delegate.
    *
    * @return last delegate, null if the token is not delegated
    */
   public TokenDelegate lastDelegate() {
      return delegationChain.isEmpty() ? null : delegationChain
         .get(delegationChain.size() - 1);
   }
}
