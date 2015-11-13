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
package com.vmware.identity.saml.impl;

import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.saml.Advice;

/**
 * Determines actual advice to be put in the token being issued.
 */
final class AdviceFilter {

   /**
    * Determines advice that should be included in the token. Requested advice
    * are returned if the requester is token owner, otherwise - an union of
    * requested and present advice. Requested advice that intersect with some
    * of the present ones are ignored.
    *
    * @param tokenOwner
    *           whether the requester is token owner
    * @param requested
    *           advice requested, not null
    * @param present
    *           pieces of advice that are included in the authentication token. Null if
    *           no authentication token is used.
    * @return not null collection of pieces of advice to be put in the token
    */
   static List<Advice> processRequest(boolean tokenOwner,
      List<Advice> requested, List<Advice> present) {
      assert requested != null;

      final List<Advice> result = tokenOwner ? requested : combineNoIntersec(
         requested, present);

      assert result != null;
      return result;
   }

   private static List<Advice> combineNoIntersec(List<Advice> requested,
      List<Advice> present) {
      assert requested != null;

      final List<Advice> result = new ArrayList<Advice>();
      if (present != null) {
         result.addAll(present);
      }
      for (Advice adviceReq : requested) {
         if (!intersect(result, adviceReq)) {
            result.add(adviceReq);
         }
      }
      return result;
   }

   private static boolean intersect(List<Advice> advice, Advice singleAdvice) {
      boolean result = false;
      for (Advice adviceMember : advice) {
         if (adviceMember.intersect(singleAdvice)) {
            result = true;
            break;
         }
      }
      return result;
   }
}
