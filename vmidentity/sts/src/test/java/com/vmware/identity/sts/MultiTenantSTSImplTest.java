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
package com.vmware.identity.sts;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import org.junit.Before;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.sts.Request.Signature;
import com.vmware.identity.sts.impl.MultiTenantSTSImpl;

public class MultiTenantSTSImplTest {

   private static final String TENANT_NAME = "tenant";

   private MultiTenantSTS sts;
   private STSFactory stsFactory;

   @Before
   public void init() {
      stsFactory = createMock(STSFactory.class);
      sts = new MultiTenantSTSImpl(stsFactory);
   }

   @Test
   public void testIssue() {
      // arrange
      STS singleTenantSts = createMock(STS.class);
      Request req = createRequest();
      RequestSecurityTokenResponseCollectionType response = createMock(RequestSecurityTokenResponseCollectionType.class);

      expect(stsFactory.getSTS(eq(TENANT_NAME))).andReturn(singleTenantSts);
      expect(singleTenantSts.issue(eq(req))).andReturn(response);

      replay(stsFactory, singleTenantSts, response);

      // act
      RequestSecurityTokenResponseCollectionType issuedRSTR = sts.issue(
         TENANT_NAME, req);

      // assert
      assertEquals(response, issuedRSTR);

      verify(stsFactory, singleTenantSts, response);
   }

   @Test
   public void testIssueThrowsInvalidSignature() {
      // arrange
      STS singleTenantSts = createMock(STS.class);
      Request req = createRequest();
      RequestSecurityTokenResponseCollectionType response = createMock(RequestSecurityTokenResponseCollectionType.class);

      expect(stsFactory.getSTS(eq(TENANT_NAME))).andReturn(singleTenantSts);
      expect(singleTenantSts.issue(eq(req))).andThrow(
         new InvalidSignatureException("invalid signature"));

      replay(stsFactory, singleTenantSts, response);

      // act
      try {
         sts.issue(TENANT_NAME, req);
         fail();
      } catch (InvalidCredentialsException e) {
         // pass
      }

      // assert
      verify(stsFactory, singleTenantSts, response);
   }

   @Test
   public void testChallenge() {
      // arrange
      STS singleTenantSts = createMock(STS.class);
      RequestSecurityTokenResponseType requestSecurityTokenResponse = new RequestSecurityTokenResponseType();
      SecurityHeaderType headerInfo = new SecurityHeaderType();
      RequestSecurityTokenResponseCollectionType expectedResponse = createMock(RequestSecurityTokenResponseCollectionType.class);

      expect(stsFactory.getSTS(eq(TENANT_NAME))).andReturn(singleTenantSts);
      expect(
         singleTenantSts.challenge(requestSecurityTokenResponse, headerInfo))
         .andReturn(expectedResponse);

      replay(stsFactory, singleTenantSts, expectedResponse);

      // act
      RequestSecurityTokenResponseCollectionType challengeResponse = sts.challenge(
         TENANT_NAME, requestSecurityTokenResponse, headerInfo);

      // assert
      assertEquals(expectedResponse, challengeResponse);

      verify(stsFactory, singleTenantSts, expectedResponse);
   }

   @Test
   public void testChallengeThrowsInvalidSignature() {
      // arrange
      STS singleTenantSts = createMock(STS.class);
      RequestSecurityTokenResponseType requestSecurityTokenResponse = new RequestSecurityTokenResponseType();
      SecurityHeaderType headerInfo = new SecurityHeaderType();
      RequestSecurityTokenResponseType expectedResponse = createMock(RequestSecurityTokenResponseType.class);

      expect(stsFactory.getSTS(eq(TENANT_NAME))).andReturn(singleTenantSts);
      expect(
         singleTenantSts.challenge(requestSecurityTokenResponse, headerInfo))
         .andThrow(new InvalidSignatureException("invalid signature"));

      replay(stsFactory, singleTenantSts, expectedResponse);

      // act
      try {
         sts.challenge(TENANT_NAME, requestSecurityTokenResponse, headerInfo);
         fail();
      } catch (InvalidCredentialsException e) {
         // pass
      }

      // assert
      verify(stsFactory, singleTenantSts, expectedResponse);
   }

   @Test
   public void testRenew() {
      // arrange
      STS singleTenantSts = createMock(STS.class);
      Request req = createRequest();
      RequestSecurityTokenResponseType expectedResponse = createMock(RequestSecurityTokenResponseType.class);

      expect(stsFactory.getSTS(eq(TENANT_NAME))).andReturn(singleTenantSts);
      expect(singleTenantSts.renew(eq(req))).andReturn(expectedResponse);

      replay(stsFactory, singleTenantSts, expectedResponse);

      // act
      RequestSecurityTokenResponseType challengeResponse = sts.renew(
         TENANT_NAME, req);

      // assert
      assertEquals(expectedResponse, challengeResponse);

      verify(stsFactory, singleTenantSts, expectedResponse);
   }

   @Test
   public void testRenewThrowsInvalidSignature() {
      // arrange
      STS singleTenantSts = createMock(STS.class);
      Request req = createRequest();
      RequestSecurityTokenResponseType expectedResponse = createMock(RequestSecurityTokenResponseType.class);

      expect(stsFactory.getSTS(eq(TENANT_NAME))).andReturn(singleTenantSts);

      expect(singleTenantSts.renew(eq(req))).andThrow(
         new InvalidSignatureException("invalid signature"));

      replay(stsFactory, singleTenantSts, expectedResponse);

      // act
      try {
         sts.renew(TENANT_NAME, req);
         fail();
      } catch (InvalidCredentialsException e) {
         // pass
      }

      // assert
      verify(stsFactory, singleTenantSts, expectedResponse);
   }

   @Test
   public void testValidate() {
      // arrange
      STS singleTenantSts = createMock(STS.class);
      Request req = createRequest();
      RequestSecurityTokenResponseType expectedResponse = createMock(RequestSecurityTokenResponseType.class);

      expect(stsFactory.getSTS(eq(TENANT_NAME))).andReturn(singleTenantSts);
      expect(singleTenantSts.validate(eq(req))).andReturn(expectedResponse);

      replay(stsFactory, singleTenantSts, expectedResponse);

      // act
      RequestSecurityTokenResponseType challengeResponse = sts.validate(
         TENANT_NAME, req);

      // assert
      assertEquals(expectedResponse, challengeResponse);

      verify(stsFactory, singleTenantSts, expectedResponse);
   }

   private Request createRequest() {
      SecurityHeaderType header = new SecurityHeaderType();
      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      Signature signature = null;
      return new Request(header, rst, signature, null, null);
   }

}
