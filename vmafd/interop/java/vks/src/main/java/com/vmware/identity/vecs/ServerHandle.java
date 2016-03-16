/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.vecs;

class ServerHandle implements AutoCloseable {
   private PointerRef _handle;
   ServerHandle(PointerRef handle) {
      _handle = handle;
   }

   PointerRef getHandle() {
      return _handle;
   }

   @Override
   public void close() {
      dispose();
   }

   protected void finalize() throws Throwable {
      try {
         dispose();
      } finally {
         super.finalize();
      }
   }

   protected void dispose() {
     if (_handle != null)
     {
        VecsStoreFactory.closeServer(_handle);
        _handle = null;
     }
   }

}
