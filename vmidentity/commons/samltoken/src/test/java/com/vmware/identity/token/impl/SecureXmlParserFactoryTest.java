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

import java.io.IOException;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;

import org.junit.Test;
import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;

import com.vmware.identity.token.impl.SecureXmlParserFactory;

public class SecureXmlParserFactoryTest {

   @Test(expected = SAXParseException.class)
   public void testDoctypeDeclarationDisallowed()
      throws ParserConfigurationException, SAXException, IOException {
      SecureXmlParserFactory factory = new SecureXmlParserFactory();
      DocumentBuilder db = factory.newDocumentBuilder();
      db.parse(this.getClass().getResourceAsStream(
         "/sample_xml/xml_entities.xml"));
   }

}
