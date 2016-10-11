/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

package com.vmware.certificate;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.InputStream;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.security.KeyPair;
import java.security.PrivateKey;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.NoSuchElementException;

import org.apache.commons.codec.binary.Base64;

import com.sun.jna.Pointer;

/**
 * @author Anu Engineer
 *
 */
public class Client implements Iterable<X509Certificate> {
   public static final String BEGIN_CERT = "-----BEGIN CERTIFICATE-----\n";
   public static final String END_CERT = "\n-----END CERTIFICATE-----";

   private String ServerName;

   public enum certFilters {
      ACTIVE_CERTIFICATES, REVOKED_CERTIFICATES, EXPIRED_CERTIFICATES, ALL_CERTIFICATES

   }


   /**
    * Creates a Client class that allows you to communicate to a VMCA Server
    *
    * @param ServerName
    *           - Name of the VMCA Server
    */
   public Client(String ServerName) {
      this.setServerName(ServerName);
      setEnumFilter(certFilters.ACTIVE_CERTIFICATES);

   }

   /**
    * Creates a Certificate from a PEM encoded String
    *
    * @param certificateString
    * @return
    * @throws Exception
    */
   public static X509Certificate getCertificateFromString(
         String certificateString) throws Exception {
      InputStream is = new ByteArrayInputStream(certificateString.getBytes());
      CertificateFactory cf = CertificateFactory.getInstance("X509");
      X509Certificate c = (X509Certificate) cf.generateCertificate(is);
      return c;

   }

   private String encodeX509CertificatesToString(X509Certificate[] certs)
         throws Exception {
      if (certs == null || certs.length == 0) {
         return null;
      }

      int stringBuilderSize = certs.length * 1024; // approximate string builder
                                                   // size
      StringBuilder sb = new StringBuilder(stringBuilderSize);
      for (X509Certificate cert : certs) {
         String pem = getEncodedStringFromCertificate(cert);
         sb.append(pem);
         sb.append('\n');
      }
      if (sb.length() > 0) {
         sb.deleteCharAt(sb.length() - 1);
      }

      return sb.toString();
   }

   /**
    * returns a PEM Encoded String from a X509Certificate
    *
    * @param certificate
    * @return
    * @throws Exception
    */
   private String getEncodedStringFromCertificate(X509Certificate certificate)
         throws Exception {
      if (certificate == null) {
         throw new IllegalStateException(
               "Invalid Certificate, certificate cannot be null");
      }
      
      String encoded = new String (Base64.encodeBase64(certificate.getEncoded()));
      StringBuffer pemencode =  new StringBuffer();
      for ( int x =0; x< encoded.length(); x++)
      {

        if ((x > 0) && (x % 64 == 0)) {
            pemencode.append("\n");
            pemencode.append(encoded.charAt(x));
         } else  {
            pemencode.append(encoded.charAt(x));

         }
        }
      return  BEGIN_CERT  + pemencode.toString() + END_CERT;

   }

   /*
    * retuns a PEM encoded CSR
    *quest
    *           - A Fully populated and Signed PKCS10 Object
    * @return Base64 Encoded DER string

   public static String getEncodedStringFromPKCS10(PKCS10 certRequest)
         throws Exception {
         throw new IllegalStateException(
               "Invalid PKCS10 request, request cannot be null");
      }
      ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
      PrintStream printStream = new PrintStream(byteStream);
      certRequest.print(printStream);
      byte[] pkcsBytes = byteStream.toByteArray();
      return new String(pkcsBytes);
   }

*/
   /**
    * Gets root CA Certificate Array from VMCA
    *
    * @return X509Certificate Root CA Certificate
    * @throws Exception
    */
   public X509Certificate[] getRootCertificates() throws Exception {

	   ArrayList<X509Certificate> trustedRoots = new ArrayList<X509Certificate>();
       trustedRoots.add(Client.getCertificateFromString(VMCAAdapter
            .GetRootCertificate(ServerName)));
       return trustedRoots.toArray(new X509Certificate[trustedRoots.size()]);
   }


