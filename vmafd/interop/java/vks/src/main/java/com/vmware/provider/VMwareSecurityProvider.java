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

import java.security.AccessController;
import java.security.Provider;

public final class VMwareSecurityProvider extends Provider {
   private static final long serialVersionUID = 6919385612889729159L;

   private static final String INFO = "VMware Endpoint Certificate Store"
         + " X.509 certificates; VKS keystore";
   public static String PROVIDER_NAME = "VECS";

   public VMwareSecurityProvider() {
      super(PROVIDER_NAME, 0.1, INFO);
      AccessController
            .doPrivileged(new java.security.PrivilegedAction<Object>() {
               @Override
               public Object run() {
                  put("KeyStore.VKS",
                        "com.vmware.provider.VecsKeyStoreEngine");
                  put("KeyStore.VKS2",
                          "com.vmware.provider.VecsKeyStoreEngine2");
                  put("CertStore.VCS",
                          "com.vmware.provider.VecsCertStoreEngine");
                  return null;
               }
            });
   }
}
