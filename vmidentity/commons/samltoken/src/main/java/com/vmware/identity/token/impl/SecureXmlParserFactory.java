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

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.vim.sso.client.XmlParserFactory;

/**
 * XML Parser factory that:
 * <ul>
 * <li>disables validation
 * <li>doesn't load external DTDs
 * <li>resolves all external XML entities to empty string
 * <li>supports standard XML entities and local "string-substition" XML entities
 * </ul>
 */
public class SecureXmlParserFactory implements XmlParserFactory {

   private final Logger log = LoggerFactory
      .getLogger(SecureXmlParserFactory.class);

   @Override
   public DocumentBuilder newDocumentBuilder()
      throws ParserConfigurationException {
      DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
      dbf.setNamespaceAware(true);

      /*
       * IMPORTANT: We disable validation because it might bring synthetic
       * ("non-specified", i.e. generated from the Schema's default values)
       * attributes into the DOM tree; they'll become "specified" after copying
       * and will most probably break the signature.
       */
      dbf.setValidating(false);

      /*
       * Optional features, recommended by security team. The feature handling
       * depends on what XML parser is on the classpath. Successfully setting
       * the features is not as essential, as dbf.validating = false and the
       * custom entity resolver.
       */
      trySetFeature(dbf, "http://xml.org/sax/features/validation", false);
      trySetFeature(dbf, XMLConstants.FEATURE_SECURE_PROCESSING, true);

      // configuration for Xerces XML parser; has not effect if using JDK parser
      trySetFeature(dbf,
         "http://apache.org/xml/features/nonvalidating/load-dtd-grammar", false);
      trySetFeature(dbf,
         "http://apache.org/xml/features/nonvalidating/load-external-dtd",
         false);
      trySetFeature(dbf, "http://apache.org/xml/features/disallow-doctype-decl", true);

      return dbf.newDocumentBuilder();
   }

   private void trySetFeature(DocumentBuilderFactory dbf, String featureKey,
      boolean value) {
      try {
         dbf.setFeature(featureKey, value);
      } catch (ParserConfigurationException e) {
         // Note that this may happen on every token parse.
         if (log.isDebugEnabled()) {
            log.debug("Couldn't apply feature " + featureKey
               + " to DocumentBuilderFactory " + dbf.getClass().getName()
               + " Can be safely ignored.");
         }
      }
   }
}
