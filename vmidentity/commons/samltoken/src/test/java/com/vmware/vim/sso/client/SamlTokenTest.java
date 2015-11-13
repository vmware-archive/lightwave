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
package com.vmware.vim.sso.client;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.security.cert.X509Certificate;
import java.util.List;

import javax.xml.parsers.ParserConfigurationException;

import org.junit.Assert;
import org.junit.Test;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import com.vmware.identity.token.impl.exception.ParserException;
import com.vmware.vim.sso.PrincipalId;
import com.vmware.vim.sso.client.Advice.AdviceAttribute;
import com.vmware.vim.sso.client.exception.InvalidSignatureException;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;

/**
 * Test various SAML token creation scenarios
 */
public class SamlTokenTest {

   private static final String EMAIL_ADDRESS_FORMAT = "http://schemas.xmlsoap.org/ws/2005/05/identity/claims/emailaddress";
   private static final String SUBJECT_NAME_ID_OF_TOKEN_WITH_RECIPIENT_AND_IN_RESPONSE_TO = "AdminEmail@example.com";
   private static final String UPN_FORMAT = "http://schemas.xmlsoap.org/claims/UPN";
   private static final String ENTITY_FORMAT = "urn:oasis:names:tc:SAML:2.0:nameid-format:entity";
   private static final String ISSUER = "https://GStaykov-Dev.vmware.com:8444/STS";
   private static final String UPN_SUBJECT = "HoKUser@example.com";
   private static final String SAML_TOKEN_DIR = "/saml_token/";
   private static final int INVALID_TOKEN_COUNT = 13;

   @Test
   public void createValidToken() throws IOException, ParserException,
      SsoKeyStoreOperationException, InvalidTokenException {
      String samlXml = TestTokenUtil.getValidSamlTokenString();

      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
      SamlToken token = DefaultTokenFactory.createToken(samlXml,
         keystore.getCertificate());

      assertBasicFieldValid(token);
   }

   @Test
   public void createValidTokenWithGroups() throws IOException,
      SsoKeyStoreOperationException, InvalidTokenException {
      String samlXml = TestTokenUtil.loadStreamContent(this.getClass()
         .getResourceAsStream(SAML_TOKEN_DIR + "saml_token_valid_groups.xml"));

      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
      SamlToken token = DefaultTokenFactory.createToken(samlXml,
         keystore.getCertificate());

      assertBasicFieldValid(token);
      assertEquals(3, token.getGroupList().size());
   }

   @Test
   public void createTokenWithInvalidSignature() throws IOException,
      SsoKeyStoreOperationException, InvalidTokenException {
      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();

      String validTokenXml = TestTokenUtil.loadStreamContent(this.getClass()
         .getResourceAsStream(SAML_TOKEN_DIR + "saml_token_valid.xml"));

      String invalidSignatureSamlXml = putWrongCharInSignature(validTokenXml);

      try {
         DefaultTokenFactory.createToken(invalidSignatureSamlXml,
            keystore.getCertificate());
         Assert.fail();
      } catch (InvalidSignatureException e) {
         // expected
      }
   }

   @Test
   public void createValidTokenWhitespaceInConfirmation() throws IOException,
      ParserException, SsoKeyStoreOperationException, InvalidTokenException {
      String samlXml = TestTokenUtil.loadStreamContent(this.getClass()
         .getResourceAsStream(
            SAML_TOKEN_DIR + "saml_token_valid_whitespace.xml"));

      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
      SamlToken token = DefaultTokenFactory.createToken(samlXml,
         keystore.getCertificate());

      assertBasicFieldValid(token);
   }

   @Test
   public void createValidTokenFromDom() throws IOException, SAXException,
      ParserConfigurationException, ParserException, InvalidTokenException,
      SsoKeyStoreOperationException {

      final KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();

      Element tokenRoot = TestTokenUtil.getValidSamlTokenElement();
      SamlToken token = DefaultTokenFactory.createTokenFromDom(tokenRoot,
         keystore.getCertificate());
      assertNotNull(token);

      Element tokenRootDuplicate = TestTokenUtil.parseXml(token.toXml());
      SamlToken tokenDuplicate = DefaultTokenFactory.createTokenFromDom(tokenRootDuplicate,
         keystore.getCertificate());
      assertNotNull(tokenDuplicate);

      assertEquals(tokenDuplicate, token);
   }

