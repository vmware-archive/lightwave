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

import java.security.cert.X509Certificate;
import java.util.Date;

import junit.framework.Assert;

import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.UseKeyType;
import org.oasis_open.docs.ws_sx.ws_trust._200802.ActAsType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.w3._2000._09.xmldsig_.ObjectFactory;
import org.w3._2000._09.xmldsig_.SignatureType;

import com.vmware.identity.sts.CertificateUtil;
import com.vmware.identity.sts.ContradictoryHoKConditionsException;
import com.vmware.identity.sts.InvalidSecurityHeaderException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.Request.CertificateLocation;
import com.vmware.identity.sts.Request.Signature;

public final class HoKConditionsAnalyzerTest {

   private final HoKConditionsAnalyzer analyzer = new HoKConditionsAnalyzer();
   private final X509Certificate hokCertificate = new CertificateUtil(
      CertificateUtil.STS_STORE_JKS, CertificateUtil.PASSWORD)
      .loadCert(CertificateUtil.STS_CERT_ALIAS);
   private final X509Certificate solutionCert = new CertificateUtil(
      CertificateUtil.SOLUTION_STORE_JKS, CertificateUtil.PASSWORD)
      .loadCert(CertificateUtil.SOLUTION_CERT_ALIAS);
   private final String defSignatureId = "_a3276d40-56b7-42bd-a63e-e2c1eb84c081";
   private final boolean noActAs = false;

