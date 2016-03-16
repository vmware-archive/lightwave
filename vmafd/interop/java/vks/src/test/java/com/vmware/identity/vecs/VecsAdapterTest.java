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

import java.util.ArrayList;
import java.util.List;

import junit.framework.Assert;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

public class VecsAdapterTest {
   private static PointerRef _serverHandle;

   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
      _serverHandle = new PointerRef();
      int error = VecsAdapter.VmAfdOpenServerW(null, null, null, _serverHandle);
      Assert.assertEquals(0, error);
      Assert.assertTrue(!PointerRef.isNull(_serverHandle));
   }

   @AfterClass
   public static void tearDownAfterClass() throws Exception {
      VecsAdapter.VmAfdCloseServer(_serverHandle);
   }

   @Before
   public void setUp() throws Exception {
   }

   @After
   public void tearDown() throws Exception {
   }

   @Test
   public void testCreateOpenCloseDeleteStoreWithoutStoreHandle() {
      String storeName = "BlahStore1";
      int error = VecsAdapter.VecsCreateCertStoreHW(_serverHandle, storeName, "passw",
            null);
      Assert.assertEquals(0, error);

      PointerRef pStore = new PointerRef();
      error = VecsAdapter.VecsOpenCertStoreHW(_serverHandle, storeName, "passw", pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(!PointerRef.isNull(pStore));

      error = VecsAdapter.VecsCloseCertStore(pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(PointerRef.isNull(pStore));

      error = VecsAdapter.VecsDeleteCertStoreHW(_serverHandle, storeName);
      Assert.assertEquals(0, error);
   }

   @Test
   public void testCreateCloseDeleteStoreWithStoreHandle() {
      String storeName = "BlahStore2";
      PointerRef pStore = new PointerRef();
      int error = VecsAdapter.VecsCreateCertStoreHW(_serverHandle, storeName, "passw",
            pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(!PointerRef.isNull(pStore));

      error = VecsAdapter.VecsCloseCertStore(pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(PointerRef.isNull(pStore));

      error = VecsAdapter.VecsDeleteCertStoreHW(_serverHandle, storeName);
      Assert.assertEquals(0, error);
   }

   @Test
   public void testCreateEnumDeleteStoresWithStoreHandl() {
      int times = 10;
      String baseStoreName = "BlahStore";
      int error;

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsCreateCertStoreHW(_serverHandle, baseStoreName + i,
               "passw", null);
         Assert.assertEquals(0, error);
      }

      List<String> storeNameList = new ArrayList<String>();
      error = VecsAdapter.VecsEnumCertStoreHW(_serverHandle, storeNameList);
      for (int i = 0; i < times; i++) {
         Assert.assertEquals(baseStoreName + i, storeNameList.get(i));
      }

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsDeleteCertStoreHW(_serverHandle, baseStoreName + i);
         Assert.assertEquals(0, error);
      }
   }

   @Test
   public void testCreateCountGetDeleteEntries() {
      String storeName = "BlahStore3";
      PointerRef pStore = new PointerRef();
      int error = VecsAdapter.VecsCreateCertStoreHW(_serverHandle, storeName, "passw",
            pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(!PointerRef.isNull(pStore));

      int times = 20;
      String baseEntryAlias = "entryAlias";
      String certBlob = getCertificateBlob();
      String keyBlob = getPrivateKeyBlob();

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsAddEntryW(pStore,
               VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY.getValue(),
               baseEntryAlias + i, certBlob, keyBlob, null, true);
         Assert.assertEquals(0, error);
      }

      IntRef pCount = new IntRef();
      error = VecsAdapter.VecsGetEntryCount(pStore, pCount);
      Assert.assertEquals(0, error);
      Assert.assertEquals(times, pCount.number);

      for (int i = 0; i < times; i++) {
         String alias = baseEntryAlias + i;
         VecsEntryNative pEntry = new VecsEntryNative();
         error = VecsAdapter.VecsGetEntryByAliasW(pStore, alias,
               VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2.getValue(), pEntry);
         Assert.assertEquals(0, error);

         StringRef pKey = new StringRef();
         error = VecsAdapter.VecsGetKeyByAliasW(pStore, alias, null, pKey);
         Assert.assertEquals(0, error);
         Assert.assertTrue(pKey.str.equals(keyBlob));
         Assert.assertTrue(pEntry.alias.equals(alias));
         Assert.assertTrue(pEntry.certificate.equals(certBlob));
         Assert.assertTrue(pEntry.date > 0);
         Assert.assertEquals(
               VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY.getValue(),
               pEntry.entryType);
      }

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsDeleteEntryW(pStore, baseEntryAlias + i);
         Assert.assertEquals(0, error);
      }

      error = VecsAdapter.VecsCloseCertStore(pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(PointerRef.isNull(pStore));

      error = VecsAdapter.VecsDeleteCertStoreHW(_serverHandle, storeName);
      Assert.assertEquals(0, error);
   }

   //@Test
   public void testCreateEnumDeleteEntries() {
      String storeName = "BlahStore4";
      PointerRef pStore = new PointerRef();
      int error = VecsAdapter.VecsCreateCertStoreHW(_serverHandle, storeName, "passw",
            pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(!PointerRef.isNull(pStore));

      int times = 20;
      String baseEntryAlias = "entryAlias";
      String certBlob = getCertificateBlob();
      String keyBlob = getPrivateKeyBlob();

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsAddEntryW(pStore,
               VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY.getValue(),
               baseEntryAlias + i, certBlob, keyBlob, null, true);
         Assert.assertEquals(0, error);
      }

      PointerRef pEnumContext = new PointerRef();
      error = VecsAdapter.VecsBeginEnumEntries(pStore, 100,
            VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2.getValue(), pEnumContext);
      Assert.assertEquals(0, error);

      List<VecsEntryNative> pEntries = new ArrayList<VecsEntryNative>();
      error = VecsAdapter.VecsEnumEntriesW(pEnumContext, pEntries);
      Assert.assertEquals(0, error);
      Assert.assertEquals(times, pEntries.size());

      for (int i = 0; i < times; i++) {
         String alias = baseEntryAlias + i;
         VecsEntryNative pEntry = pEntries.get(i);
         Assert.assertTrue(pEntry.alias.equals(alias));
         Assert.assertTrue(pEntry.certificate.equals(certBlob));
         Assert.assertTrue(pEntry.date > 0);
         Assert.assertEquals(
               VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY.getValue(),
               pEntry.entryType);
      }

      pEntries = new ArrayList<VecsEntryNative>();
      error = VecsAdapter.VecsEnumEntriesW(pEnumContext, pEntries);
      Assert.assertEquals(0, error);
      Assert.assertEquals(0, pEntries.size());

      error = VecsAdapter.VecsEndEnumEntries(pEnumContext);
      Assert.assertEquals(0, error);

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsDeleteEntryW(pStore, baseEntryAlias + i);
         Assert.assertEquals(0, error);
      }

      error = VecsAdapter.VecsCloseCertStore(pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(PointerRef.isNull(pStore));

      error = VecsAdapter.VecsDeleteCertStoreHW(_serverHandle, storeName);
      Assert.assertEquals(0, error);
   }

   //@Test
   public void testCreateStoresSetRevokeGetPermissions() {
      int times = 10;
      String storeName = "BlahStore5";
      String baseUserName = "User"; //For expected existing users
      //Authorization Access Masks
      final int READ_STORE =  0x40000000;
      final int WRITE_STORE = 0x80000000;
      PointerRef pStore = new PointerRef();

      int error = VecsAdapter.VecsCreateCertStoreHW(_serverHandle, storeName, "passw",
            pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(!PointerRef.isNull(pStore));

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsSetPermissionW(pStore, baseUserName + i, (i%2==0)?READ_STORE:WRITE_STORE);
         Assert.assertEquals(0, error);
      }

      StringRef pOwner = new StringRef();
      List<VecsPermissionNative> pStorePermissions = new ArrayList<VecsPermissionNative>();
      error = VecsAdapter.VecsGetPermissionsW(pStore, pOwner, pStorePermissions);
      Assert.assertEquals(0, error);

      int permSize = pStorePermissions.size();
      for (int i = 0; i < times; i++) {
         //First entry in permissions list is last entry added with set permission
         VecsPermissionNative permission = pStorePermissions.get(permSize - i - 1);
         Assert.assertTrue(permission.userName.equals(baseUserName + i));
         Assert.assertEquals((i%2==0)?READ_STORE:WRITE_STORE, permission.accessMask);
      }

      for (int i = 0; i < times; i++) {
         error = VecsAdapter.VecsRevokePermissionW(pStore, baseUserName + i, (i%2==0)?READ_STORE:WRITE_STORE);
         Assert.assertEquals(0, error);
      }

      StringRef pOwner2 = new StringRef();
      List<VecsPermissionNative> pStorePermissions2 = new ArrayList<VecsPermissionNative>();
      error = VecsAdapter.VecsGetPermissionsW(pStore, pOwner2, pStorePermissions2);
      Assert.assertEquals(0, error);
      Assert.assertEquals(0, pStorePermissions2.size());

      error = VecsAdapter.VecsCloseCertStore(pStore);
      Assert.assertEquals(0, error);
      Assert.assertTrue(PointerRef.isNull(pStore));

      error = VecsAdapter.VecsDeleteCertStoreHW(_serverHandle, storeName);
      Assert.assertEquals(0, error);
   }

   private String getPrivateKeyBlob() {
      String keyString = "-----BEGIN PRIVATE KEY-----\n"
            + "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC/Pf0mKuCRlDXu\n"
            + "yxkpKgoe56U9Cja2+FCUT+MRNO9OiEZJtQUMNhxOiHO08cNtkPvV3Rp7ceFjpcD7\n"
            + "EQP6MvHGCgsfwnwlBydEKOYAm3Q1pNG/UZpgLyp2w9d30+Xw/bYIFr9Fxpf+5qIz\n"
            + "70/Dz1eD0B1T++O17I6ytZJzeGZ2A+uJUcBypI666vruKx9vUopj2TZUFsQ2aVST\n"
            + "Kq/Wa2rqB+ooH2GRnz8WSaQi6AIEmyIjlwE7ywdDxxT755jFEM3sfDtJpLSHkH7R\n"
            + "qyhnTqTiW4WqmZKEJfzsc4+9c9tV4fuhp3JhTWaKV1YEsq4IQw0CSQ87msGLzoh0\n"
            + "uCz127bnAgMBAAECggEAAumonuRdSj+rI0lwRypkQBHqzYddIPlJj5C4lnerbhNG\n"
            + "RXNz3GVAlh5JljJudbnqaVig7CC7vD73NaZjMfQvrtf9UQ0EbgicsP2RzzrkO2Lu\n"
            + "F49gc2GUhdLix75Foh+DHmlrnV5HzLKU5YdEvD3pXnu/eUV8s4si+0F9CwmE6fw2\n"
            + "OErHKu3JZFuCpEJYCaCDZmaxVM1GofqcWqgpl2Z7trMfeo8zeZZAmDrxGHzGqoL3\n"
            + "0JJhVxtqORAanmCq7wjQuQdiex7gBIhPrPBB8xwRQ3nHl/4O3eFybMVlj29qi3sQ\n"
            + "GiwEvf8hjy+gu3ntd9dnINM4zRlsc635tr494jgBgQKBgQDT/uBGErBrPXcSU9SN\n"
            + "J/aGWRznLS8MoRJ4AI2ZgpHAV7r2Csw4Wwp5ypxioeFhjgLb6CRdthHtiS0ZRFU1\n"
            + "dgH3ee9huYDzoKFuIibGCmvpwjERKDrusrBXoS0FxO2LyslMlJJLn2ii8+9XLJ26\n"
            + "o/OfrXoBQdFFf4yDSzC90j12ZwKBgQDm8E4jYNzuaBADgYHLxz1RY7/s99C8sK0p\n"
            + "9UPQjTHn0iksRwpmb4tyzVsaEYl5BXUC8PqMOsSvbX25mGFHjuXdXo+KT/k6v8fQ\n"
            + "i/hsRV8b2LpRLMFU4kNtacF5YtZrhmjaKdGt0+v5PJf58cX44TObm8Lh85L4tWsr\n"
            + "EFIkLKprgQKBgEUHjUfD9iY1UXxqR5/fD2sXwU2VbOiT2kuxAmqmFYeoCXzsbys6\n"
            + "meKmkt2bEKSPdWbd5FmPW68ZJ9I+afCKiIFo9wW/PboW5/nQrQd3hUQMs6V+kBIB\n"
            + "pWIjXWGvihom4f1Js4lkUtIc1CZypLmNgVrRRhT6tYYHzl1CMpDc0J6pAoGBANd+\n"
            + "cUtt/XqxvA+pb1aZykwCSntzG2KXcl5usSMQPftWDnl0qO3BNFyh51rB9ofpYbCm\n"
            + "8QSdah9Qijr/R/cgDrqsnECyM5xwjKG7mspdSUyQxfstShJNOCIGxzTf17lqKLk6\n"
            + "wJ/12oIt2pqu0s9URXX0uLtjMEfH8gKzLhL3YucBAoGBALwrCT3p5XfED4o2GOhk\n"
            + "d0m3vWzYMoBYiOKUm1E36PvkzGjErWbEJ/pom4zd41EdYkRy/dDiXbm3gd+jFkty\n"
            + "qGc1I2Kvty01aFRKr290hC51IUyWV4EPTnosNr55+HjxsXqy0NHxOCmeH/cpn/Jb\n"
            + "t3I2y2Lf4p+M0T7mq0ovD0uI\n" + "-----END PRIVATE KEY-----\n";
      return keyString;
   }

   private String getCertificateBlob() {
      String certificateString = "-----BEGIN CERTIFICATE-----\n"
            + "MIIDkDCCAnigAwIBAgIRAN+NbA6d3U+aKpXIRKKhYfowDQYJKoZIhvcNAQELBQAw\n"
            + "TzELMAkGA1UEBhMCVVMxDzANBgNVBAoTBlZNd2FyZTEbMBkGA1UECxMSVk13YXJl\n"
            + "IEVuZ2luZWVyaW5nMRIwEAYDVQQDEwlDZXJUb29sQ0EwHhcNMTIxMTIxMDAzNjE3\n"
            + "WhcNMjIxMTE5MDAzNjE3WjBPMQswCQYDVQQGEwJVUzEPMA0GA1UEChMGVk13YXJl\n"
            + "MRswGQYDVQQLExJWTXdhcmUgRW5naW5lZXJpbmcxEjAQBgNVBAMTCUNlclRvb2xD\n"
            + "QTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALqfVt5lOc1gPIgy27kl\n"
            + "PvbAS+sLWpReg4NbfUx3gOMA5Ga18nBfy4pbmIv5tZIg95TFresULi83lZwZOJ6M\n"
            + "svOW7Lu1UQPDofBEiqZ0j7GFj0qbW4oUP3ePqTwBIRr2QXhgRPHhR9wxKfc5Yvl7\n"
            + "trmkaQvSbit+7OtHoirSaBCyLnU1D35UbSzT1qINkrgzS/aCHk4r2wuQDaIWCi4O\n"
            + "FdaSgMldEoFD6lhxnDXjZpXP3Cdkd20aIP7dDNj2EXJhgxtGih9HFd9tEEB6wOem\n"
            + "VRLchBLno9RF98P4tvjlfbuIQImrA4WoWp7VlcFbc/RlPOe0ubOHZ/RXDLO90KRl\n"
            + "UmsCAwEAAaNnMGUwEgYDVR0TAQH/BAgwBgEB/wIBATAOBgNVHQ8BAf8EBAMCAQYw\n"
            + "HQYDVR0OBBYEFB4QOUMmKZ3oVHst0ghXZTB+xwgWMCAGA1UdEQQZMBeBD3ZtY2FA\n"
            + "dm13YXJlLmNvbYcEfwAAATANBgkqhkiG9w0BAQsFAAOCAQEAVREDPIqZI90veiXV\n"
            + "hrwXuay9HQpyFOfPq8wXQlrAXsO83toWsDK8bFhiRBwS4qmlI3kIhu25hKUBdyJG\n"
            + "KSAoSojJkMtHhZV4pHWG6h3lUElZ/qSwfgZHfougaN/2MYmx+KL4hjqvXeJhD6ps\n"
            + "zHeNAk2az4LI1u2Xt2CBNKxOLYOgjInVNlF9qlF+EcZgr9xKtXnKcBK3c7ErWLtX\n"
            + "6oM7ZMbGvHd49+sKS0cy9RWomemhS6+LtvBb1Bk9gafmRR7nMfqHBWM0OKg0Wtfj\n"
            + "w6v8QfJWLI4MeBexS5VV2zLAOH3FD6GMJSmICkRKsVuBd7aqBEn2RMbzyW0bIvHr\n"
            + "8vVU/A==\n" + "-----END CERTIFICATE-----\n";

      return certificateString;
   }
}
