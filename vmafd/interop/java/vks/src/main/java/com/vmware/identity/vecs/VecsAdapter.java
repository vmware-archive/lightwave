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

import java.util.List;

class VecsAdapter {

   static final int ERROR_OBJECT_NOT_FOUND = 4312;
//   static final int ERROR_NO_MORE_ITEMS = 259;
   static final int ERROR_ALREADY_EXISTS = 183;
   static final int ERROR_INVALID_PARAMETER = 87;

//   static final int VECS_BASE_ERROR = 90000;
   // private static final int VECS_GENERIC_FILE_IO = VECS_BASE_ERROR + 1;
   // private static final int VECS_CRL_OPEN_ERROR = VECS_BASE_ERROR + 2;
   // private static final int VECS_BUFFER_SIZE_ERROR = VECS_BASE_ERROR + 3;
   // private static final int VECS_CRL_WRITE_ERROR = VECS_BASE_ERROR + 4;
   // private static final int VECS_ALIAS_NAME_ERROR = VECS_BASE_ERROR + 5;
   // private static final int VECS_UNIQUE_ALIAS_ERROR = VECS_BASE_ERROR + 6;
   // static final int VECS_NO_CERT_FOUND = VECS_BASE_ERROR + 7;
   // private static final int VECS_CRL_IO_ERROR = VECS_BASE_ERROR + 8;
   // private static final int VECS_UNKNOW_ERROR = VECS_BASE_ERROR + 101;

   static {
      String osName = System.getProperty("os.name");
      boolean isWindows  = osName.startsWith("Windows") ? true : false;
      if (isWindows) {
         System.loadLibrary("libvecsjni");
      } else {
            try {
              System.load("/opt/vmware/lib64/libvecsjni.so");
            }
            catch (UnsatisfiedLinkError e) {
              System.load("/usr/lib/vmware-vmafd/lib64/libvecsjni.so");
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
   VecsCreateCertStoreHW(
         PointerRef pServer,
         String storeName,
         String password,
         PointerRef pStore
         );

   static native int
   VecsOpenCertStoreHW(
         PointerRef pServer,
         String storeName,
         String password,
         PointerRef pStore
         );

   static native int
   VecsEnumCertStoreHW(
         PointerRef pServer,
         List<String> storeNameList
         );

   static native int
   VecsAddEntryW(PointerRef pStore,
         int entryType,
         String alias,
         String certificate,
         String privateKey,
         String password,
         boolean bAutoRefresh
         );

   static native int
   VecsGetEntryByAliasW(
         PointerRef pStore,
         String alias,
         int infoLevel,
         VecsEntryNative pEntry
         );

   static native int
   VecsGetKeyByAliasW(
         PointerRef pStore,
         String pszAlias,
         String pszPassword,
         StringRef pKey
         );

   static native int
   VecsGetEntryCount(
         PointerRef pStore,
         IntRef count
         );

   static native int
   VecsBeginEnumEntries(
         PointerRef pStore,
         int maxEntryCount,
         int infoLevel,
         PointerRef pEnumContext
         );

   static native int
   VecsEnumEntriesW(
         PointerRef pEnumContext,
         List<VecsEntryNative> pEntries
         );

   static native int
   VecsEndEnumEntries(
         PointerRef pEnumContext
         );

   static native int
   VecsDeleteEntryW(
         PointerRef pStore,
         String alias
         );

   static native int
   VecsSetPermissionW(
         PointerRef pStore,
         String userName,
         int accessMask
         );

   static native int
   VecsRevokePermissionW(
         PointerRef pStore,
         String userName,
         int accessMask
         );

   static native int
   VecsGetPermissionsW(
         PointerRef pStore,
         StringRef owner,
         List<VecsPermissionNative> pStorePermissions
         );

   static native int
   VecsCloseCertStore(
         PointerRef pStore
         );

   static native int
   VecsDeleteCertStoreHW(
         PointerRef pServer,
         String storeName
         );

   static native int
   VmAfdCloseServer(
         PointerRef pServer
         );
}
