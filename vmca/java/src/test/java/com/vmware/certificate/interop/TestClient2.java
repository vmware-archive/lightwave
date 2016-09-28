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

package com.vmware.certificate.interop;
/**
 * We need to have this in tests since all the certificates returned by
 * VMCA is X509Certificate class , which is part of Java Security cert.
 */
import java.io.File;
import java.security.KeyPair;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.GregorianCalendar;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import com.vmware.certificate.Client;
import com.vmware.certificate.Request;
import com.vmware.certificate.VMCAException;

/**
 * Import VMCA interface.
 *
 */

public class TestClient2 {

   private VMCAClient vmcaClient = null;
   private _username = "Administrator";
   private _password = "Admin!23";
   private _domain = "vsphere.local";
   private _servername = "localhost";

   /**
    * @throws java.lang.Exception
    */
   @Before
   public void setUp() throws Exception {
      vmcaClient = new VMCAClient(_username, _domain, _password, _servername);
   }

   /**
    * @throws java.lang.Exception
    */
   @After
   public void tearDown() throws Exception {
   }

   /**
    * This test returns the Server Version String by talking to the Server
    *
    * @throws Exception
    */
   @Test
   public void testGetServerVersion() throws Exception {
    try {
      String serverVersion = vmcaClient.getServerVersion();
        System.out.println(serverVersion);
        Assert.assertNotNull(serverVersion);
        Assert.assertTrue(serverVersion.length() != 0);
      } catch(VMCAException vx) {
      System.out.println(vx.getErrorCode());
    }
   }

   /**
    * This function reads the Root Certificate from VMCA and verifies that
    * Subject DN is accessible using Java APIs.
    *
    * @throws Exception
    */
   @Test
   public void testGetRootCertificate() throws Exception {
      X509Certificate rootCert = vmcaClient.getRootCertificate();
      Assert.assertNotNull(rootCert);
      String subjectDN = rootCert.getSubjectDN().getName();
      Assert.assertNotNull(subjectDN);
      System.out.println(subjectDN);
   }

   /**
    * This function reads the Root Certificate and checks to make sure that Root
    * Certificate is valid, ie. Not Expired in relation to the Current Date.
    *
    * @throws Exception
    */
   @Test
   public void testRootisValidToday() throws Exception {
      Calendar today = new GregorianCalendar();
      X509Certificate rootCerts = vmcaClient.getRootCertificate();
      Assert.assertNotNull(rootCerts);
      // verify that rootCert is valid "now", will throw if not so.
      rootCerts.checkValidity(today.getTime());
      System.out.println("Root Certificate is valid today");
   }

   @Test
   public void testKeyGeneration() throws Exception {
      Request req = new Request();
      KeyPair Keys = req.createKeyPair(2048);
      Assert.assertNotNull(Keys.getPrivate());
      Assert.assertNotNull(Keys.getPublic());
      System.out.println("Private Key Algoritm is : "
            + Keys.getPrivate().getAlgorithm());
   }

   @Test
   public void testPKCS10Generation() throws Exception {
     /* Request req = new Request();
      KeyPair Keys = req.createKeyPair(2048);
      req.setName("JavaPKCS");
      req.setCountry("US");
      req.setOrganization("VMware");
      PKCS10 pkcs = req.getCertificateSigningRequest(Keys);
      X500Name subjectName = pkcs.getSubjectName();
      PublicKey pubKey = pkcs.getSubjectPublicKeyInfo();
      Assert.assertNotNull(subjectName);
      Assert.assertNotNull(pubKey);
      Assert.assertEquals("US", subjectName.getCountry());
      System.out.println(subjectName.getCountry());
      Assert.assertEquals("JavaPKCS", subjectName.getCommonName());
      System.out.println(subjectName.getCommonName());
      System.out.println(subjectName.getOrganization());
      String csrPem = Client.getEncodedStringFromPKCS10(pkcs);
      Assert.assertNotNull(csrPem);
      System.out.println(csrPem);
      */
   }

   /**
    * This test just gets 128 certificates from VMCA.
    *
    * @throws Exception
    */
   @Test
   public void testGetCertificateFromVMCA() throws Exception {
    Request req = new Request();
      KeyPair Keys = req.createKeyPair(2048);
      for (int x = 0; x < 128; x++) {
         Calendar notBefore = Calendar.getInstance();
         Calendar notAfter = Calendar.getInstance();

         req.setName("JavaPKCS" + Integer.toString(x));
         req.setCountry("US");
         req.setOrganization("VMware");
         notAfter.add(Calendar.YEAR, 1);
         X509Certificate cert =
               vmcaClient.getCertificate(req, Keys, notBefore.getTime(),
                     notAfter.getTime());
         Assert.assertNotNull(cert);
         String subjectDN = cert.getSubjectDN().getName();
         System.out.println(subjectDN);
      }
   }

   @Test
   public void testRevokeCertificateTest() throws Exception {
      Request req = new Request();
        KeyPair Keys = req.createKeyPair(2048);
         Calendar notBefore = Calendar.getInstance();
         Calendar notAfter = Calendar.getInstance();

         req.setName("JavaPKCS");
         req.setCountry("US");
         req.setOrganization("VMware");
         notAfter.add(Calendar.YEAR, 1);
         X509Certificate cert =
               vmcaClient.getCertificate(req, Keys, notBefore.getTime(),
                     notAfter.getTime());
         Assert.assertNotNull(cert);
         String subjectDN = cert.getSubjectDN().getName();
         System.out.println(subjectDN);
         vmcaClient.RevokeCertificate(cert);
     }

