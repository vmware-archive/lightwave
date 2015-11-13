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

import java.math.BigInteger;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;

import oasis.names.tc.saml._2_0.assertion.AssertionType;
import oasis.names.tc.saml._2_0.assertion.ConditionsType;
import oasis.names.tc.saml._2_0.assertion.ProxyRestrictionType;

import org.oasis_open.docs.ws_sx.ws_trust._200512.DelegateToType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.LifetimeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ParticipantType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.ParticipantsType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RenewTargetType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.AttributedString;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UsernameTokenType;
import org.w3._2005._08.addressing.AttributedURIType;
import org.w3._2005._08.addressing.EndpointReferenceType;

import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.util.TimePeriod;

/**
 * Insert your comment for TestSetup here
 */
final class TestSetup {

   static RequestSecurityTokenType createRST(String delegate,
      AssertionType renewToken) {
      return createRST(delegate, renewToken, null,
         Collections.<String> emptySet());
   }

   static RequestSecurityTokenType createRST(String delegate,
           AssertionType renewToken, SignatureAlgorithm signatureAlgorithm,
           Collection<String> audience){
       return createRST(delegate, renewToken, signatureAlgorithm, null, audience);
   }
   static RequestSecurityTokenType createRST(String delegate,
      AssertionType renewToken, SignatureAlgorithm signatureAlgorithm,
      String primaryAudience, Collection<String> audience) {
      assert audience != null;

      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      if (signatureAlgorithm != null) {
         rst.setSignatureAlgorithm(signatureAlgorithm.toString());
      }
      if (delegate != null) {
         DelegateToType delegateTo = new DelegateToType();
         UsernameTokenType usernameToken = new UsernameTokenType();
         AttributedString username = new AttributedString();
         username.setValue(delegate);
         usernameToken.setUsername(username);
         delegateTo.setUsernameToken(usernameToken);
         rst.setDelegateTo(delegateTo);
      }

      if (renewToken != null) {
         final RenewTargetType renewTarget = new RenewTargetType();
         renewTarget.setAssertion(renewToken);
         rst.setRenewTarget(renewTarget);
      }
      setAudience(rst, primaryAudience, audience);
      return rst;
   }

   static RequestSecurityTokenType setLifetime(RequestSecurityTokenType rst,
      Date start, Date end) {
      final LifetimeType lifetime = LifetimeConvertor
         .toResponseLifetime(new TimePeriod(start, end));
      rst.setLifetime(lifetime);
      return rst;
   }

   static AssertionType createAssertion(int authTokenDelCount) {
      AssertionType assertion = new AssertionType();
      ProxyRestrictionType proxyRestrictionType = new ProxyRestrictionType();
      proxyRestrictionType.setCount(BigInteger.valueOf(authTokenDelCount));
      ConditionsType conditions = new ConditionsType();
      conditions.getConditionOrAudienceRestrictionOrOneTimeUseOrProxyRestriction().add(
         proxyRestrictionType);
      assertion.setConditions(conditions);
      return assertion;
   }

   private static void setAudience(RequestSecurityTokenType rst,
      String primary,
      Collection<String> audience) {
       final ParticipantsType participants = new ParticipantsType();
       if( primary != null ){
           final ParticipantType participant = new ParticipantType();
           final EndpointReferenceType ref = new EndpointReferenceType();
           final AttributedURIType attrValue = new AttributedURIType();
           attrValue.setValue(primary);
           ref.setAddress(attrValue);
           participant.setEndpointReference(ref);
           participants.setPrimary(participant);
      }
      for (String audienceParty : audience) {
         final ParticipantType participant = new ParticipantType();
         final EndpointReferenceType ref = new EndpointReferenceType();
         final AttributedURIType attrValue = new AttributedURIType();
         attrValue.setValue(audienceParty);
         ref.setAddress(attrValue);
         participant.setEndpointReference(ref);
         participants.getParticipant().add(participant);
      }
      rst.setParticipants(participants);
   }
}