   @Test
   public void createTokenFromParsedTokenXml() throws ParserException,
      SsoKeyStoreOperationException, InvalidTokenException {

      Element tokenElement = TestTokenUtil.getValidSamlTokenElement();
      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
      SamlToken token = DefaultTokenFactory.createTokenFromDom(tokenElement,
         keystore.getCertificate());

      assertNotNull(token);

      SamlToken dupToken = DefaultTokenFactory.createToken(token.toXml(),
         keystore.getCertificate());

      assertNotNull(dupToken);
      assertEquals(token.getSubject(), dupToken.getSubject());
   }

   @Test
   public void createInvalidToken() throws IOException,
      SsoKeyStoreOperationException, InvalidTokenException {
      KeyStoreData keystore = new KeyStoreData(this.getClass()
         .getResource("/" + TestTokenUtil.TEST_KEYSTORE_FILENAME).getFile(),
         TestTokenUtil.TEST_KEYSTORE_KEY.toCharArray(),
         TestTokenUtil.TEST_KEYSTORE_CERT_ALIAS);

      for (int currentToken = 1; currentToken < INVALID_TOKEN_COUNT + 1; currentToken++) {
         String samlXml = TestTokenUtil.loadStreamContent(this.getClass()
            .getResourceAsStream(
               SAML_TOKEN_DIR + "saml_token_invalid_" + currentToken + ".xml"));

         try {
            DefaultTokenFactory.createToken(samlXml, keystore.getCertificate());
            Assert.fail();
         } catch (InvalidTokenException e) {
            // expected
         }
      }
   }

   @Test(expected = IllegalArgumentException.class)
   public void createTokenWithInvalidString()
      throws SsoKeyStoreOperationException, InvalidTokenException {
      KeyStoreData keystore = new KeyStoreData(this.getClass()
         .getResource("/" + TestTokenUtil.TEST_KEYSTORE_FILENAME).getFile(),
         TestTokenUtil.TEST_KEYSTORE_KEY.toCharArray(),
         TestTokenUtil.TEST_KEYSTORE_CERT_ALIAS);

      DefaultTokenFactory.createToken(null, keystore.getCertificate());
   }

