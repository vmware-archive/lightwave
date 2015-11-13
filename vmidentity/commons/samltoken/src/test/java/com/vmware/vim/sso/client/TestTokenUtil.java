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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

import com.vmware.identity.token.impl.exception.ParserException;
import com.vmware.vim.sso.client.util.KeyStoreData;
import com.vmware.vim.sso.client.util.exception.SsoKeyStoreOperationException;

/**
 * Utility class for containing common test helper methods
 */
public class TestTokenUtil {

   private static final String SAML_TOKEN_RECIPIENT_IN_SUBJECT_XML = "saml_token_recipient_in_subject.xml";
   private static final String SAML_TOKEN_VALID_XML = "saml_token_valid.xml";
   private static final String ANOTHER_SAML_TOKEN_VALID_XML = "saml_token_valid2.xml";
   public static final String TEST_KEYSTORE_FILENAME = "sso_test.jks";
   public static final String TEST_KEYSTORE_KEY = "vmware";
   public static final String TEST_KEYSTORE_CERT_ALIAS = "vmware";
   public static final String TEST_KEYSTORE_PRIV_KEY_PASSWORD = "vmware";
   public static final String SAML_TOKEN_DIR = "/saml_token/";

   /**
    * Loads a file into string
    *
    * @param fileName
    * @return String content of the file
    * @throws IOException
    */
   public static String loadStreamContent(InputStream stream)
      throws IOException {
      StringBuilder content = new StringBuilder();

      BufferedReader reader = new BufferedReader(new InputStreamReader(stream,
         "UTF-8"));
      try {
         char[] buff = new char[1024];
         int i = 0;
         while ((i = reader.read(buff)) != -1) {
            content.append(buff, 0, i);
         }
      } finally {
         reader.close();
      }

      return content.toString();
   }

   /**
    * Loads the default keystore for the test cases
    *
    * @return KeyStoreData
    * @throws SsoKeyStoreOperationException
    */
   public static KeyStoreData loadDefaultKeystore()
      throws SsoKeyStoreOperationException {

      return new KeyStoreData(TestTokenUtil.class.getResource(
         "/" + TestTokenUtil.TEST_KEYSTORE_FILENAME).getFile(),
         TestTokenUtil.TEST_KEYSTORE_KEY.toCharArray(),
         TestTokenUtil.TEST_KEYSTORE_CERT_ALIAS);
   }

   /**
    * Loads a valid token string.
    *
    * @return
    * @throws ParserException
    */
   public static String getValidSamlTokenString() throws ParserException {
      return getValidSamlTokenStringImpl(SAML_TOKEN_VALID_XML);
   }

   /**
    * Loads a valid token string with recipient in the subject.
    *
    * @return
    * @throws ParserException
    */
   public static String getValidSamlTokenString_RecipientInSubject()
      throws ParserException {
      return getValidSamlTokenStringImpl(SAML_TOKEN_RECIPIENT_IN_SUBJECT_XML);
   }

   /**
    * Loads another valid token string.
    *
    * @return
    * @throws ParserException
    */
   public static String getAnotherValidSamlTokenString() throws ParserException {
      return getValidSamlTokenStringImpl(ANOTHER_SAML_TOKEN_VALID_XML);
   }

   /**
    * Load a valid token DOM element.
    *
    * @return
    * @throws ParserException
    */
   public static Element getValidSamlTokenElement() throws ParserException {
      return parseXml(getValidSamlTokenString());
   }

   /**
    * Load another valid token DOM element.
    *
    * @return
    * @throws ParserException
    */
   public static Element getAnotherValidSamlTokenElement()
      throws ParserException {
      return parseXml(getAnotherValidSamlTokenString());
   }

   /**
    * Parses arbitrary xml
    *
    * @param token
    * @return non null DOM-tree root element
    * @throws ParserException
    */
   public static Element parseXml(String token) throws ParserException {

      XmlParserFactory parserFactory = XmlParserFactory.Factory
         .createSecureXmlParserFactory();

      try {
         DocumentBuilder docBuilder = parserFactory.newDocumentBuilder();

         InputSource src = new InputSource(new StringReader(token));
         return docBuilder.parse(src).getDocumentElement();

      } catch (ParserConfigurationException e) {
         throw new ParserException("Internal creating XML parser", e);

      } catch (SAXException e) {
         throw new ParserException("Error parsing token XML", e);

      } catch (IOException e) {
         throw new ParserException(
            "Unexpected error reading from in-memory stream", e);
      }
   }

   private static String getValidSamlTokenStringImpl(String tokenFileName)
      throws ParserException {
      try {
         return TestTokenUtil.loadStreamContent(TestTokenUtil.class
            .getResourceAsStream(TestTokenUtil.SAML_TOKEN_DIR + tokenFileName));

      } catch (IOException e) {
         throw new ParserException("SamlToken cannot be read", e);
      }
   }

}
