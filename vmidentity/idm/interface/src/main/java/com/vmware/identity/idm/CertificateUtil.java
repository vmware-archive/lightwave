/*
 *
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */
package com.vmware.identity.idm;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.X509Certificate;

public final class CertificateUtil {

   private static final String FINGERPRINT_ALGORITHM = "SHA-1";

   /**
    * Gets SHA-1 hash over the given certificate and return a properly
    * formatted figureprint string. This is similar to the Thumbnail concept in
    * .NET, except Java doesn't support this as a property by default. This
    * field could be used to uniquely identify a certificate.
    */
   public static String generateFingerprint(X509Certificate cert)
           throws CertificateEncodingException, NoSuchAlgorithmException,
           CertificateEncodingException {

       try {
           return generateFingerprint(cert.getEncoded());

       } catch (CertificateEncodingException e) {
           // TODO: Add logging.
           throw e;
       } catch (NoSuchAlgorithmException e) {
           // TODO: Add logging.
           throw e;
       }
   }

   /**
    * Calculate SHA-1 hash over the given data (certificate bytes) and return
    * properly formatted string in lowercase.<br/>
    * A properly formatted string is in the form:
    * <hex_digit>:<hex_digit>:<hex_digit>, e.g. ab:03:1f
    */
   public static String generateFingerprint(byte[] certificateDer) throws NoSuchAlgorithmException {

      ValidateUtil.validateNotEmpty(certificateDer, "DER certificate");

      MessageDigest digest = MessageDigest.getInstance(FINGERPRINT_ALGORITHM);
      // Calculate SHA-1 hash
      byte[] hash = digest.digest(certificateDer);

      // These steps will produce fingerprint formatted as string:

      // 1. Apply 0xff mask to remove byte's sign extension (use unsigned byte)
      // 2. Convert every byte to hex number
      // 3. Format every hex number as two characters
      // 4. Put ':' delimiter between every two characters

      final char delimiter = ':';

      // Calculate the number of characters in our fingerprint
      // ('# of bytes' * 2) chars + ('# of bytes' - 1) chars for delimiters
      final int len = hash.length * 2 + hash.length - 1;
      // Typically SHA-1 algorithm produces 20 bytes, i.e. len should be 59
      StringBuilder fingerprint = new StringBuilder(len);

      for (int i = 0; i < hash.length; i++) {
         // Step 1: unsigned byte
         hash[i] &= 0xff;

         // Steps 2 & 3: byte to hex in two chars
         // Lower cased 'x' at '%02x' enforces lower cased char for hex value!
         fingerprint.append(String.format("%02x", hash[i]));

         // Step 4: put delimiter
         if (i < hash.length - 1) {
            fingerprint.append(delimiter);
         }
      }

      return fingerprint.toString();
   }
}