   @Test
   public void testBearer() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, noActAs,
            null, null), null, null));
   }
   @Test
   public void testBearerBearerSamlToken() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, noActAs,
            null, null), null, getBearerToken()));
   }
   @Test
   public void testBearerHokSamlToken() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, noActAs,
            null, null), null, getHokToken()));
   }

   @Test
   public void testBearerSignature() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), null, null));
   }
   @Test
   public void testBearerSignatureBearerSamlToken() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), null, getBearerToken()));
   }
   @Test
   public void testBearerSignatureHokSamlToken() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), null, getHokToken()));
   }

   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testBearerDelegate() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, noActAs,
            hokCertificate, null), solutionCert, null);
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testBearerDelegateBearerSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, noActAs,
            hokCertificate, null), solutionCert, getBearerToken());
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testBearerDelegateHokSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, noActAs,
            hokCertificate, null), solutionCert, getHokToken());
   }

   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testBearerActAs() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, true,
            hokCertificate, null), solutionCert, null);
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testBearerActAsBearerSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, true,
            hokCertificate, null), solutionCert, getBearerToken());
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testBearerActAsHokSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, null, true,
            hokCertificate, null), solutionCert, getHokToken());
   }

   @Test(expected = InvalidSecurityHeaderException.class)
   public void testUseKeyNoCertificate() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, defSignatureId,
            noActAs, null, null), null, null);
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testUseKeyNoCertificateBearerSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, defSignatureId,
            noActAs, null, null), null, getBearerToken());
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testUseKeyNoCertificateHokSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.BEARER_KEY_TYPE, defSignatureId,
            noActAs, null, null), null, getHokToken());
   }

   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testUseKeyDelegate() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), solutionCert, null);
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testUseKeyDelegateBearerSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), solutionCert, getBearerToken());
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testUseKeyDelegateHokSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), solutionCert, getHokToken() );
   }

   @Test
   public void testNoKeyTypeWithSignature() {
       Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
           createRequest(null, defSignatureId, noActAs, hokCertificate,
              null), null, null));
   }
   @Test
   public void testNoKeyTypeWithSignatureBearerSamlToken() {
       Assert.assertNull(analyzer.getSigningCertificate(
          createRequest(null, defSignatureId, noActAs, hokCertificate,
          null), null, getBearerToken()));
   }
   @Test
   public void testNoKeyTypeWithSignatureHokSamlToken() {
       Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
           createRequest(null, defSignatureId, noActAs, hokCertificate,
              null), null, getHokToken()));
   }

   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignature() {
       Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
           createRequest(
               null, defSignatureId, noActAs, hokCertificate,
               new SecurityHeaderType()), null, null));
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureBearerSamlToken() {
       Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
               createRequest(
                   null, defSignatureId, noActAs, hokCertificate,
                   new SecurityHeaderType()), null, null));
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureHokSamlToken() {
       Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
           createRequest(
               null, defSignatureId, noActAs, hokCertificate,
               new SecurityHeaderType()), null, getHokToken()));
   }

   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureId() {
      SecurityHeaderType securityHeaderType = new SecurityHeaderType();
      SignatureType xmlSignature = new SignatureType();
      ObjectFactory objectFactory = new ObjectFactory();
      securityHeaderType.getAny().add(objectFactory.createSignature(xmlSignature));
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
              createRequest(
                  null, defSignatureId, noActAs, hokCertificate,
                  securityHeaderType), null, null));
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureIdBearerSamlToken() {
      SecurityHeaderType securityHeaderType = new SecurityHeaderType();
      SignatureType xmlSignature = new SignatureType();
      ObjectFactory objectFactory = new ObjectFactory();
      securityHeaderType.getAny().add(objectFactory.createSignature(xmlSignature));
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
              createRequest(
                  null, defSignatureId, noActAs, hokCertificate,
                  securityHeaderType), null, null));
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureIdHokSamlToken() {
      SecurityHeaderType securityHeaderType = new SecurityHeaderType();
      SignatureType xmlSignature = new SignatureType();
      ObjectFactory objectFactory = new ObjectFactory();
      securityHeaderType.getAny().add(objectFactory.createSignature(xmlSignature));
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
              createRequest(
                  null, defSignatureId, noActAs, hokCertificate,
                  securityHeaderType), null, getHokToken()));
   }

   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureIdMatchingInRST() {
      SecurityHeaderType securityHeaderType = new SecurityHeaderType();
      SignatureType xmlSignature = new SignatureType();
      xmlSignature.setId("notMatchingId");
      ObjectFactory objectFactory = new ObjectFactory();
      securityHeaderType.getAny().add(objectFactory.createSignature(xmlSignature));
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
          createRequest(
              null, defSignatureId, noActAs, hokCertificate,
              securityHeaderType), null, null));
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureIdMatchingInRSTBearerSamlToken() {
      SecurityHeaderType securityHeaderType = new SecurityHeaderType();
      SignatureType xmlSignature = new SignatureType();
      xmlSignature.setId("notMatchingId");
      ObjectFactory objectFactory = new ObjectFactory();
      securityHeaderType.getAny().add(objectFactory.createSignature(xmlSignature));
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
              createRequest(
                  null, defSignatureId, noActAs, hokCertificate,
                  securityHeaderType), null, null));
   }
   @Test(expected = InvalidSecurityHeaderException.class)
   public void testNoKeyTypeNoSignatureIdMatchingInRSTHokSamlToken() {
      SecurityHeaderType securityHeaderType = new SecurityHeaderType();
      SignatureType xmlSignature = new SignatureType();
      xmlSignature.setId("notMatchingId");
      ObjectFactory objectFactory = new ObjectFactory();
      securityHeaderType.getAny().add(objectFactory.createSignature(xmlSignature));
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
          createRequest(
              null, defSignatureId, noActAs, hokCertificate,
              securityHeaderType), null, getHokToken()));
   }

   @Test
   public void testNoKeyTypeNoUseKeyNoDelegate() {
      Assert.assertEquals(
         hokCertificate,
         analyzer.getSigningCertificate(
            createRequest(null, null, noActAs, hokCertificate, null), null, null));
   }
   @Test
   public void testNoKeyTypeNoUseKeyNoDelegateBearerSamlToken() {
      Assert.assertNull(
         analyzer.getSigningCertificate(
            createRequest(null, null, noActAs, hokCertificate, null), null, getBearerToken()));
   }
   @Test
   public void testNoKeyTypeNoUseKeyNoDelegateHokSamlToken() {
      Assert.assertEquals(
         hokCertificate,
         analyzer.getSigningCertificate(
            createRequest(null, null, noActAs, hokCertificate, null), null, getHokToken()));
   }

   @Test
   public void testNoKeyTypeNoUseKeyNoDelegateNoCert() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(null, null, noActAs, null, null), null, null));
   }
   @Test
   public void testNoKeyTypeNoUseKeyNoDelegateNoCertBearerSamlToken() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(null, null, noActAs, null, null), null, getBearerToken()));
   }
   @Test
   public void testNoKeyTypeNoUseKeyNoDelegateNoCertHokSamlToken() {
      Assert.assertNull(analyzer.getSigningCertificate(
         createRequest(null, null, noActAs, null, null), null, getHokToken()));
   }

   @Test
   public void testNoKeyTypeNoUseKeyDelegateNoCert() {
      Assert.assertEquals(
         solutionCert,
         analyzer.getSigningCertificate(
            createRequest(null, null, noActAs, null, null), solutionCert, null));
   }
   @Test(expected=ContradictoryHoKConditionsException.class)
   public void testNoKeyTypeNoUseKeyDelegateNoCertBearerSamlToken() {
      Assert.assertNull(
         analyzer.getSigningCertificate(
            createRequest(null, null, noActAs, null, null), solutionCert, getBearerToken()));
   }
   @Test
   public void testNoKeyTypeNoUseKeyDelegateNoCertHokSamlToken() {
      Assert.assertEquals(
         solutionCert,
         analyzer.getSigningCertificate(
            createRequest(null, null, noActAs, null, null), solutionCert, getHokToken()));
   }

   @Test
   public void testNoKeyTypeNoUseKeyDelegateWithCert() {
      Assert.assertEquals(solutionCert, analyzer
         .getSigningCertificate(
            createRequest(null, null, noActAs, hokCertificate, null),
            solutionCert, null));
   }
   @Test(expected=ContradictoryHoKConditionsException.class)
   public void testNoKeyTypeNoUseKeyDelegateWithCertBearerSamlToken() {
      Assert.assertNull(analyzer
         .getSigningCertificate(
            createRequest(null, null, noActAs, hokCertificate, null),
            solutionCert, getBearerToken()));
   }
   @Test
   public void testNoKeyTypeNoUseKeyDelegateWithCertHokSamlToken() {
      Assert.assertEquals(solutionCert, analyzer
         .getSigningCertificate(
            createRequest(null, null, noActAs, hokCertificate, null),
            solutionCert, getHokToken()));
   }

   @Test
   public void testHoKWithSignature() {
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), null, null));
   }
   @Test(expected=ContradictoryHoKConditionsException.class)
   public void testHoKWithSignatureBearerSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), null, getBearerToken());
   }
   @Test
   public void testHoKWithSignatureHokSamlToken() {
      Assert.assertEquals(hokCertificate, analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, defSignatureId,
            noActAs, hokCertificate, null), null, getHokToken()));
   }

   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testHoKNoUseKeyNoDelegateNoCert() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null, noActAs, null,
            null), null, null);
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testHoKNoUseKeyNoDelegateNoCertBearerSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null, noActAs, null,
            null), null, getBearerToken());
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testHoKNoUseKeyNoDelegateNoCertHokSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null, noActAs, null,
            null), null, getHokToken());
   }

   @Test
   public void testHoKNoUseKeyDelegateNoCert() {
      Assert.assertEquals(solutionCert, analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null, noActAs, null,
            null), solutionCert, null));
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testHoKNoUseKeyDelegateNoCertBearerSamlToken() {
      analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null, noActAs, null,
            null), null, getBearerToken());
   }
   @Test
   public void testHoKNoUseKeyDelegateNoCertHokSamlToken() {
      Assert.assertEquals(solutionCert, analyzer.getSigningCertificate(
         createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null, noActAs, null,
            null), solutionCert, getHokToken()));
   }

   @Test
   public void testHoKNoUseKeyDelegateWithCert() {
      Request req = createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null,
         noActAs, hokCertificate, null);
      Assert.assertEquals(solutionCert,
         analyzer.getSigningCertificate(req, solutionCert, null));
   }
   @Test(expected = ContradictoryHoKConditionsException.class)
   public void testHoKNoUseKeyDelegateWithCertBearerSamlToken() {
      Request req = createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null,
         noActAs, hokCertificate, null);
      analyzer.getSigningCertificate(req, solutionCert, getBearerToken());
   }
   @Test
   public void testHoKNoUseKeyDelegateWithCertHokSamlToken() {
      Request req = createRequest(HoKConditionsAnalyzer.HOK_KEY_TYPE, null,
         noActAs, hokCertificate, null);
      Assert.assertEquals(solutionCert,
         analyzer.getSigningCertificate(req, solutionCert, getHokToken()));
   }

   private Request createRequest(String keyType, String signatureId,
      boolean actAs, X509Certificate requestSigningCertificate,
      SecurityHeaderType securityHeader) {
      Signature signature = (requestSigningCertificate == null) ? null
         : new Signature(requestSigningCertificate, CertificateLocation.BST);
      return new Request(securityHeader != null ? securityHeader
         : getSecurityHeader(signatureId), createRST(keyType, signatureId, actAs),
         signature, null, actAs ? new SamlTokenMock("aa", "bb", solutionCert, new Date()) : null);
   }

   private RequestSecurityTokenType createRST(String keyType, String signatureId, boolean actAs) {
      RequestSecurityTokenType rst = new RequestSecurityTokenType();
      rst.setKeyType(keyType);
      if (signatureId != null) {
         UseKeyType useKey = new UseKeyType();
         useKey.setSig(signatureId);
         rst.setUseKey(useKey);
      }
      if (actAs){
         rst.setActAs(new ActAsType());
      }

      return rst;
   }

   private SecurityHeaderType getSecurityHeader(String signatureId) {
      SecurityHeaderType header = new SecurityHeaderType();
      SignatureType xmlSignature = new SignatureType();
      xmlSignature.setId(signatureId);
      ObjectFactory objectFactory = new ObjectFactory();
      header.getAny().add(objectFactory.createSignature(xmlSignature));

      return header;
   }

   private SamlTokenMock getBearerToken()
   {
       return new SamlTokenMock("aa", "bb", null, new Date());
   }
   private SamlTokenMock getHokToken()
   {
       return new SamlTokenMock("aa", "bb", hokCertificate, new Date());
   }
}