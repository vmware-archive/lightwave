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

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;

import com.vmware.identity.token.impl.SecureXmlParserFactory;

/**
 * Simplified version of {@link javax.xml.parsers.DocumentBuilderFactory}
 * designed for use for SAML token parsing.
 */
public interface XmlParserFactory {

   /**
    * Creates a document builder with configuration acceptable only for securely
    * parsing SAML tokens and related XML contexts. Do not use as a general XML
    * parser.
    *
    * @return a new document builder suitable for secure SAML token parsing, not null
    * @throws ParserConfigurationException
    */
   public DocumentBuilder newDocumentBuilder() throws ParserConfigurationException;

   /**
    * Factory for different flavor XML parsers.
    */
   public static class Factory {
      /**
       * Creates a XML parser factory that:
       * <ul>
       *  <li>disables validation
       *  <li>doesn't load external DTDs
       *  <li>resolves all external XML entities to empty string
       *  <li>supports standard XML entities and local "string-substition" XML entities
       *  <li>is namespace aware
       *  <li>is thread-safe
       * </ul>
       * @return a secure XML parser factory.
       */
      public static XmlParserFactory createSecureXmlParserFactory() {
         return new SecureXmlParserFactory();
      }
   }
}