   @Test(expected = IllegalArgumentException.class)
   public void createTokenWithNoTrustedCertificates()
      throws InvalidTokenException, ParserException {
      DefaultTokenFactory.createToken(TestTokenUtil.getValidSamlTokenString(),
         (X509Certificate[]) null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void createTokenWithNullTrustedCertificate()
      throws InvalidTokenException, ParserException {
      DefaultTokenFactory.createToken(TestTokenUtil.getValidSamlTokenString(),
         (X509Certificate) null);
   }

   @Test
   public void serializeToXml() throws IOException, ParserException,
      SsoKeyStoreOperationException, InvalidTokenException {
      String samlXml = TestTokenUtil.getValidSamlTokenString();
      SamlToken token = getToken(samlXml);

      String serializedToken = token.toXml();
      assertFalse(serializedToken.contains("<?xml"));
      assertTrue(serializedToken.contains(token.getSubject().getName()));
   }

   // the next two test-cases include validation of inResponseTo and Recipient
   // attributes in the subjectConfirmationData
   @Test
   public void checkSubjectInEmailFormat() throws IOException, ParserException,
      SsoKeyStoreOperationException, InvalidTokenException {
      String tokenWithRecipient_ResponseTo_SubjectInEmailFormat = TestTokenUtil
         .getValidSamlTokenString_RecipientInSubject();
      // tokenWithRecipient_ResponseTo_SubjectInEmailFormat - this string equals
      // to the one in the file
      SamlToken token = getToken(tokenWithRecipient_ResponseTo_SubjectInEmailFormat);
      assertEquals(SUBJECT_NAME_ID_OF_TOKEN_WITH_RECIPIENT_AND_IN_RESPONSE_TO,
         token.getSubjectNameId().getValue());
      assertEquals(EMAIL_ADDRESS_FORMAT, token.getSubjectNameId().getFormat());
      assertNull(token.getSubject());
   }

   @Test
   public void checkSubjectInUPNFormat() throws IOException, ParserException,
      SsoKeyStoreOperationException, InvalidTokenException {
      String samlXml = TestTokenUtil.getValidSamlTokenString();
      SamlToken token = getToken(samlXml);

      assertEquals(UPN_SUBJECT, token.getSubjectNameId().getValue());
      assertEquals(UPN_FORMAT, token.getSubjectNameId().getFormat());
      assertNotNull(token.getSubject());
      PrincipalId subject = token.getSubject();
      assertEquals(UPN_SUBJECT, subject.getName() + "@" + subject.getDomain());
   }

   @Test
   public void checkIssuer() throws IOException, ParserException,
      SsoKeyStoreOperationException, InvalidTokenException {
      String samlXml = TestTokenUtil.getValidSamlTokenString();
      SamlToken token = getToken(samlXml);

      assertTrue( "Token should implement ValidatableSamlTokenEx", (token instanceof ValidatableSamlTokenEx));
      ValidatableSamlTokenEx tokenEx = (ValidatableSamlTokenEx)token;
      assertNotNull(tokenEx.getIssuerNameId());
      assertEquals(ENTITY_FORMAT, tokenEx.getIssuerNameId().getFormat());
      assertEquals(ISSUER, tokenEx.getIssuerNameId().getValue());
   }

   @Test
   public void testAdviceFriendlyName() throws IOException,
      ParserException, SsoKeyStoreOperationException, InvalidTokenException {
      String samlXml = TestTokenUtil.loadStreamContent(this.getClass()
         .getResourceAsStream(
            SAML_TOKEN_DIR + "saml_token_valid_adviceattr.xml"));

      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
      SamlToken token = DefaultTokenFactory.createToken(samlXml,
         keystore.getCertificate());

      assertBasicFieldValid(token);

      List<Advice> adviceList = token.getAdvice();
      assertNotNull("advice should be present", adviceList);
      assertEquals("advice should be present", 1, adviceList.size());
      Advice advice = adviceList.get(0);
      List<AdviceAttribute> attrs = advice.getAttributes();
      assertNotNull("advice attribute should be present", attrs);
      assertEquals("advice attribute should be present", 1, attrs.size());
      AdviceAttribute adviceAttribute = attrs.get(0);
      assertNotNull("advice attribute should be present", adviceAttribute);

      final String friendlyName = "my advice attribute";
      final String name = "urn:vc:admin.users";

      assertEquals( name, adviceAttribute.getName() );
      assertEquals( friendlyName, adviceAttribute.getFriendlyName() );
   }

   private SamlToken getToken(String samlXml) throws ParserException,
      SsoKeyStoreOperationException, InvalidTokenException {
      KeyStoreData keystore = TestTokenUtil.loadDefaultKeystore();
      SamlToken token = DefaultTokenFactory.createToken(samlXml,
         keystore.getCertificate());
      return token;
   }

   private void assertBasicFieldValid(SamlToken token) {
      assertNotNull(token);
      assertTrue("Confirmation type",
         token.getConfirmationType().equals(ConfirmationType.HOLDER_OF_KEY));
      assertTrue(!token.isDelegable());
      assertNotNull(token.getGroupList());
      assertTrue(!token.isRenewable());
      assertNotNull(token.getSubject());
      assertNotNull(token.getExpirationTime());
      assertNotNull(token.getStartTime());
   }

   private String putWrongCharInSignature(String samlXml) {
      assert samlXml != null;

      int charIndex = samlXml.indexOf("Q==</ds:SignatureValue>") - 1;

      String startXmlPart = samlXml.substring(0, charIndex);
      String endXmlPart = samlXml.substring(charIndex + 1);

      assert samlXml.charAt(charIndex) == '5';
      return startXmlPart + "6" + endXmlPart;
   }
}
