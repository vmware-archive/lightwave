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

import java.util.Map;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.apache.commons.collections.map.LRUMap;

import com.vmware.identity.sts.Request;

/**
 * Multithread-safe storage of STS requests which keeps only the most recent N
 * requests.
 */
final class LRURequests {

   private final ReadWriteLock lock = new ReentrantReadWriteLock();
   private final Map<String, Request> requests;

   @SuppressWarnings("unchecked")
   LRURequests(int capacity) {
      assert capacity > 0;
      requests = new LRUMap(capacity);
   }

   Request retrieve(String id) {
      lock.readLock().lock();
      try {
         return requests.get(id);
      } finally {
         lock.readLock().unlock();
      }
   }

   void save(String id, Request req) {
      lock.writeLock().lock();
      try {
         requests.put(id, req);
      } finally {
         lock.writeLock().unlock();
      }
   }

   void remove(String id) {
      lock.writeLock().lock();
      try {
         requests.remove(id);
      } finally {
         lock.writeLock().unlock();
      }
   }
}
