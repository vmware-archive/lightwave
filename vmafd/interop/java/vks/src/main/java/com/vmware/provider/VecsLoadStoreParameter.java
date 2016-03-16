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

package com.vmware.provider;

import java.security.KeyStore.LoadStoreParameter;
import java.security.KeyStore.ProtectionParameter;

public class VecsLoadStoreParameter implements LoadStoreParameter{

   private final String _storeName;
   public VecsLoadStoreParameter (String storeName) {
      _storeName = storeName;
   }

   public String getStoreName() {
      return _storeName;
   }

   @Override
   public ProtectionParameter getProtectionParameter() {
      // We are not planning any additional password protection for the VKS. It is done through other mechanisms.
      return null;
   }
}
