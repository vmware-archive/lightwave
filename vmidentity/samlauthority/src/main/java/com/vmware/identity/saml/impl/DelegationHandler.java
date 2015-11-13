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

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.DelegationException;
import com.vmware.identity.saml.InvalidPrincipalException;
import com.vmware.identity.saml.PrincipalAttributesExtractor;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec.DelegationHistory;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;

/**
 * This class is responsible for all work related to token delegates
 */
final class DelegationHandler {

   private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
      .getLogger(DelegationHandler.class);

   private final int initialDelegationCount;
   private final PrincipalAttributesExtractor principalAttributesExtractor;

   /**
    * Creates handler of relegation requests.
    *
    * @param initialDelegationCount
    *           initial count of allowed delegations, not negative
    * @param principalAttributesExtractor
    *           necessary to check delegate validity, required
    */
   DelegationHandler(int initialDelegationCount,
      PrincipalAttributesExtractor principalAttributesExtractor) {
      assert initialDelegationCount >= 0;
      assert principalAttributesExtractor != null;

      this.initialDelegationCount = initialDelegationCount;
      this.principalAttributesExtractor = principalAttributesExtractor;

   }

   /**
    * @param requesterIsTokenOwner
    * @param delegationSpec
    *           required
    * @return the delegation info as it should be imprinted into the resulting
    *         token, nut null.
    * @throws DelegationException
    *            when delegation is not possible or the delegate is invalid
    */
   DelegationInfo getDelegationInfo(boolean requesterIsTokenOwner,
      DelegationSpec delegationSpec) {
      assert delegationSpec != null;

      checkActiveDelegate(delegationSpec);
      return new DelegationInfo(getDelegateList(delegationSpec),
         getDelegationCount(requesterIsTokenOwner, delegationSpec));
   }

   private void checkActiveDelegate(DelegationSpec spec) {
      if (delegationRequested(spec.getDelegate())) {
         boolean active = false;
         try {
            active = principalAttributesExtractor.isActive(spec.getDelegate());
         } catch (InvalidPrincipalException e) {
            // invalid principal -> non active
         }
         if (!active) {
            throw new DelegationException("Invalid delegate");
         }
      }

   }

   /**
    * @param requesterIsTokenOwner
    * @param spec
    *           required
    * @return the delegation count as it should be imprinted into the resulting
    *         token
    * @throws DelegationException
    *            when delegation is not possible
    */
   private int getDelegationCount(boolean requesterIsTokenOwner,
      DelegationSpec spec) {
      assert spec != null;

      int delegationCount = initialDelegationCount;

      if (!requesterIsTokenOwner) {
         assert spec.getDelegationHistory() != null;
         delegationCount = spec.getDelegationHistory()
            .getRemainingDelegations();
         log.debug("Current delegation chain have {} remaining delegations",
            delegationCount);
         // every subsequent delegation (after the initial) should decrement
         // the delegation count imprinted into the resulting token

         delegationCount = (delegationRequested(spec.getDelegate())) ? delegationCount - 1
            : delegationCount;
      }

      if (delegationCount < 0) {
         throw new DelegationException("Cannot continue delegation chain");
      }

      delegationCount = (spec.isDelegable()) ? delegationCount : 0;

      log.debug("Token final delegation count is {}", delegationCount);

      return delegationCount;
   }

   /**
    * @param spec
    *           required
    * @return the delegate list as it should be imprinted into the resulting
    *         token. not null - if no delegate list is present an empty array
    *         will be returned
    */
   private List<TokenDelegate> getDelegateList(DelegationSpec spec) {
      assert spec != null;

      List<TokenDelegate> delegateList = new ArrayList<TokenDelegate>();
      DelegationHistory history = spec.getDelegationHistory();
      if (history != null) {
         delegateList.addAll(history.getCurrentDelegateList());
      }

      PrincipalId delegate = spec.getDelegate();
      if (delegationRequested(delegate)) {
         delegateList.add(new TokenDelegate(delegate, new Date()));
      }

      log.debug("Token final delegate list is {}", delegateList);

      return delegateList;
   }

   /**
    * @param req
    * @return true if token delegation has been requested
    */
   private boolean delegationRequested(PrincipalId delegate) {
      return delegate != null;
   }
}
