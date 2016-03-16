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

package com.vmware.example;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.security.Key;
import java.security.KeyFactory;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;

import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLServerSocketFactory;

import sun.misc.BASE64Decoder;

import com.vmware.provider.VecsLoadStoreParameter;

public class SSLServer {

   public static void main(String[] args) throws Exception {
      SSLContext context;
      KeyManagerFactory kmf;
      KeyStore ks;

      context = SSLContext.getInstance("TLS");
      kmf = KeyManagerFactory.getInstance("SunX509");
      ks = KeyStore.getInstance("VKS");
      ks.load(new VecsLoadStoreParameter("store-name"));
//      addEntry(ks);
//      ks = KeyStore.getInstance("JKS");  
//      ks.load(new FileInputStream("/home/andrey/myservice.jks"),  
//          "myservicestoresecret".toCharArray()); 

      kmf.init(ks, null);
      context.init(kmf.getKeyManagers(), null, null);
      SSLServerSocketFactory ssf = context.getServerSocketFactory();

      ServerSocket ss = ssf.createServerSocket(443);
      while (true) {
        Socket s = ss.accept();
        PrintStream out = new PrintStream(s.getOutputStream());
        out.println("Hi");
        out.close();
        s.close();
      }
   }
   
   private  static void addEntry(KeyStore ks) throws KeyStoreException, UnrecoverableKeyException, NoSuchAlgorithmException, CertificateException {
      Key key = getPrivateKey();
      Certificate cert = getCertificate();
      ks.setKeyEntry("sslentry", key, null, new Certificate[]{cert});
   }
   
   private static PrivateKey getPrivateKey() throws UnrecoverableKeyException, NoSuchAlgorithmException {
      
      String keyString =
            "-----BEGIN PRIVATE KEY-----\n"
                  + "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC9IOnro3PMXTQ8\n"
                  + "sos2IdenBUwEREDV5z/Y6dI4Ia++M7B2Qir/8v/sQg3fSqeP83Li27Ato1sDSiEA\n"
                  + "P8IGqadpuYmVeN6H7U3uyu3Nedxw3w+aPat9njr1qfyF6jT3oUYxAgm1i19qc1E1\n"
                  + "dP7yCAjw1KLb49BkS7HmlYesusSfuK/PAPjxuv/jh/oX1dHb5bsrK5iOAO7owCO1\n"
                  + "ZOIhOwc3fzjXyjSpIhw5uATl2KPoa4V1HEsq0E7xmHWyMn04AjYcqJphhHMP7TcI\n"
                  + "965i0K9GZ74QAiS1qyrm7trrQzf0TAIJoTwU0cQ6UvgjTS3dul0WPmSKSoLQtbuB\n"
                  + "ZkRey9khAgMBAAECggEAEo2RBPGi4XBfOHkXWPcW7aaCX4TGi3VE+SVu3tzHHbR6\n"
                  + "Yg95omQXIPkV3Uq4h1GNep2seb2lpvv1os2eXgL0jlWKHqSdx93KoFs92sNSkawA\n"
                  + "HtRf51HHXOQRBp4EFiq29QXLKAkFEi8/zyur+tg48E8brtB5V+1pBUKz7Yjg0v4i\n"
                  + "6VHwsomi7w/7Sw7kFXfPj/Plkl9+1hIaL/kJFqlDA+/q7UV9xMmYPs/w3a/b7AQQ\n"
                  + "zFSEmPXOIQ1jKzQ3xFYC2B/4vf5llu1maU/GMQ6k1g0ji2wkRRNqyFh02y3zLzwC\n"
                  + "W1Exbfe3ZRZdm7ScS8B3clqSG2x/xu+gSPGlA1UBxQKBgQDh9mFJ4ZTPITawrQye\n"
                  + "v8ikfoPcVvyLz/JYnRX8aVRMRxW2HbNSWjqitjjpL0n+Jo1MPQIM8xEDs8rbfFRp\n"
                  + "rC1xMf+AySU9mpXkV7XFfnsukp2+nqy44FQGI+OHNPxfCrhZQ4IHsApC0I16eGzY\n"
                  + "hIobmwAG9XwlQ8TtDg2y1YsIGwKBgQDWRQ7wY1Gnl7bFz9c14gVlDHuN7w99CWVq\n"
                  + "fVyDYEjHZQvcRgnchfXO80M7gQLG5GRiDuqfKMyCrQnXMMFT6ZNCQ2LIrud86+yX\n"
                  + "1KZQjInUdHoYlhmBSZYQbG1a0HxZrnoSUpVcAfstSEC+nED5tYoe4qeCu6+JfnhX\n"
                  + "7MI2JrLvcwKBgQCkwmuJ3WwN45/nDycGkbyRBJbNELgwkb2Zk2C2nW6T97kDA4D/\n"
                  + "aT8b6VZx4MNJB387ubhHDOy5EF230F4UdzDjOpucog3/bzogNzwY9vKGsO1IUpaK\n"
                  + "bzeVCpIawi3KdxyYKbskM94nfb3LMQfckDfLjSdl65VL7rAmUXr3jPFFRQKBgCVx\n"
                  + "6htub+2m4MUO/mAnQKQZG1qBhaZnRvbmM42t/OoDqZ/0CXMlYgCpmH1EnXSeo0BT\n"
                  + "tQLdgWfwz4Bwv8hnUFnLJu9FSaUWIcGi4vG+rbOYK2IykQB8GKKEIFQ3jROrHyAm\n"
                  + "mH18xyVxtAbbjatV425kU7vQCuwvUt6ivIn0F18rAoGAd7BudEuk22x7akgyZghK\n"
                  + "fyWb5B1Eo7kMX7gQlHLNzcfyxYgBcmlfgb52o4oh+R1efncHYfuIQKDUwH1HSuLt\n"
                  + "ospf0P8pIjV8bF0WwaMOdWzCJIYDPpBjQW72Alf2k7CUikGGyocyspuqkpA5NcwB\n"
                  + "D4q/gd0sEp/kyR2I3P1lBi4=\n"
                  + "-----END PRIVATE KEY-----";
      PrivateKey key = decodePrivateKey(keyString);
      return key;

   }

