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

import java.util.regex.Pattern;

import com.vmware.identity.token.impl.exception.ParserException;
import com.vmware.vim.sso.PrincipalId;

/**
 * Parse principal id from string
 */
public final class PrincipalIdParser {

   /**
    * TODO : TBD
    *
    * @param upn
    * @return
    * @throws ParserException
    */
   public static PrincipalId parseUpn(String upn) throws ParserException {
      ValidateUtil.validateNotEmpty(upn, "upn");
      String[] parts = splitInTwo(upn, '@');
      return new PrincipalId(parts[0], parts[1]);
   }

   /**
    * TODO : TBD
    *
    * @param groupId
    * @return
    * @throws ParserException
    */
   public static PrincipalId parseGroupId(String groupId) throws ParserException {
      ValidateUtil.validateNotEmpty(groupId, "groupId");

      final PrincipalId group;
      if (groupId.contains("\\")) {
         String[] parts = splitInTwo(groupId, '\\');
         group = new PrincipalId(parts[1], parts[0]);

      } else {
         String[] parts = splitInTwo(groupId, '/');
         group = new PrincipalId(parts[0], parts[1]);
      }

      return group;
   }

   /**
    * TODO : TBD
    *
    * @param value
    * @param separator
    * @return
    * @throws ParserException
    */
   private static String[] splitInTwo(String value, char separator)
      throws ParserException {

      Pattern splitter = Pattern.compile(Pattern.quote(String.valueOf(separator)));
      String split[] = splitter.split(value, 3);
      if (split.length != 2 || split[0].isEmpty() || split[1].isEmpty()) {
         throw new ParserException(String.format(
               "Invalid principal value: `%s' (incorrect number of fields)",
               value));
      }

      return split;
   }
}
