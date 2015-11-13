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

import junit.framework.Assert;

import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.sts.Request;

/**
 * Insert your comment for LRURequestsTest here
 */
public final class LRURequestsTest {

   private static final int CAPACITY = 10;
   private static final String KEY1 = "key1";
   private static final String KEY2 = "key2";

   @Test
   public void testSaveGet() {
      final LRURequests requests = new LRURequests(CAPACITY);
      assertNo(requests, KEY1);

      saveRequest(requests, KEY1);
      assertPresent(requests, KEY1);
   }

   @Test
   public void testSaveRemove() {
      final LRURequests requests = new LRURequests(CAPACITY);
      assertNo(requests, KEY1);
      saveRequest(requests, KEY2);
      assertPresent(requests, KEY2);

      requests.remove(KEY1);
      assertNo(requests, KEY1);
      assertPresent(requests, KEY2);

      saveRequest(requests, KEY1);
      assertPresent(requests, KEY1);
      assertPresent(requests, KEY2);

      requests.remove(KEY1);
      assertNo(requests, KEY1);
      assertPresent(requests, KEY2);

      requests.remove(KEY2);
      assertNo(requests, KEY1);
      assertNo(requests, KEY2);
   }

   @Test
   public void testOverflow() {
      final LRURequests requests = new LRURequests(1);

      saveRequest(requests, KEY1);
      assertPresent(requests, KEY1);

      saveRequest(requests, KEY2);
      assertNo(requests, KEY1);
      assertPresent(requests, KEY2);
   }

   private void assertPresent(LRURequests requests, String id) {
      Assert.assertNotNull(requests.retrieve(id));
   }

   private void assertNo(LRURequests requests, String id) {
      Assert.assertNull(requests.retrieve(id));
   }

   private void saveRequest(LRURequests requests, String id) {
      requests.save(id, new Request(new SecurityHeaderType(),
         new RequestSecurityTokenType(), null, null, null));
   }
}
