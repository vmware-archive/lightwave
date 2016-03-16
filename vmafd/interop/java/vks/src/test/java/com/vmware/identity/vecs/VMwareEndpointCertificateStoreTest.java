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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.UnsupportedEncodingException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.CRLException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.Date;
import java.util.Enumeration;
import java.util.List;

import junit.framework.Assert;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import sun.misc.BASE64Decoder;

public class VMwareEndpointCertificateStoreTest {
   private VecsStoreFactory _factory = VecsStoreFactory.getVecsStoreFactoryViaIPC();
   @BeforeClass
   public static void setUpBeforeClass() throws Exception {
   }

   @AfterClass
   public static void tearDownAfterClass() throws Exception {
   }

   @Before
   public void setUp() throws Exception {
   }

   @After
   public void tearDown() throws Exception {
   }

   @Test
   public void testCreateDeleteStores() throws AlreadyExistsException {
      int times = 20;
      String baseStoreName = "BlahStore";

      for (int i = 0; i < times; i++) {
         Assert.assertNotNull(_factory.createCertStore(baseStoreName + i));
      }

      List<String> certStores = _factory.enumCertStores();
      Assert.assertEquals(times, certStores.size());

      for (int i = 0; i < times; i++) {
         Assert.assertEquals(baseStoreName + i, certStores.get(i));
      }

      for (int i = 0; i < times; i++) {
          _factory.deleteCertStore(baseStoreName + i);
      }

      certStores = _factory.enumCertStores();
      Assert.assertEquals(0, certStores.size());

   }

   @Test
   public void testDeleteNonExistingStore() {
      try {
         _factory.deleteCertStore("nonexisting");
         Assert.assertTrue(true);
      } catch (VecsGenericException vge ) {
         Assert.fail();
      }
   }

   @Test
   public void testAddDeleteCRLs() throws AlreadyExistsException, CertificateException, CRLException {
      String storeName = "BlahStore3";
      VMwareEndpointCertificateStore vecs = _factory.createCertStore(storeName);
      vecs.openStore();
      String baseEntryAlias = "entryAlias";
      X509CRL inCRL = getCRL();

      vecs.addCrlEntry(baseEntryAlias, inCRL);
      VecsEntry entry = vecs.getEntryByAlias(baseEntryAlias, VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2);
      X509CRL outCRL = entry.crl;
      Assert.assertNotNull(outCRL);
      Assert.assertEquals(inCRL, outCRL);

      vecs.deleteEntryByAlias(baseEntryAlias);
      vecs.closeStore();
      _factory.deleteCertStore(storeName);
   }

   @Test
   public void testGetDeleteByNonExistingAlias() throws AlreadyExistsException, CertificateException, UnrecoverableKeyException, NoSuchAlgorithmException, CRLException {
      String storeName = "BlahStore3";
      VMwareEndpointCertificateStore vecs = _factory.createCertStore(storeName);
      vecs.openStore();

      VecsEntry entry = vecs.getEntryByAlias("nonexisting", VecsEntryInfoLevel.ENTRY_INFO_LEVEL_1);
      Assert.assertNull(entry);

      Key key = vecs.getKeyByAlias("nonexisting", null);
      Assert.assertNull(key);

      try {
         vecs.deleteEntryByAlias("nonexistinggg");
         Assert.assertTrue(true);
      } catch (VecsGenericException vge) {
         Assert.fail();
      }
      vecs.closeStore();
      _factory.deleteCertStore(storeName);
   }

   @Test
   public void testDoubleAddEntry() throws AlreadyExistsException, CertificateException, UnrecoverableKeyException, NoSuchAlgorithmException, UnsupportedEncodingException {
      String storeName = "BlahStore3";
      VMwareEndpointCertificateStore vecs = _factory.createCertStore(storeName);
      vecs.openStore();

      String baseEntryAlias = "entryAlias";
      X509Certificate[] inCertChain = new X509Certificate[] {
            getCertificate1()/*, getCertificate2()*/ };
      PrivateKey inKey = getPrivateKey();

      vecs.addPrivateKeyEntry(baseEntryAlias, inCertChain, inKey, null, true);

      try {
         vecs.addPrivateKeyEntry(baseEntryAlias, inCertChain, inKey, null, true);
         Assert.fail();
      } catch (VecsGenericException vge) {
         Assert.assertEquals(VecsAdapter.ERROR_ALREADY_EXISTS, vge.getErrorCode());
      }

      int entryCount = vecs.getEntryCount();
      Assert.assertEquals(1, entryCount);

      vecs.deleteEntryByAlias(baseEntryAlias);
      entryCount = vecs.getEntryCount();
      Assert.assertEquals(0, entryCount);


      vecs.addTrustedCertEntry(baseEntryAlias, getCertificate1());

      try {
         vecs.addTrustedCertEntry(baseEntryAlias, getCertificate1());
         Assert.fail();
      } catch (VecsGenericException vge) {
         Assert.assertEquals(VecsAdapter.ERROR_ALREADY_EXISTS, vge.getErrorCode());
      }

      entryCount = vecs.getEntryCount();
      Assert.assertEquals(1, entryCount);

      vecs.deleteEntryByAlias(baseEntryAlias);
      entryCount = vecs.getEntryCount();
      Assert.assertEquals(0, entryCount);

      vecs.closeStore();
      _factory.deleteCertStore(storeName);
   }

