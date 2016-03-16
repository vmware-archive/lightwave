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

/**
 * Represents a public information of VECS permission.
 */
public class VecsPermission {
   public enum AccessMask {
      READ(0x40000000),
      WRITE(0x80000000);

      private final int value;

      AccessMask(int value) {
        this.value = value;
      }

      public int getValue() {
        return value;
      }
   }
   /**
    * User name.
    */
   public final String userName;
   /**
    * Access mask.
    */
   public final AccessMask accessMask;

   public static final int READ  = 0x40000000;

   public static final int WRITE = 0x80000000;

   VecsPermission(String userName, AccessMask accessMask) {
      this.userName = userName;
      this.accessMask = accessMask;
   }
}
