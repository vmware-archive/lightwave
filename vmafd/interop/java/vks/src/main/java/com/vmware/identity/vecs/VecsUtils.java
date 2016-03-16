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
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.UnrecoverableKeyException;
import java.security.cert.CRLException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509CRL;
import java.security.cert.X509Certificate;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.Collection;

import javax.crypto.spec.SecretKeySpec;

import sun.misc.BASE64Decoder;
import sun.misc.BASE64Encoder;


class VecsUtils {

   public static final String PEM_PRIVATEKEY_PREFIX = "-----BEGIN PRIVATE KEY-----";
   public static final String PEM_PRIVATEKEY_SUFFIX = "-----END PRIVATE KEY-----";
   public static final String PEM_CERTIFICATE_PREFIX = "-----BEGIN CERTIFICATE-----";
   public static final String PEM_CERTIFICATE_SUFFIX = "-----END CERTIFICATE-----";
   public static final String PEM_CRL_PREFIX = "-----BEGIN X509 CRL-----";
   public static final String PEM_CRL_SUFFIX = "-----END X509 CRL-----";

   static PrivateKey getPrivateKeyFromString(String pemPrivateKey)
         throws NoSuchAlgorithmException, UnrecoverableKeyException {
      if (pemPrivateKey == null) {
         return null;
      }

      pemPrivateKey = pemPrivateKey.replace(PEM_PRIVATEKEY_PREFIX, "");
      pemPrivateKey = pemPrivateKey.replace(PEM_PRIVATEKEY_SUFFIX, "");

      byte[] encodedKey;
      BASE64Decoder decoder = new BASE64Decoder();
      try {
         encodedKey = decoder.decodeBuffer(pemPrivateKey);
      } catch (IOException e) {
         UnrecoverableKeyException uke = new UnrecoverableKeyException(
                "Not able to do BASE64 decoding.");
         uke.initCause(e);
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
         uke.initCause(e);
         throw uke;
      }

      return pKey;
   }

   static X509Certificate[] getX509CertificatesFromString(String pemString)
         throws CertificateException {
      if (pemString == null) {
         return null;
      }

      InputStream is = new ByteArrayInputStream(pemString.getBytes());
      Collection<? extends Certificate> certCollection = null;
      CertificateFactory cf = CertificateFactory.getInstance("X509");
      certCollection = cf.generateCertificates(is);
      try {
         is.close();
      } catch (IOException e) {
         throw new IllegalStateException(
               "ByteArrayInputStream failed to close.", e);
      }

      X509Certificate[] x509Certs = certCollection
            .toArray(new X509Certificate[0]);

      return x509Certs;
   }

   static String encodeCharArrToString(char[] arr) {
      if (arr == null) {
         return null;
      }
      String arrString = new String(arr);
      return arrString;
   }

   static String encodeX509CertificatesToString(X509Certificate[] certs)
         throws CertificateEncodingException {
      if (certs == null || certs.length == 0) {
         return null;
      }

      int stringBuilderSize = certs.length * 1024; // approximate string builder
                                                   // size
      StringBuilder sb = new StringBuilder(stringBuilderSize);
      for (X509Certificate cert : certs) {
         String pem = pemEncodeEncodedBytes(cert.getEncoded(),
               PEM_CERTIFICATE_PREFIX, PEM_CERTIFICATE_SUFFIX);
         sb.append(pem);
         sb.append('\n');
      }
      if (sb.length() > 0) {
         sb.deleteCharAt(sb.length() - 1);
      }

      return sb.toString();
   }

   static String encodeX509CRLToString(X509CRL crl) throws CRLException {
      if (crl == null) {
         return null;
      }

      String pem = pemEncodeEncodedBytes(crl.getEncoded(),
            PEM_CRL_PREFIX, PEM_CRL_SUFFIX);
      return pem;
   }

   static String encodeSecretKeyToBase64String(SecretKeySpec key) {
      if (key == null) {
         return null;
      }

      BASE64Encoder encoder = new BASE64Encoder();
      String b64EncodedString = new String(encoder.encode(key.getEncoded()));
      b64EncodedString = b64EncodedString.replaceAll("(\\r|\\n)", "");
      String pem = new String(b64EncodedString);
      return pem;
   }

   static SecretKeySpec getSecretKeyFromString(String base64Str) throws UnrecoverableKeyException {
      if (base64Str == null) {
         return null;
      }
      byte[] byteKey = null;
      BASE64Decoder decoder = new BASE64Decoder();
      try {
         byteKey = decoder.decodeBuffer(base64Str);
      } catch (IOException e) {
         UnrecoverableKeyException uke = new UnrecoverableKeyException(
               "Not able to do BASE64 decoding.");
         uke.initCause(e);
         throw uke;
      }
      SecretKeySpec key = new SecretKeySpec(byteKey, "");

      return key;
   }

   static X509CRL getX509CRLFromString(String pemString)
         throws CertificateException, CRLException {
      if (pemString == null) {
         return null;
      }

      InputStream is = new ByteArrayInputStream(pemString.getBytes());
      CertificateFactory cf = CertificateFactory.getInstance("X509");
      X509CRL crl = (X509CRL)cf.generateCRL(is);
      try {
         is.close();
      } catch (IOException e) {
         throw new IllegalStateException(
               "ByteArrayInputStream failed to close.", e);
      }

      return crl;
   }

   static String encodePrivateKeyToString(PrivateKey key) throws UnsupportedEncodingException {
      if (key == null) {
         return null;
      }
      String pem = pemEncodeEncodedBytes(key.getEncoded(),
            PEM_PRIVATEKEY_PREFIX, PEM_PRIVATEKEY_SUFFIX);

      return pem;
   }

   static private String pemEncodeEncodedBytes(byte[] encodedBytes,
         String prefix, String suffix) {
      BASE64Encoder encoder = new BASE64Encoder();
      String b64EncodedString = new String(encoder.encode(encodedBytes));
      b64EncodedString = b64EncodedString.replaceAll("(\\r|\\n)", "");

      int period = 64;
      int b64EncodedStrLen = b64EncodedString.length();
      int totalCapacity = b64EncodedStrLen + b64EncodedStrLen / period
            + prefix.length() + suffix.length() + 2; // This is for prefix and
                                                     // suffix (\n)'s
      StringBuilder sb = new StringBuilder(totalCapacity);
      sb.append(prefix);
      sb.append('\n');
      for (int beginloc = 0; beginloc < b64EncodedStrLen; beginloc += period) {
         int endloc = beginloc + period;
         if (endloc > b64EncodedStrLen) {
            endloc = b64EncodedStrLen;
         }
         sb.append(b64EncodedString.substring(beginloc, endloc));
         sb.append('\n');
      }
      sb.append(suffix);

      String pem = sb.toString();
      return pem;
   }
}