   @Test
   public void testCreateCountGetDeleteEntries() throws AlreadyExistsException,
         CertificateException, UnrecoverableKeyException,
         NoSuchAlgorithmException, UnsupportedEncodingException, CRLException {
      String storeName = "BlahStore3";
      VMwareEndpointCertificateStore vecs = _factory.createCertStore(storeName);
      vecs.openStore();

      int times = 20;
      String baseEntryAlias = "entryAlias";
      X509Certificate[] inCertChain = new X509Certificate[] {
            getCertificate1()/*, getCertificate2()*/ };
      PrivateKey inKey = getPrivateKey();

      for (int i = 0; i < times; i++) {
         vecs.addPrivateKeyEntry(baseEntryAlias + i, inCertChain, inKey, null,
               true);
      }

      int entryCount = vecs.getEntryCount();
      Assert.assertEquals(times, entryCount);

      for (int i = 0; i < times; i++) {
         String alias = baseEntryAlias + i;
         VecsEntry entry = vecs.getEntryByAlias(alias,
               VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2);

         PrivateKey outKey = (PrivateKey) vecs.getKeyByAlias(alias, null);
         Assert.assertEquals(inKey, outKey);
         Assert.assertEquals(alias, entry.alias);
         Assert.assertEquals(inCertChain.length, entry.certificateChain.length);
         Assert.assertEquals(inCertChain[0], entry.certificateChain[0]);
         //Assert.assertEquals(inCertChain[1], entry.certificateChain[1]);
         Assert.assertTrue((new Date()).getTime() - entry.date.getTime() > 5000);
         Assert.assertEquals(VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY,
               entry.entryType);
      }

      for (int i = 0; i < times; i++) {
         vecs.deleteEntryByAlias(baseEntryAlias + i);
      }

      entryCount = vecs.getEntryCount();
      Assert.assertEquals(0, entryCount);

      vecs.closeStore();
      _factory.deleteCertStore(storeName);
   }

   @Test
   public void multiThreadedCreateCountGetDeleteEntries()
         throws CertificateException, UnrecoverableKeyException,
         NoSuchAlgorithmException, UnsupportedEncodingException,
         AlreadyExistsException, CRLException {
      Thread thr1 = new Thread(new Runnable() {

         @Override
         public void run() {
            try {
               testCreateCountGetDeleteEntries();
            } catch (UnrecoverableKeyException | CertificateException
                  | NoSuchAlgorithmException | UnsupportedEncodingException
                  | AlreadyExistsException | CRLException e) {
               // TODO Auto-generated catch block
               e.printStackTrace();
            }

         }
      });
      thr1.run();
      testCreateCountGetDeleteEntries();
   }

   //@Test
   public void testCreateEnumDeleteEntries() throws AlreadyExistsException,
         CertificateException, UnrecoverableKeyException,
         NoSuchAlgorithmException, UnsupportedEncodingException {
      String storeName = "BlahStore4";
      VMwareEndpointCertificateStore vecs = _factory.createCertStore(storeName);
      vecs.openStore();

      int times = 20;
      String baseEntryAlias = "entryAlias";
      X509Certificate[] inCertChain = new X509Certificate[] { getCertificate1() };
      PrivateKey inKey = getPrivateKey();

      for (int i = 0; i < times; i++) {
         vecs.addPrivateKeyEntry(baseEntryAlias + i, inCertChain, inKey, null,
               true);
      }

      int entryCount = vecs.getEntryCount();
      Assert.assertEquals(times, entryCount);

      Enumeration<VecsEntry> entryEnum = vecs
            .enumerateEntries(VecsEntryInfoLevel.ENTRY_INFO_LEVEL_2);
      int k = 0;
      while (entryEnum.hasMoreElements()) {
         String alias = baseEntryAlias + k;
         k++;
         VecsEntry entry = entryEnum.nextElement();

         PrivateKey outKey = (PrivateKey) vecs.getKeyByAlias(alias, null);
         Assert.assertEquals(inKey, outKey);
         Assert.assertEquals(alias, entry.alias);
         Assert.assertEquals(inCertChain.length, entry.certificateChain.length);
         Assert.assertEquals(inCertChain[0], entry.certificateChain[0]);
         Assert.assertTrue((new Date()).getTime() - entry.date.getTime() > 5000);
         Assert.assertEquals(VecsEntryType.CERT_ENTRY_TYPE_PRIVATE_KEY,
               entry.entryType);
      }

      for (int i = 0; i < times; i++) {
         vecs.deleteEntryByAlias(baseEntryAlias + i);
      }

      entryCount = vecs.getEntryCount();
      Assert.assertEquals(0, entryCount);

      vecs.closeStore();
      _factory.deleteCertStore(storeName);
   }