   /**
    * Adding new root CA Certificate chain to VMCA
    *
    * @param certificateChain
    *           Chain of certificates in case VMCA becomes a subordinate CA. Remember that the leaf certificate goes first (at index 0).
    * @param key
    * @throws Exception
    */
   public void addRootCertificate(X509Certificate[] certificateChain, PrivateKey key) throws Exception {
	   VMCAAdapter.AddRootCertificate(ServerName,encodeX509CertificatesToString(certificateChain),encodePrivateKeyToString(key));
   }

   /**
    * Gets root CA Certificate from VMCA
    *
    * @return X509Certificate Root CA Certificate
    * @throws Exception
    */


   public X509Certificate getRootCertificate() throws Exception {
	   return Client.getCertificateFromString(VMCAAdapter.GetRootCertificate(ServerName));
   }

   /*
    * Returns a Signed Certificate from the Server
    *
    * @param certificateRequest
    *           -- A PKCS 10 Certificate request
    * @param notBefore
    *           - Start date of the Certificate
    * @param notAfter
    *           - End Date of the Certificate
    * @return X509Certificate
    * @throws Exception
    */

   public X509Certificate getCertificate(String certificateRequest,
         Date notBefore, Date notAfter) throws Exception {

     long epochNotBefore = notBefore.getTime();
     long epochNotAfter = notAfter.getTime();

     epochNotBefore = epochNotBefore / 1000;
     epochNotAfter = epochNotAfter / 1000;

     return  getCertificateFromString(
                VMCAAdapter.VMCAGetSignedCertificateFromCSR(
                            ServerName,
                            certificateRequest,
                            epochNotBefore,
                            epochNotAfter));
   }

   private String encodePrivateKeyToString(PrivateKey key) throws UnsupportedEncodingException {
      if (key == null) {
         return null;
      }
      byte[] privBytes = key.getEncoded();
      String encoded = new String (Base64.encodeBase64(privBytes));
      StringBuffer pemencode =  new StringBuffer();
      for ( int x =0; x< encoded.length(); x++)
      {

        if ((x > 0) && (x % 64 == 0)) {
            pemencode.append("\n");
            pemencode.append(encoded.charAt(x));
         } else  {
            pemencode.append(encoded.charAt(x));

         }
        }
      return  "-----BEGIN PRIVATE KEY-----\n" + pemencode.toString() + "\n" + "-----END PRIVATE KEY-----";
   }

   private String getPEMEncodedKey(KeyPair Keys)
   {
      byte[] privBytes = Keys.getPrivate().getEncoded();
      String encoded = new String (Base64.encodeBase64(privBytes));
      StringBuffer pemencode =  new StringBuffer();
      for ( int x =0; x< encoded.length(); x++)
      {

        if ((x > 0) && (x % 64 == 0)) {
            pemencode.append("\n");
            pemencode.append(encoded.charAt(x));
         } else  {
            pemencode.append(encoded.charAt(x));

         }
        }
      return  "-----BEGIN PRIVATE KEY-----\n" + pemencode.toString() + "\n" + "-----END PRIVATE KEY-----";
   }

   /**
    * Returns a Signed Certificate from the Server
    *
    * @param Req
    *           -- A Request Object
    * @param Keys
    *           - A Java Key Pair Object
    * @param notBefore
    *           - Start Date for the Certificate
    * @param notAfter
    *           - End Date for the validity of the Certificate
    * @return X509Certificate that is signed by VMCA
    */
   public X509Certificate getCertificate(Request req, KeyPair Keys,
         Date notBefore, Date notAfter) throws Exception {

     long epochNotBefore = notBefore.getTime();
     long epochNotAfter = notAfter.getTime();

     epochNotBefore = epochNotBefore / 1000;
     epochNotAfter = epochNotAfter / 1000;
        String certString = VMCAAdapter.VMCAJavaGenCert(
        this.getServerName(),
        req.getName(),
        req.getCountry(),
        req.getLocality(),
        req.getState(),
        req.getOrganization(),
        req.getOrgunit(),
        req.getDnsname(),
        req.getUri(),
        req.getEmail(),
        req.getIpaddress(),
        req.getKeyusage(),
        0,
        getPEMEncodedKey(Keys),
        epochNotBefore,
        epochNotAfter);

     return getCertificateFromString(certString);

   }

