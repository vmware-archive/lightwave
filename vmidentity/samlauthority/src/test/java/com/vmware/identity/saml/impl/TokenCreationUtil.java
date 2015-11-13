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

import static com.vmware.identity.saml.TestUtil.DEFAULT_MAXIMUM_TOKEN_LIFETIME;
import static com.vmware.identity.saml.TestUtil.TOKEN_DELEGATION_COUNT;
import static com.vmware.identity.saml.TestUtil.TOKEN_RENEW_COUNT;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Date;
import java.util.List;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SamlTokenSpec.Builder;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SamlTokenSpec.DelegationSpec.DelegationHistory;
import com.vmware.identity.saml.SamlTokenSpec.TokenDelegate;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.config.TokenRestrictions;
import com.vmware.identity.util.TimePeriod;

public class TokenCreationUtil {

   private static final PrincipalId DEFAULT_TOKEN_PRESENTER = new PrincipalId(
      "user", "example.com");
   private static final String SYSTEM_DOMAIN = "system-domain";
   private final static String UPN = "http://schemas.xmlsoap.org/claims/UPN";
   private final static String FIRST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/givenname";
   private final static String LAST_NAME = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/surname";
   private final static String GROUPS = "http://rsa.com/schemas/attr-names/2009/01/GroupIdentity";
   private final static String SUBJECT_TYPE = "http://vmware.com/schemas/attr-names/2011/07/isSolution";
   private final static String EMAIL = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress";

   private final static Collection<String> attributeNames = Arrays
      .asList(new String[] { FIRST_NAME, LAST_NAME, GROUPS, SUBJECT_TYPE, EMAIL });

