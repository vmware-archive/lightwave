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

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import com.vmware.identity.token.impl.PrincipalIdParser;
import com.vmware.identity.token.impl.exception.ParserException;
import com.vmware.vim.sso.PrincipalId;

/**
 * Unit test for {@link PrincipalIdParser} class
 */
public class PrincipalIdParserTest {

   private static final String DOMAIN = "eng.vmware.com";
   private static final String NAME = "jdoe";
   private static final char SEPARATOR = '@';

   @Test(expected = IllegalArgumentException.class)
   public void testParseNullStringValue() throws ParserException {
      PrincipalIdParser.parseUpn(null);
   }

   @Test(expected = IllegalArgumentException.class)
   public void testParseEmptyStringValue() throws ParserException {
      PrincipalIdParser.parseUpn("");
   }

   @Test(expected = ParserException.class)
   public void testParseWithManySeparators() throws ParserException {
      String strValue = String.format("%s%2$c%s%2$c", NAME, SEPARATOR, DOMAIN);
      PrincipalIdParser.parseUpn(strValue);
   }

   @Test
   public void testParse() throws ParserException {
      String strValue = String.format("%s%c%s", NAME, SEPARATOR, DOMAIN);

      PrincipalId result = PrincipalIdParser.parseUpn(strValue);

      assertEquals(new PrincipalId(NAME, DOMAIN), result);
   }

   @Test(expected = ParserException.class)
   public void testParseNoDomain() throws ParserException {
      PrincipalIdParser.parseUpn(String.format("%s%c", NAME, SEPARATOR));
   }

   @Test(expected = ParserException.class)
   public void testParseNoName() throws ParserException {
      PrincipalIdParser.parseUpn(String.format("%c%s", SEPARATOR, DOMAIN));
   }

   @Test(expected = ParserException.class)
   public void testParseNoSeparator() throws ParserException {
      PrincipalIdParser.parseUpn(NAME + DOMAIN);
   }

}
