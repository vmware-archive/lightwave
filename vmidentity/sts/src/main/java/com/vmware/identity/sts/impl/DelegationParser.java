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

import java.util.ArrayList;
import java.util.List;

import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.ConditionAbstractType;
import oasis.names.tc.saml._2_0.assertion.ConditionsType;
import oasis.names.tc.saml._2_0.assertion.ProxyRestrictionType;

import org.oasis_open.docs.ws_sx.ws_trust._200512.DelegateToType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;

import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SamlTokenDelegate;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec.DelegationHistory;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;
import com.vmware.identity.sts.InvalidRequestException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.idm.PrincipalDiscovery;
import com.vmware.identity.sts.idm.SystemException;

/**
 * This class is responsible for extracting everything related to token
 * delegation from the request
 */
public final class DelegationParser {

   private final PrincipalDiscovery principalDiscovery;

   /**
    * @param config
    *           required
    * @param principalDiscovery
    *           required
    */
   public DelegationParser(PrincipalDiscovery principalDiscovery) {
      assert principalDiscovery != null;
      this.principalDiscovery = principalDiscovery;
   }

   /**
    * Extract delegation history from request containing SAML token.
    *
    * @param token
    *           input token, required
    * @param assertion
    *           input token in form of JAXB assertion, required
    *
    * @return not null delegation history of the authentication token. A history
    *         is created nevertheless token is delegated or not.
    */
   public DelegationHistory extractDelegationHistory(ServerValidatableSamlToken token,
      AssertionType assertion) {
      assert token != null : "DelegationParser.extractDelegationHistory() - token must not be null";
      assert assertion != null : "DelegationParser.extractDelegationHistory() - assertion must not be null";
      // TODO [849937] pass in one token representation only

      return new DelegationHistory(
          token.getSubject().subjectUpn(),
          getDelegateList(token.getDelegationChain()),
          parseCurrentDelegationCount(assertion), token.getExpirationTime());
   }

   /**
    * @param rst
    *           required
    * @return the solution user corresponding to the value of wst:delegateTo
    *         field. could be null
    * @throws InvalidRequestException
    *            if the solution user in delegateTo field is not found
    * @throws NoSuchIdPException
    *            if the underlying idp cannot be found
    * @throws RequestFailedException
    *            if the delegate cannot be found due to a system error
    */
   public SolutionUser extractDelegate(RequestSecurityTokenType rst)
      throws InvalidRequestException, RequestFailedException,
      NoSuchIdPException {
      assert rst != null;

      SolutionUser result = null;
      DelegateToType delegateTo = rst.getDelegateTo();
      if (delegateTo != null) {
         final String username = delegateTo.getUsernameToken().getUsername()
            .getValue();

         try {
            result = principalDiscovery.findSolutionUserByName(username);
         } catch (SystemException e) {
            throw new RequestFailedException(
               "Error occured looking for solution user with name " + username,
               e);
         }

         if (result == null) {
            throw new InvalidRequestException("Invalid delegate");
         }
      }

      return result;
   }

   private int parseCurrentDelegationCount(AssertionType assertion) {
      assert assertion != null;

      // according to schema conditions cannot be null
      ConditionsType conditions = assertion.getConditions();
      assert conditions != null;
      return getAuthTokenDelegationCount(conditions
         .getConditionOrAudienceRestrictionOrOneTimeUseOrProxyRestriction());
   }

   private List<TokenDelegate> getDelegateList(
      List<SamlTokenDelegate> authTokenDelChain) {
      assert authTokenDelChain != null;

      List<TokenDelegate> delegateList = new ArrayList<TokenDelegate>();
      for (SamlTokenDelegate delegate : authTokenDelChain) {
         delegateList.add(convert(delegate));
      }

      return delegateList;
   }

   /**
    * @param delegate
    *           required
    * @return
    */
   private TokenDelegate convert(
      SamlTokenDelegate delegate) {
      assert delegate != null;
      return new TokenDelegate(delegate.subject().subjectUpn(), delegate.delegationInstant());
   }

   /**
    * @param conditions
    *           could be null
    * @return the delegation count of the token which is used for subject
    *         authentication in the wst:request
    */
   private int getAuthTokenDelegationCount(
      List<ConditionAbstractType> conditions) {
      int delegationCount = 0;
      if (conditions != null) {
         for (ConditionAbstractType condition : conditions) {
            if (condition instanceof ProxyRestrictionType) {
               // TODO [848537] raise an exception when the number > MAX_INT. We
               // definitely will not support them! The schema type is
               // nonNegativeInteger. We may want to double check < 0 here also.
               delegationCount = ((ProxyRestrictionType) condition).getCount()
                  .intValue();
            }
         }
      }
      return delegationCount;
   }
}