   @Test
   public void testEnumCerts() throws Exception {
      int x = 0;
      for (X509Certificate cert : vmcaClient) {
         if (++x == 1024)
            break; // This takes too long as the Data base size increases
         String subjectDN = cert.getSubjectDN().getName();
         System.out.println(subjectDN);
         Assert.assertNotNull(subjectDN);
      }

   }

   @Test
   public void testRevokeOneCert() throws Exception {
      vmcaClient.setEnumFilter(Client.certFilters.ACTIVE_CERTIFICATES);
      for (X509Certificate cert : vmcaClient) {
         vmcaClient.RevokeCertificate(cert);
         break;
      }
   }

   @Test
   public void testEnumRevokedCert() throws Exception {

      int x = 0;
      vmcaClient.setEnumFilter(Client.certFilters.REVOKED_CERTIFICATES);
      for (X509Certificate cert : vmcaClient) {
         x++;
         System.out.print("Revoked Cert Serial : ");
         System.out.println(cert.getSerialNumber());
      }
      System.out.print("Revoke Cert Count : ");
      System.out.println(x);
   }


   @Test
   public void TestCSRToCert() throws Exception {
     String csr =
         "-----BEGIN CERTIFICATE REQUEST-----\n"
         +"MIICvDCCAaQCAQAweTELMAkGA1UEBhMCVVMxEzARBgNVBAgTCkNhbGlmb3JuaWEx\n"
         +"EjAQBgNVBAcTCVBhbG8gQWx0bzEPMA0GA1UEChMGVk13YXJlMRswGQYDVQQLExJT\n"
         +"dG9yYWdlIE1hbmFnZW1lbnQxEzARBgNVBAMTClZNd2FyZSBpVlAwggEiMA0GCSqG\n"
         +"SIb3DQEBAQUAA4IBDwAwggEKAoIBAQDKbCvYpxH8P9bVJAyBQmWJ5/FIdh1QAvn2\n"
         +"tsTNzqmtcz7w3NXSMXGgxoPDyvyT6PIHl0u1DsqPXoC6c4nWkNB3IvywDDAI2B8o\n"
         +"lNt6AcN56YJ/PI9rBnOVrabXSI1L0FJSU3fhAhfQ0aCn3PFWArR/zM6BXJx4835k\n"
         +"8Zq8cGdznN8DHdHwOzP74CINSzmXhzf7b84vjL8vruBIkCNRjqlV2DotZ8EwB/L4\n"
         +"LSbi2EAVQe6nqej3f3mzUIM5xn7i+/U46vs/ADUBCF4ECmxLYjaULXpaoOW8SVMb\n"
         +"kChPhep3EeQ9mHHn7p6xz/yMIIc+pVOtz5Zw1SDV+ctMULv4dkn9AgMBAAEwDQYJ\n"
         +"KoZIhvcNAQELBQADggEBAJYrZGqG08F+iqqQ81gJgs/yl823AoUuBx2YmtfaqUSt\n"
         +"yre9sqGMLrWgCpqHs035uuI1a5lR9GcLPMNWCnHXJvevnZzmUBrYIl7I/FQ662qB\n"
         +"KK5dxYa4r2ujmUuuAKlnfNmXtJf9g/z7IX2klvKA2w0cEx6WpcMfjSaEUTSKFVoo\n"
         +"f0E3FDXrJE0Ncti9P9yNZRVoMSHqs1F2ffM5I/7iNE3UGujOuBerIIUm/Ffg2H7V\n"
         +"QOIq/pNfk3FepxeSsc2ODUekVdiCmRQEHv1obEA59E5RgvBq2Fd2lpe72hFasup5\n"
         +"oQiFZFqrvo5+W3KyxTiwr45WkfPhOTh4kOjo9z9rRO8=\n"
         +"-----END CERTIFICATE REQUEST-----\n";
    try {
     Calendar notBefore = Calendar.getInstance();
       Calendar notAfter = Calendar.getInstance();

       notAfter.add(Calendar.YEAR, 1);

       System.out.println("Requested Not Before"  + notBefore);
       System.out.println("Requested Not After"  +  notAfter);


       X509Certificate cert =
             vmcaClient.getCertificate(csr, notBefore.getTime(),
                   notAfter.getTime());
       Assert.assertNotNull(cert);
       String subjectDN = cert.getSubjectDN().getName();
       System.out.println(subjectDN);
       System.out.println(cert);

       Assert.assertTrue(cert.getNotBefore().equals(notBefore));
       Assert.assertTrue(cert.getNotAfter().equals(notAfter));
    } catch (VMCAException vx)
    {
      System.out.println(vx.getErrorCode());
    }
   }
 

   @Test
   public void TestGetCRL() throws Exception {
   File f = new File("Y:\\TestEnlist\\lotus\\main\\vmca\\x64\\Release\\vmcajava.crl");
   vmcaClient.getCRL(null, f.getPath());
   Assert.assertTrue(f.exists());
   }


}
