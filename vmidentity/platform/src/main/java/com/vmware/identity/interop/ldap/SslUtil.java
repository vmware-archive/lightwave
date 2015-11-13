/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package com.vmware.identity.interop.ldap;

import java.io.ByteArrayInputStream;
import java.security.MessageDigest;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateFactory;


public class SslUtil {

   private static final String DIGEST_ALGORITHM = "SHA-1";
   private static final String HEX = "0123456789ABCDEF";

   public static byte[] computeCertificateThumbprint(X509Certificate cert)
         throws Exception {
      String digestAlgorithm = DIGEST_ALGORITHM;
      MessageDigest md = MessageDigest.getInstance(digestAlgorithm);
      return md.digest(cert.getEncoded());
   }

   public static String computeHexCertificateThumbprint(X509Certificate cert) throws Exception
   {
       byte[] digest = computeCertificateThumbprint(cert);
       StringBuilder thumbprint = new StringBuilder();
       for (int i = 0, len = digest.length; i < len; ++i) {
          if (i > 0) {
             thumbprint.append(':');
          }
          byte b = digest[i];
          thumbprint.append(HEX.charAt((b & 0xF0) >> 4));
          thumbprint.append(HEX.charAt(b & 0x0F));
       }
       return thumbprint.toString();
   }

   public static X509Certificate decodeCertificate(byte[] cert)
         throws CertificateException {
      CertificateFactory cf = CertificateFactory.getInstance("X.509");
      return (X509Certificate) cf.generateCertificate(new ByteArrayInputStream(cert));
   }
}