   private static X509Certificate getCertificate() throws CertificateException {

      String certificateString =
            "-----BEGIN CERTIFICATE-----\n"
                  + "MIIDWTCCAkGgAwIBAgIJAO8rVOfuBWnjMA0GCSqGSIb3DQEBCwUAMGExIDAeBgNV\n"
                  + "BAMTF0NBLCBkYz12c3BoZXJlLGRjPWxvY2FsMQswCQYDVQQGEwJVUzEwMC4GA1UE\n"
                  + "ChMncGEtcmRpbmZyYTMtdm0zLWRoY3A4MDc4LmVuZy52bXdhcmUuY29tMB4XDTE0\n"
                  + "MDMxMzE2NTg0N1oXDTI0MDMxMzA0NTg0N1owPzEwMC4GA1UEAxMncGEtcmRpbmZy\n"
                  + "YTMtdm0zLWRoY3A4MDc4LmVuZy52bXdhcmUuY29tMQswCQYDVQQGEwJVUzCCASIw\n"
                  + "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL0g6eujc8xdNDyyizYh16cFTARE\n"
                  + "QNXnP9jp0jghr74zsHZCKv/y/+xCDd9Kp4/zcuLbsC2jWwNKIQA/wgapp2m5iZV4\n"
                  + "3oftTe7K7c153HDfD5o9q32eOvWp/IXqNPehRjECCbWLX2pzUTV0/vIICPDUotvj\n"
                  + "0GRLseaVh6y6xJ+4r88A+PG6/+OH+hfV0dvluysrmI4A7ujAI7Vk4iE7Bzd/ONfK\n"
                  + "NKkiHDm4BOXYo+hrhXUcSyrQTvGYdbIyfTgCNhyommGEcw/tNwj3rmLQr0ZnvhAC\n"
                  + "JLWrKubu2utDN/RMAgmhPBTRxDpS+CNNLd26XRY+ZIpKgtC1u4FmRF7L2SECAwEA\n"
                  + "AaM2MDQwMgYDVR0RBCswKYIncGEtcmRpbmZyYTMtdm0zLWRoY3A4MDc4LmVuZy52\n"
                  + "bXdhcmUuY29tMA0GCSqGSIb3DQEBCwUAA4IBAQB077A9JyYQzM1IZ+Xgeq5C4C5M\n"
                  + "F1mmc/abeG+sN45eJu5ZMCYRkOOvfCsQKBD5ONTh4rcDQSijnuY8dRFElnJF0pRk\n"
                  + "WLO+L6bLQ67DVdm81A6QxxAO6BIs+EOGIVrJUzi15M0EtplizSwvTvBNS01W7Ipg\n"
                  + "j41QSwhYtWO5I2+CUVr0gF+aTZfSUWC7krnbu735beOMoivW2IIcLMSo0luN3S6u\n"
                  + "ub9mSPSRi3zvIsYnybxdzF+r24AzGfZtjwMMsZoK04mjeoSulhwxFomabv/RQaam\n"
                  + "Xn30s+Jfp33HOERZXDLBMX2cFltavYqwKzt7uUKoLqINX8ZGRnYMYY1Dvbnj\n"
                  + "-----END CERTIFICATE-----";

         InputStream is;
         X509Certificate cert = null;
         is = new ByteArrayInputStream(certificateString.getBytes());
         CertificateFactory cf = CertificateFactory.getInstance("X.509");
         cert = (X509Certificate) cf.generateCertificate(is);
         return cert;
   }

   private static PrivateKey decodePrivateKey(String privateKey) throws NoSuchAlgorithmException, UnrecoverableKeyException {
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
         UnrecoverableKeyException uke =
               new UnrecoverableKeyException("Not able to do BASE64 decoding.");
         uke.addSuppressed(e);
         throw uke;
      }

      KeyFactory rSAKeyFactory = KeyFactory.getInstance("RSA");
      PrivateKey pKey;
      try {
         pKey = rSAKeyFactory.generatePrivate(new PKCS8EncodedKeySpec(encodedKey));
      } catch (InvalidKeySpecException e) {
         UnrecoverableKeyException uke =
               new UnrecoverableKeyException("Not able to generate private key from key spec.");
         uke.addSuppressed(e);
         throw uke;
      }
      return pKey;
   }
}