   //@Test
   public void testCreateStoresSetRevokeGetPermissions() throws AlreadyExistsException,
         CertificateException, UnrecoverableKeyException,
         NoSuchAlgorithmException, UnsupportedEncodingException {
      int times = 10;
      String storeName = "BlahStore5";
      String baseUserName = "User"; //For expected existing users

      VMwareEndpointCertificateStore vecs = _factory.createCertStore(storeName);
      vecs.openStore();

      for (int i = 0; i < times; i++) {
         vecs.setPermission(baseUserName + i, (i%2==0)?
                                 VecsPermission.AccessMask.READ: VecsPermission.AccessMask.WRITE);
      }

      List<VecsPermission> pStorePermissions = vecs.getPermissions();

      int permSize = pStorePermissions.size();
      for (int i = 0; i < times; i++) {
         //First entry in permissions list is last entry added with set permission
         VecsPermission permission = pStorePermissions.get(permSize - i - 1);
         Assert.assertTrue(permission.userName.equals(baseUserName + i));
         Assert.assertEquals((i%2==0)?VecsPermission.AccessMask.READ:VecsPermission.AccessMask.WRITE, permission.accessMask);
      }

      for (int i = 0; i < times; i++) {
         vecs.revokePermission(baseUserName + i, (i%2==0)?VecsPermission.AccessMask.READ:VecsPermission.AccessMask.WRITE);
      }

      List<VecsPermission> pStorePermissions2 = vecs.getPermissions();
      Assert.assertEquals(0, pStorePermissions2.size());

      try {
         vecs.setPermission(null, VecsPermission.AccessMask.READ);
         Assert.fail();
      } catch (VecsGenericException vge) {
         Assert.assertEquals(VecsAdapter.ERROR_INVALID_PARAMETER, vge.getErrorCode());
      }

      vecs.closeStore();
      _factory.deleteCertStore(storeName);
   }

   private PrivateKey getPrivateKey() throws UnrecoverableKeyException,
         NoSuchAlgorithmException {
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
            + "t3I2y2Lf4p+M0T7mq0ovD0uI\n" + "-----END PRIVATE KEY-----";
      PrivateKey key = decodePrivateKey(keyString);
      return key;

   }

   private X509CRL getCRL() throws CertificateException, CRLException {
      String crlString = "-----BEGIN X509 CRL-----\n"
            + "MIIBqDCBkQIBATANBgkqhkiG9w0BAQUFADA8MQswCQYDVQQGEwJVUzELMAkGA1UE\n"
            + "CBMCQ0ExDDAKBgNVBAcTA1JlZDESMBAGCSqGSIb3DQEJARYDYUBiFw0xNDA4MDYy\n"
            + "MTI1MTdaFw0xNDA5MDUyMTI1MTdaoCEwHzAdBgNVHRQEFgIUEAAQABAAEAAQABAA\n"
            + "EAAQABAAEAAwDQYJKoZIhvcNAQEFBQADggEBAJfgiM/8/zziCUB9/jnOP/P7V4tX\n"
            + "zy1KLqfxgYEKhYF6DJ/iqzRjvF6x/GBJD18Gnm29dLiwMOcfqZsaRmX1wySjt/Jj\n"
            + "+aMaNqPYcaUkRHy8eQIIIhqig76fX33pKy9E0/lzQE0qjuU5ius/b4PxCSecrRCg\n"
            + "C3XSPjyDjL79bXksdFi6GysMNrUCltYXth4T3/biTNKwcn2laeyHyYZajyQT2lqa\n"
            + "eO/i1TIkmSkOy0wFY/1TTfG65fx4lUmRHGvtFUCGkwSg/YKgzv3psPKchcYdo+WN\n"
            + "/EsDuc4rwqaIJs/VZ61ub2E+NN3+HY3tLe1e2Cnq66/zGiMqL6UvNsxCYfE=\n"
            + "-----END X509 CRL-----\n";
      X509CRL crl = VecsUtils.getX509CRLFromString(crlString);
      return crl;
   }

