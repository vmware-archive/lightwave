/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * ********************************************************************
 */
package com.vmware.identity.heartbeat;

class VmAfdHeartbeatAdapter {

      static final int ERROR_OBJECT_NOT_FOUND = 4312;
      static final int ERROR_ALREADY_EXISTS = 183;

      static {
        String osName = System.getProperty("os.name");
        boolean isWindows  = osName.startsWith("Windows") ? true : false;
        if (isWindows) {
          System.loadLibrary("libheartbeatjni");
        } else {
            try {
              System.load("/opt/vmware/lib64/libheartbeatjni.so");
            }
            catch (UnsatisfiedLinkError e) {
              System.load("/usr/lib/vmware-vmafd/lib64/libheartbeatjni.so");
            }
        }
      }

   static native int
   VmAfdOpenServerW(
         String pwszServerName,
         String pwszUserName,
         String pwszPassword,
         PointerRef pServer
         );

   static native int
   VmAfdStartHeartBeatW(
         String pszServiceName,
         int port,
         PointerRef pHandle
         );

   static native int
   VmAfdStopHeartbeat(
         PointerRef pHandle
         );

   static native int
   VmAfdCloseServer(
         PointerRef pServer
         );
}