   /**
    * Revokes a Certificate
    *
    * @param certificate
    * @throws Exception
    */
   public void RevokeCertificate(X509Certificate certificate) throws Exception {
      VMCAAdapter.RevokeCertificate(ServerName,
            getEncodedStringFromCertificate(certificate));
   }

   /**
    * Gets the Name of the Server we are talking to
    *
    * @return Server Name
    */
   public String getServerName() {
      return ServerName;
   }

   /**
    * Gets the VMCA Server Version
    *
    * @return Version String
    * @throws Exception
    */
   public String getServerVersion() throws Exception {
      return VMCAAdapter.getServerVersion(ServerName);
   }

   /**
    * Sets the ServerName which we are communicating
    *
    * @param serverName
    */
   public void setServerName(String serverName) {
      ServerName = serverName;
   }

   /* (non-Javadoc)
    * @see java.lang.Iterable#iterator()
    */
   @Override
   public Iterator<X509Certificate> iterator() {
      return new VMCACertIterator(ServerName, filterToInteger(getEnumFilter()));

   }

   /**
    * @return the enumFilter
    */
   public certFilters getEnumFilter() {
      return enumFilter;
   }

   /**
    * @param enumFilter
    *           the enumFilter to set
    */
   public void setEnumFilter(certFilters enumFilter) {
      this.enumFilter = enumFilter;
   }

   private certFilters enumFilter;


   /**
    * filter to Integer, since I am using ENUM java Enum to Integer is not
    * direct
    *
    * @param filter
    * @return
    */
   private static int filterToInteger(final certFilters filter) {
      switch (filter) {
      case ACTIVE_CERTIFICATES:
         return 0;
      case REVOKED_CERTIFICATES:
         return 1;
      case EXPIRED_CERTIFICATES:
         return 2;
      case ALL_CERTIFICATES:
         return 4;
      }
      return 0;
   }


   private class VMCACertIterator implements Iterator<X509Certificate> {
      private Pointer ctx;
      private String nextCert;
      private int certFilter;

      /**
       * Ctor for Iterator
       *
       * @param serverName
       * @param certFilter
       */
      public VMCACertIterator(String serverName, int certFilter) {
         try {
            this.certFilter = certFilter;
            ctx = VMCAAdapter.VMCAOpenEnumContext(serverName, this.certFilter);
         } catch (Exception e) {
            // Nothing much to do here.
         }
      }


      @Override
      public boolean hasNext() {
         try {
            if (ctx != null) {
               nextCert = VMCAAdapter.VMCAGetNextCertificate(ctx);
               if (nextCert != null) {
                  return true;
               } else {
                  VMCAAdapter.VMCACloseEnumContext(ctx);
                  ctx = null;
               }
            }
         } catch (Exception e) {
            if (ctx != null) {
               VMCAAdapter.VMCACloseEnumContext(ctx);
               ctx = null;
            }
         }
         return false;
      }

      @Override
      public X509Certificate next() {
         try {
            return Client.getCertificateFromString(nextCert);
         } catch (Exception e) {
            throw new NoSuchElementException(e.getMessage());
         }
      }

      @Override
      public void remove() {
         throw new UnsupportedOperationException(
               "To Remove or Revoke a Certificate, Please use Revoke Certificate");

      }

   }

   public void getCRL(String existingCRL, String string) throws VMCAException {
   @SuppressWarnings("unused")
	   String returnedCRL = VMCAAdapter.VMCAGetCRL(this.getServerName(), existingCRL, string);
   }

   public void PublishRoots() throws VMCAException, Exception {
      VMCAAdapter.VMCAPublishRoots(this.getServerName());
   }

}
