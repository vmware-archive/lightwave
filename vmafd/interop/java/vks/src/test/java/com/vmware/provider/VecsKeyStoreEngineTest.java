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

import static org.junit.Assert.assertNotNull;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.security.Key;
import java.security.KeyFactory;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.Provider;
import java.security.Security;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.Enumeration;

import javax.crypto.spec.SecretKeySpec;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.vecs.AlreadyExistsException;
import com.vmware.identity.vecs.VecsStoreFactory;

import sun.misc.BASE64Decoder;

public class VecsKeyStoreEngineTest {

   private static final String _storeName = "store-name";
   private static KeyStore _keyStore;
   private static VecsStoreFactory _factory = VecsStoreFactory.getVecsStoreFactoryViaIPC();

   @BeforeClass
   static public void init() throws Exception {
      Security.addProvider(new VMwareSecurityProvider());
      try {
         _factory.createCertStore(_storeName);
      } catch (AlreadyExistsException e) {

      }
      _keyStore = KeyStore.getInstance("VKS");
      _keyStore.load(new VecsLoadStoreParameter(_storeName));
   }

   @AfterClass
   static public void destroy() throws Exception {
      _factory.deleteCertStore(_storeName);
   }

   @Test
   public void test() {
      String PROVIDER_NAME = "VECS";
      System.out.println("ClassPath : " + System.getProperty("java.classpath"));
      Provider p = Security.getProvider(PROVIDER_NAME);
      assertNotNull(p);
      System.out.println("Name : " + p.getName());
      System.out.println("version " + p.getVersion());
      System.out.println("info " + p.getInfo());
   }

   @Test
   public void setAndGetCertificateEntry() throws KeyStoreException,
         CertificateException {
      X509Certificate inCert = getCertificate();
      String alias = "pubcert";

      _keyStore.setCertificateEntry(alias, inCert);

      Certificate outCert = _keyStore.getCertificate(alias);

      Assert.assertEquals(inCert, outCert);

      _keyStore.deleteEntry(alias);
   }

   @Test
   public void setAndGetPrivateKeyEntry() throws CertificateException,
         UnrecoverableKeyException, NoSuchAlgorithmException, KeyStoreException {

      X509Certificate cert = getCertificate();
      PrivateKey inKey = getPrivateKey();
      String alias = "privatentry";

      _keyStore.setKeyEntry(alias, inKey, null, new X509Certificate[] { cert });

      Key outKey = _keyStore.getKey(alias, null);
      Assert.assertEquals(inKey, outKey);

      _keyStore.deleteEntry(alias);
   }

   @Test
   public void setAndGetSecretKeyEntry() throws KeyStoreException, UnrecoverableKeyException, NoSuchAlgorithmException {
      String secret = "my_$ecret!@#%\\";
      SecretKeySpec inKey = new SecretKeySpec(secret.getBytes(), "myalgo");
      String alias = "secretentry";

      _keyStore.setKeyEntry(alias, inKey, null, null);

      Key outKey = _keyStore.getKey(alias, null);
      String outSecret = new String(outKey.getEncoded());
      Assert.assertTrue(secret.equals(outSecret));

      _keyStore.deleteEntry(alias);
   }

   @Test
   public void enumAliasesAndCheckStoreSizeAndCheckTheEntryTypes()
         throws CertificateException, UnrecoverableKeyException,
         NoSuchAlgorithmException, KeyStoreException {
      // setup
      X509Certificate cert = getCertificate();
      PrivateKey inKey = getPrivateKey();
      String privAlias = "privatentry";
      int privCount = 10;
      String publAlias = "publicentry";
      int publCount = 10;

      int i;
      for (i = 0; i < privCount; i++) {
         _keyStore.setKeyEntry(privAlias + i, inKey, null,
               new X509Certificate[] { cert });
      }

      int j;
      for (j = 0; j < publCount; j++) {
         _keyStore.setCertificateEntry(publAlias + j, cert);
      }

      Enumeration<String> aliasEnum = _keyStore.aliases();
      i = 0;
      String alias = null;
      while (aliasEnum.hasMoreElements()) {
         i++;
         alias = aliasEnum.nextElement();
         Assert.assertNotNull(alias);
      }

      Assert.assertEquals(privCount + publCount, i);

      int storeSize = _keyStore.size();
      Assert.assertEquals(privCount + publCount, storeSize);

      // getEntryTypeByAlias test
      for (i = 0; i < privCount; i++) {
         Assert.assertTrue(_keyStore.containsAlias(privAlias + i));
         Assert.assertTrue(_keyStore.isKeyEntry(privAlias + i));
      }

      for (j = 0; j < publCount; j++) {
         Assert.assertTrue(_keyStore.containsAlias(publAlias + j));
         Assert.assertTrue(_keyStore.isCertificateEntry(publAlias + j));
      }

      // clean up
      for (i = 0; i < privCount; i++) {
         _keyStore.deleteEntry(privAlias + i);
      }

      for (j = 0; j < publCount; j++) {
         _keyStore.deleteEntry(publAlias + j);
      }
   }

   @Test
   public void containsAliasNegative() throws KeyStoreException {
      boolean bRes =  _keyStore.containsAlias("non-existing");
      Assert.assertFalse(bRes);
   }

   @Test
   public void multiThreadedEnumAliasesAndCheckStoreSizeAndCheckTheEntryTypes()
         throws CertificateException, UnrecoverableKeyException,
         NoSuchAlgorithmException, KeyStoreException {
      Thread thr1 = new Thread(new Runnable() {

         @Override
         public void run() {
            // TODO Auto-generated method stub
            try {
               enumAliasesAndCheckStoreSizeAndCheckTheEntryTypes();
            } catch (UnrecoverableKeyException e) {
               // TODO Auto-generated catch block
               e.printStackTrace();
            } catch (CertificateException e) {
               // TODO Auto-generated catch block
               e.printStackTrace();
            } catch (NoSuchAlgorithmException e) {
               // TODO Auto-generated catch block
               e.printStackTrace();
            } catch (KeyStoreException e) {
               // TODO Auto-generated catch block
               e.printStackTrace();
            }
         }

      });
      thr1.run();
      enumAliasesAndCheckStoreSizeAndCheckTheEntryTypes();
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

   private X509Certificate getCertificate() throws CertificateException {
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
