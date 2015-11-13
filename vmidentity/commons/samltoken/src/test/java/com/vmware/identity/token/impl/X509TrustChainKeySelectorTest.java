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
package com.vmware.identity.token.impl;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.security.GeneralSecurityException;
import java.security.Key;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.xml.crypto.KeySelector;
import javax.xml.crypto.KeySelectorException;
import javax.xml.crypto.dom.DOMStructure;
import javax.xml.crypto.dsig.XMLSignature;
import javax.xml.crypto.dsig.XMLSignatureException;
import javax.xml.crypto.dsig.XMLSignatureFactory;
import javax.xml.crypto.dsig.XMLValidateContext;
import javax.xml.crypto.dsig.dom.DOMValidateContext;
import javax.xml.crypto.dsig.keyinfo.KeyInfo;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.junit.BeforeClass;
import org.junit.Test;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import com.vmware.identity.token.impl.X509TrustChainKeySelector;

public final class X509TrustChainKeySelectorTest {

   private static final String TEST_FIXTURE_PREFIX = "cert_chain_selector/";

   private static X509Certificate CERT_ROOT_CA;
   private static X509Certificate CERT_CA1;
   private static X509Certificate CERT_CA2;
   private static X509Certificate CERT_STS;

   @BeforeClass
   public static void setupSuite() throws GeneralSecurityException {
      ClassLoader loader = ClassLoader.getSystemClassLoader();

      CertificateFactory cf = CertificateFactory.getInstance("X.509");

      CERT_ROOT_CA = (X509Certificate) cf.generateCertificate(loader
         .getResourceAsStream(TEST_FIXTURE_PREFIX + "root.cert"));

      CERT_CA1 = (X509Certificate) cf.generateCertificate(loader
         .getResourceAsStream(TEST_FIXTURE_PREFIX + "ca1.cert"));

      CERT_CA2 = (X509Certificate) cf.generateCertificate(loader
         .getResourceAsStream(TEST_FIXTURE_PREFIX + "ca1.cert"));

      CERT_STS = (X509Certificate) cf.generateCertificate(loader
         .getResourceAsStream(TEST_FIXTURE_PREFIX + "sts.cert"));
   }


   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
   /*                         Construction Tests                              */
   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   @Test(expected = IllegalArgumentException.class)
   public void testCreateNoTrustedRoots() {
      new X509TrustChainKeySelector();
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateNullArray() {
      new X509TrustChainKeySelector((X509Certificate[]) null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testCreateNullCertificate() {
      new X509TrustChainKeySelector(CERT_ROOT_CA, null, CERT_STS);
   }

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
   /*                      KeyInfo Parsing Tests                              */
   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   @Test
   public void testInvalidPurpose() throws KeySelectorException {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA);
      assertNull(ks.select(null, KeySelector.Purpose.SIGN, null, null)
         .getKey());
   }

   @Test
   public void testParseNullKeyInfo() throws KeySelectorException {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA, CERT_CA1);

      assertNull(ks.select(null, KeySelector.Purpose.VERIFY, null, null)
         .getKey());
   }

   @Test
   public void testParseNoX509Data() throws Exception {
      assertNull(selectKeyFromKeyInfo("keyinfo-no-x509data"));
   }

   @Test
   public void testParseEmptyX509Data() throws Exception {
      assertNull(selectKeyFromKeyInfo("keyinfo-empty-x509data"));
   }

   @Test
   public void testParseMultiX509Data() throws Exception {
      assertNull(selectKeyFromKeyInfo("keyinfo-multi-x509data"));
   }

   @Test
   public void testParseMultiNoX509Certificates() throws Exception {
      assertNull(selectKeyFromKeyInfo("keyinfo-no-x509cert"));
   }

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
   /*                         Validation Tests                                */
   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   @Test
   public void testValidationTrustRootCA() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA);
      assertTrue(loadAndValidateSignature("ds-valid-full-ordered-chain",
         ks));
   }

   @Test
   public void testValidationTrustIntermediateCA() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_CA2);
      assertTrue(loadAndValidateSignature("ds-valid-full-ordered-chain",
         ks));
   }

   @Test
   public void testValidationTrustSigningKey() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_STS);
      assertTrue(loadAndValidateSignature("ds-valid-full-ordered-chain",
         ks));
   }

   @Test
   public void testValidationUnorderedChain() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA);
      assertTrue(loadAndValidateSignature("ds-valid-unordered-chain",
         ks));
   }

   @Test
   public void testValidationChainWithoutRoot() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA);
      assertTrue(loadAndValidateSignature("ds-valid-chain-noroot",
         ks));
   }

   @Test(expected = XMLSignatureException.class)
   public void testValidationIncompleteChain() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA);
      assertTrue(loadAndValidateSignature("ds-valid-incomplete-chain",
         ks));
   }

   @Test
   public void testValidationNoChainAndManualKey() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_STS);
      assertTrue(loadAndValidateSignature("ds-valid-nochain", ks));
   }

   @Test(expected = XMLSignatureException.class)
   public void testValidationNoChainAndMissingKey() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA, CERT_CA1);
      loadAndValidateSignature("ds-valid-nochain", ks);
   }

   @Test
   public void testValidationWithInvalidKey() throws Exception {
      KeySelector ks = new X509TrustChainKeySelector(CERT_ROOT_CA);
      try {
         assertFalse(loadAndValidateSignature("ds-valid-nochain", ks));
      } catch (XMLSignatureException e) {
         // thrown by Java7 XMLSignature.validate, Java6 returns false
      }
   }

   /**
    * Helper: Load the XML document from the resource with the specified name
    * and return whether or not it's signature is valid. The validation is
    * performed with the specified KeySelector. The resources are resolved under
    * TEST_FIXTURE_PREFIX.
    */
   private boolean loadAndValidateSignature(String name, KeySelector keySelector)
      throws Exception {

      DocumentBuilderFactory docBuilderFactory = DocumentBuilderFactory
         .newInstance();
      docBuilderFactory.setNamespaceAware(true);

      DocumentBuilder builder = docBuilderFactory.newDocumentBuilder();
      Document doc = builder.parse(getClass().getResourceAsStream(
         "/" + TEST_FIXTURE_PREFIX + name + ".xml"));


      Element data = (Element) doc.getElementsByTagName("data").item(0);
      if (data.hasAttribute("id")) {
         data.setIdAttribute("id", true);
      }
      Element signatureElement = (Element) doc.getElementsByTagNameNS(
         XMLSignature.XMLNS, "Signature").item(0);

      XMLSignatureFactory dsFactory = XMLSignatureFactory.getInstance("DOM");
      XMLValidateContext ctx = new DOMValidateContext(keySelector,
         signatureElement);

      return dsFactory.unmarshalXMLSignature(ctx).validate(ctx);
   }

   /**
    * Helper: parse a KeyInfo element from the XML resource with the specified
    * name, apply the X509TrustChainKeySelector on it and return the selected
    * key. The resource is resolved under the TEST_FIXTURE_PREFIX.
    */
   private Key selectKeyFromKeyInfo(String name)
      throws Exception {

      DocumentBuilderFactory docBuilderFactory = DocumentBuilderFactory
         .newInstance();
      docBuilderFactory.setNamespaceAware(true);

      DocumentBuilder builder = docBuilderFactory.newDocumentBuilder();
      Document doc = builder.parse(getClass().getResourceAsStream(
         "/" + TEST_FIXTURE_PREFIX + name + ".xml"));

      XMLSignatureFactory dsFactory = XMLSignatureFactory.getInstance("DOM");
      KeyInfo keyInfo = dsFactory.getKeyInfoFactory().unmarshalKeyInfo(
         new DOMStructure(doc.getDocumentElement()));

      return new X509TrustChainKeySelector(CERT_ROOT_CA, CERT_CA1).select(
         keyInfo, KeySelector.Purpose.VERIFY, null, null).getKey();
   }
}