   private X509Certificate getCertificate1() throws CertificateException {
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

      InputStream is;
      X509Certificate cert = null;
      is = new ByteArrayInputStream(certificateString.getBytes());
      CertificateFactory cf = CertificateFactory.getInstance("X.509");
      cert = (X509Certificate) cf.generateCertificate(is);
      return cert;
   }

   private X509Certificate getCertificate2() throws CertificateException {
      String certificateString = "-----BEGIN CERTIFICATE-----\n"
            + "MIIDODCCAiCgAwIBAgIJANDyfktUDFiaMA0GCSqGSIb3DQEBCwUAMEkxIDAeBgNV"
            + "BAMTF0NBLCBkYz12c3BoZXJlLGRjPWxvY2FsMQswCQYDVQQGEwJVUzEYMBYGA1UE"
            + "ChMPV0lOLTJYOVgyWjhXQTBaMB4XDTE0MDQyODA3NTUyN1oXDTI0MDQyNTA3NTUy"
            + "N1owSTEgMB4GA1UEAxMXQ0EsIGRjPXZzcGhlcmUsZGM9bG9jYWwxCzAJBgNVBAYT"
            + "AlVTMRgwFgYDVQQKEw9XSU4tMlg5WDJaOFdBMFowggEiMA0GCSqGSIb3DQEBAQUA"
            + "A4IBDwAwggEKAoIBAQC1dDp61XdiEvolJjlmr3044f58V7HjJd2M4H7fBl+vRS2K"
            + "GAs1pOzH7vParW+3hRyiCfvKsWSTKeYJQOB0NNghc242U4SWHj0L06DbD9pWifxZ"
            + "3DUS3wiFdhqyw5qdryTW3RAnEoHfl/T5jbAxx/EqJB3ZP7fQvYQZuONxlfhxiZLd"
            + "pSeXX2Dal3rzgavRg04Nd3QMdVD5LDI0KANm7yFFan11uFZtvEexzxCrRbVI0QQx"
            + "Rox0gkvltHg6g7+GWFfLwvlZCXryykfuVjcED813Et8LTK1daTrE4+tiCGeKm0me"
            + "ahEWDGE3pyqHtjcVoWo03J2YXvRSvhN4X7rD/GE/AgMBAAGjIzAhMA4GA1UdDwEB"
            + "/wQEAwIBBjAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQA0QOX3"
            + "j3RyX/+sI4m1znCnulJ20GEENQ4Lo06fV0BCWlr6cfivddpNiSIv/9dTjI18rbav"
            + "GehVkYhXkqXpQpu/pE17EcvmvrI+93PthsMlJeJ422o/XEg2B91+tkiaIXke4/yX"
            + "KjI/Ke2Esjrx5uwhZ5qQgUUBpOkzlkZR3PFKEP+jbTmMCR/2nViRzkEtf9CqVyCN"
            + "AEE6p7MuWiNiwHLXoi+awsM0Dv2xKU+GLhv9wAN0TasQc0xMa6t+tmWCoXEq0WY+"
            + "MI7q+y5izRyEUU7xH+FVY/z2GbXPzg7TFPeJ+23d8n8IDcSojSdliiQVJrMDBeps"
            + "D+nD0ZDUMhS1pQni\n" + "-----END CERTIFICATE-----\n";

      InputStream is;
      X509Certificate cert = null;
      is = new ByteArrayInputStream(certificateString.getBytes());
      CertificateFactory cf = CertificateFactory.getInstance("X.509");
      cert = (X509Certificate) cf.generateCertificate(is);
      return cert;
   }

   private PrivateKey decodePrivateKey(String privateKey)
         throws NoSuchAlgorithmException, UnrecoverableKeyException {
      if (privateKey == null) {
         throw new NullPointerException("pem string key is null");
      }

      String pemPrivateKey = new String(privateKey);
      pemPrivateKey = pemPrivateKey.replace("-----BEGIN PRIVATE KEY-----", "");
      pemPrivateKey = pemPrivateKey.replace("-----END PRIVATE KEY-----", "");

      BASE64Decoder decoder = new BASE64Decoder();
      byte[] encodedKey;
      try {
         encodedKey = decoder.decodeBuffer(pemPrivateKey);
      } catch (IOException e) {
         UnrecoverableKeyException uke = new UnrecoverableKeyException(
               "Not able to do BASE64 decoding.");
         uke.addSuppressed(e);
         throw uke;
      }

      KeyFactory rSAKeyFactory = KeyFactory.getInstance("RSA");
      PrivateKey pKey;
      try {
         pKey = rSAKeyFactory.generatePrivate(new PKCS8EncodedKeySpec(
               encodedKey));
      } catch (InvalidKeySpecException e) {
         UnrecoverableKeyException uke = new UnrecoverableKeyException(
               "Not able to generate private key from key spec.");
         uke.addSuppressed(e);
         throw uke;
      }
      return pKey;
   }

}