   public static SamlTokenSpec createSpec(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan, UPN, attributeNames, null).createSpec();
   }

   public static SamlTokenSpec createSpec(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan, SignatureAlgorithm signatureAlgorithm) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan, UPN, attributeNames, signatureAlgorithm)
         .createSpec();
   }

   public static SamlTokenSpec createSpec(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan, String identityAttribute) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan, identityAttribute, attributeNames, null)
         .createSpec();
   }

   public static SamlTokenSpec createSpec(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan, String identityAttribute,
      SignatureAlgorithm signatureAlgorithm) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan, identityAttribute, attributeNames,
         signatureAlgorithm).createSpec();
   }

   public static SamlTokenSpec createSpec(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan, String identityAttribute,
      Collection<String> attributeNames) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan, identityAttribute, attributeNames, null)
         .createSpec();
   }

   public static SamlTokenSpec createSpec(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan, String identityAttribute,
      Collection<String> attributeNames, SignatureAlgorithm signatureAlgorithm) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan, identityAttribute, attributeNames,
         signatureAlgorithm).createSpec();
   }

   public static Builder createSpecBuilder(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan);
   }

   public static Builder createSpecBuilder(X509Certificate confirmationCert,
      Date authnTime, TimePeriod lifespan, String recipient, String inResponseTo) {
      return createSpecBuilder(DEFAULT_TOKEN_PRESENTER, confirmationCert,
         authnTime, lifespan, recipient, inResponseTo);
   }

   public static Builder createSpecBuilder(PrincipalId tokenPresenter,
      X509Certificate confirmationCert, Date authnTime, TimePeriod lifespan) {
      return createSpecBuilder(tokenPresenter, confirmationCert, authnTime,
         lifespan, UPN, attributeNames, null);
   }

   public static Builder createSpecBuilder(PrincipalId tokenPresenter,
      X509Certificate confirmationCert, Date authnTime, TimePeriod lifespan,
      String recipient, String inResponseTo) {
      return createSpecBuilder(tokenPresenter, confirmationCert, authnTime,
         lifespan, UPN, attributeNames, null, recipient, inResponseTo);
   }

   /**
    * If recipient or inResponseTo is present then a bearer token is created
    * (without confirmationCert).
    */
   public static Builder createSpecBuilder(PrincipalId tokenPresenter,
      X509Certificate confirmationCert, Date authnTime, TimePeriod lifespan,
      String identityAttribute, Collection<String> attributeNames,
      SignatureAlgorithm desiredSignatureAlgorithmInRequest, String recipient,
      String inResponseTo) {
      // TODO change confirmation if WS-Trust starts to support recipient and
      // inResponseTo in HoK tokens, not only supporting it for websso
      Confirmation confirmation = (confirmationCert == null) ? new Confirmation(
         inResponseTo, recipient) : new Confirmation(confirmationCert);
      AuthenticationData authnData = new AuthenticationData(tokenPresenter,
         authnTime, AuthnMethod.ASSERTION, identityAttribute);
      // the default one
      Builder builder = new Builder(lifespan, confirmation, authnData,
         attributeNames);
      if (desiredSignatureAlgorithmInRequest != null) {
         builder = builder
            .setSignatureAlgorithm(desiredSignatureAlgorithmInRequest);
      }
      return builder;
   }

   public static Builder createSpecBuilder(PrincipalId tokenPresenter,
      X509Certificate confirmationCert, Date authnTime, TimePeriod lifespan,
      String identityAttribute, Collection<String> attributeNames,
      SignatureAlgorithm desiredSignatureAlgorithmInRequest) {
      return createSpecBuilder(tokenPresenter, confirmationCert, authnTime,
         lifespan, identityAttribute, attributeNames,
         desiredSignatureAlgorithmInRequest, null, null);
   }

   public static TokenRestrictions createDefaultTokenRestrictions() {
      return createTokenRestrictions(DEFAULT_MAXIMUM_TOKEN_LIFETIME,
         DEFAULT_MAXIMUM_TOKEN_LIFETIME, TOKEN_DELEGATION_COUNT,
         TOKEN_RENEW_COUNT);
   }

   public static TokenRestrictions createTokenRestrictions(
      long maximumBearerTokenLifetime, long maximumHoKTokenLifetime) {
      return createTokenRestrictions(maximumBearerTokenLifetime,
         maximumHoKTokenLifetime, TOKEN_DELEGATION_COUNT, TOKEN_RENEW_COUNT);
   }

   public static TokenRestrictions createTokenRestrictions(
      long maximumBearerTokenLifetime, long maximumHoKTokenLifetime,
      int delegationCount, int renewCount) {
      return new TokenRestrictions(maximumBearerTokenLifetime,
         maximumHoKTokenLifetime, delegationCount, renewCount);
   }

   public static void createDelegationChain(Builder builder,
      PrincipalId tokenSubject, Date delegateAssertionEndTime) {

      setDelegationChain(builder, tokenSubject, new PrincipalId("solution2",
         SYSTEM_DOMAIN), delegateAssertionEndTime);
   }

   public static void setNoDelegateSpec(Builder builder,
      PrincipalId tokenSubject, Date delegateAssertionEndTime) {

      setDelegationChain(builder, tokenSubject, null, delegateAssertionEndTime);
   }

   private static void setDelegationChain(Builder builder,
      PrincipalId tokenSubject, PrincipalId delegateTo,
      Date delegateAssertionEndTime) {
      DelegationHistory delHistory = oneDelegateHistory(tokenSubject,
         delegateAssertionEndTime);
      builder.setDelegationSpec(new SamlTokenSpec.DelegationSpec(delegateTo,
         false, delHistory));
   }

   private static DelegationHistory oneDelegateHistory(
      PrincipalId tokenSubject, Date delegateAssertionEndTime) {
      TokenDelegate delegate1 = new TokenDelegate(new PrincipalId("solution1",
         SYSTEM_DOMAIN), new Date());
      List<TokenDelegate> chain = new ArrayList<SamlTokenSpec.TokenDelegate>();
      chain.add(delegate1);
      DelegationHistory delHistory = new DelegationHistory(tokenSubject, chain,
         5, delegateAssertionEndTime);
      return delHistory;
   }
}
