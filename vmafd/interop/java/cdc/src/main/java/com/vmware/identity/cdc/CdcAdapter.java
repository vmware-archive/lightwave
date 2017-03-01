/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved
 * ********************************************************************
 */
package com.vmware.identity.cdc;

import java.util.List;

class CdcAdapter {

      static final int ERROR_OBJECT_NOT_FOUND = 4312;
      static final int ERROR_ALREADY_EXISTS = 183;

      static {
        String osName = System.getProperty("os.name");
        boolean isWindows  = osName.startsWith("Windows") ? true : false;
        if (isWindows) {
            System.loadLibrary("libcdcjni");
        } else {
            try {
              System.load("/opt/vmware/lib64/libcdcjni.so");
            }
            catch (UnsatisfiedLinkError e) {
              System.load("/usr/lib/vmware-vmafd/lib64/libcdcjni.so");
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
   CdcEnableClientAffinity(
         PointerRef pServer
         );

   static native int
   CdcDisableClientAffinity(
         PointerRef pServer
         );

   static native int
   CdcGetDCNameW(
         PointerRef pServer,
         String     domainName,
         int        flags,
         CdcDCEntryNative pDcInfo
         );

   static native int
   CdcEnumDCEntriesW(
         PointerRef   pServer,
         List<String> dcEntriesList
         );
   
   static native int
   CdcGetCurrentState(
		 PointerRef   pServer,
		 IntRef       state
		 );

   static native int
   CdcFreeDomainControllerInfoW(
         PointerRef pDcInfo
         );

   static native int
   VmAfdCloseServer(
         PointerRef pServer
         );
}
