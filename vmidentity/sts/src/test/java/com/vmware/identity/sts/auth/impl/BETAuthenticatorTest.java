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

import java.util.Arrays;
import java.util.UUID;

import junit.framework.Assert;

import org.apache.commons.codec.binary.Base64;
import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.BinaryExchangeType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.idm.GSSResult;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.auth.Result.AuthnMethod;

/**
 * Insert your comment for BETAuthenticatorTest here
 */
public final class BETAuthenticatorTest {

   private static final String WST_VALUE_TYPE = BETAuthenticator.WST_VALUE_TYPE;
   private static final String WST_ENCODING_TYPE = BETAuthenticator.WST_ENCODING_TYPE;

   private static final String BASE64_ENCODED_INITIATOR_LEG = "base64_encoded_initiator_leg";

   private SecurityHeaderType header;

   @Before
   public void init() {
      header = new SecurityHeaderType();
   }

   @Test
   public void testNoBET() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new BETAuthenticator(idmAuth);

      final RequestSecurityTokenType rst = new RequestSecurityTokenType();
      Assert.assertNull(authenticator.authenticate(newReq(rst)));

      rst.setBinaryExchange(new BinaryExchangeType());
      Assert.assertNull(authenticator.authenticate(newReq(rst)));
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testNoValueType() {
      testInvalidBETInt(null, WST_ENCODING_TYPE, BASE64_ENCODED_INITIATOR_LEG);
   }

   @Test
   public void testInvalidValueType() {
      testInvalidBETInt("unknown_value_type", WST_ENCODING_TYPE,
         BASE64_ENCODED_INITIATOR_LEG);
   }

   @Test
   public void testNoEncodingType() {
      testInvalidBETInt(WST_VALUE_TYPE, null, BASE64_ENCODED_INITIATOR_LEG);
   }

   @Test
   public void testInvalidEncodingType() {
      testInvalidBETInt(WST_VALUE_TYPE, "unknown_enc_type",
         BASE64_ENCODED_INITIATOR_LEG);
   }

   @Test
   public void testAuthCompleted() {
      final RequestSecurityTokenType rst = newValidRst();
      final byte[] initRawLeg = Base64.decodeBase64(rst.getBinaryExchange()
         .getValue());

      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);

      final PrincipalId principalId = new PrincipalId("user", "acme.com");
      final GSSResult gssResult = new GSSResult(rst.getContext(), principalId);

      EasyMock.expect(idmAuth.authenticate(EasyMock.eq(rst.getContext()), EasyMock.aryEq(initRawLeg)))
         .andReturn(gssResult);
      EasyMock.replay(idmAuth);

      final Authenticator authenticator = new BETAuthenticator(idmAuth);
      final Result result = authenticator.authenticate(newReq(rst));

      Assert.assertNotNull(result);
      Assert.assertTrue(result.completed());
      Assert.assertEquals(principalId, result.getPrincipalId());
      Assert.assertEquals(AuthnMethod.KERBEROS, result.getAuthnMethod());
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testAuthNotCompleted() {
      final RequestSecurityTokenType rst = newValidRst();
      final byte[] initRawLeg = Base64.decodeBase64(rst.getBinaryExchange()
         .getValue());

      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);

      final byte[] serverLeg = new byte[] { -3, 9 };
      final GSSResult gssResult = new GSSResult(rst.getContext(), serverLeg);

      EasyMock.expect(idmAuth.authenticate(EasyMock.eq(rst.getContext()), EasyMock.aryEq(initRawLeg)))
         .andReturn(gssResult);
      EasyMock.replay(idmAuth);

      final Authenticator authenticator = new BETAuthenticator(idmAuth);
      final Result result = authenticator.authenticate(newReq(rst));

      Assert.assertNotNull(result);
      Assert.assertFalse(result.completed());
      Assert.assertTrue(Arrays.equals(serverLeg, result.getServerLeg()));
      EasyMock.verify(idmAuth);
   }

   @Test
   public void testAuthFailed() {
      final RequestSecurityTokenType rst = newValidRst();
      final byte[] initRawLeg = Base64.decodeBase64(rst.getBinaryExchange()
         .getValue());

      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.expect(idmAuth.authenticate(EasyMock.eq(rst.getContext()), EasyMock.aryEq(initRawLeg)))
         .andThrow(new com.vmware.identity.sts.idm.InvalidCredentialsException());
      EasyMock.replay(idmAuth);

      final Authenticator authenticator = new BETAuthenticator(idmAuth);

      intTestInvalidCredExc(authenticator, rst);

      EasyMock.verify(idmAuth);
   }

   private void testInvalidBETInt(String valueType, String encodingType,
      String value) {
      com.vmware.identity.sts.idm.Authenticator idmAuth = noCalledIDMAuth();
      final Authenticator authenticator = new BETAuthenticator(idmAuth);

      intTestUnsupportedSecToken(authenticator,
         newRst(valueType, encodingType, value));

      EasyMock.verify(idmAuth);
   }

   private com.vmware.identity.sts.idm.Authenticator noCalledIDMAuth() {
      com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock
         .createMock(com.vmware.identity.sts.idm.Authenticator.class);
      EasyMock.replay(idmAuth);
      return idmAuth;
   }

   private void intTestUnsupportedSecToken(Authenticator authenticator,
      RequestSecurityTokenType rst) {
      try {
         authenticator.authenticate(newReq(rst));
         Assert.fail();
      } catch (UnsupportedSecurityTokenException e) {
         // expected
      }
   }

   private void intTestInvalidCredExc(Authenticator authenticator,
      RequestSecurityTokenType rst) {
      try {
         authenticator.authenticate(newReq(rst));
         Assert.fail();
      } catch (InvalidCredentialsException e) {
         // expected
      }
   }

   private RequestSecurityTokenType newValidRst() {
      return newRst(WST_VALUE_TYPE, WST_ENCODING_TYPE,
         BASE64_ENCODED_INITIATOR_LEG);
   }

   private RequestSecurityTokenType newRst(String valueType,
      String encodingType, String value) {
      final RequestSecurityTokenType rst = new RequestSecurityTokenType();
      final BinaryExchangeType beToken = new BinaryExchangeType();
      final String contextId = UUID.randomUUID().toString();
      beToken.setValueType(valueType);
      rst.setContext(contextId);
      beToken.setEncodingType(encodingType);
      beToken.setValue(value);
      rst.setBinaryExchange(beToken);
      return rst;
   }

   private Request newReq(final RequestSecurityTokenType rst) {
      return new Request(header, rst, null, null, null);
   }

}
