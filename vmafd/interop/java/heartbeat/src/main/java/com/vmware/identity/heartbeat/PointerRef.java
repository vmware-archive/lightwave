/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * ********************************************************************
 */
package com.vmware.identity.heartbeat;

public class PointerRef {
   private static long NULL_VALUE = 0L;

   private long pointer = NULL_VALUE;

   static boolean isNull(PointerRef ptr) {
      return (ptr == null || ptr.pointer == NULL_VALUE);
   }
}
