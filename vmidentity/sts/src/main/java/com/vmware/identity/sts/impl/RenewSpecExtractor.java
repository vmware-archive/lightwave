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
package com.vmware.identity.sts.impl;

import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.ConditionAbstractType;
import oasis.names.tc.saml._2_0.assertion.ConditionsType;

import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewingType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;

import com.rsa.names._2009._12.std_ext.saml2.RenewRestrictionType;
import com.vmware.identity.saml.SamlTokenSpec.RenewSpec;

/**
 * Builds renew specifications extracting all necessary data from the input
 * token and request.
 */
final class RenewSpecExtractor {

   /**
    * Create specification that indicate request for token renew.
    *
    * @param inputToken
    *           required
    * @return specification to renew the given token, not null
    */
   static RenewSpec extractRenew(AssertionType inputToken) {
      assert inputToken != null;

      final boolean renewable = true;
      final boolean renew = true;
      return new RenewSpec(renewable, renew, parseRenewCount(inputToken));
   }

   /**
    * Create specification that indicate request for token issue.
    *
    * @param rst
    *           issue request, required
    * @param inputToken
    *           authentication token, if any, null otherwise
    *
    * @return specification to issue token, not null
    */
   static RenewSpec extractIssue(RequestSecurityTokenType rst,
      AssertionType inputToken) {
      assert rst != null;

      final boolean renewable = isRenewable(rst.getRenewing());
      final boolean renew = false;

      return (inputToken == null) ? new RenewSpec(renewable) : new RenewSpec(
         renewable, renew, parseRenewCount(inputToken));
   }

   private static boolean isRenewable(RenewingType renewing) {
      return (renewing != null && renewing.isAllow() != null) ? renewing
         .isAllow() : true;
   }

   private static int parseRenewCount(AssertionType assertion) {
      assert assertion != null;

      int renewCount = 0;
      // according to schema conditions cannot be null
      ConditionsType conditions = assertion.getConditions();
      assert conditions != null;
      for (ConditionAbstractType condition : conditions
         .getConditionOrAudienceRestrictionOrOneTimeUseOrProxyRestriction()) {
         if (condition instanceof RenewRestrictionType) {
            // TODO loss of precision
            // in XSD it is nonNegativeInteger
            renewCount = ((RenewRestrictionType) condition).getCount()
               .intValue();
         }
      }
      return renewCount;
   }

}
