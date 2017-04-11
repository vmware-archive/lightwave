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

/**
 * @author Anu Engineer
 *
 */
package com.vmware.certificate;

import java.io.File;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.platform.win32.Advapi32Util;
import com.sun.jna.platform.win32.WinReg;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

public final class VMCAAdapter {

   // / enum values
   public static interface VMCA_OID {
      public static final int VMCA_OID_CN = 1;
      public static final int VMCA_OID_DC = 2;
      public static final int VMCA_OID_COUNTRY = 3;
      public static final int VMCA_OID_LOCALITY = 4;
      public static final int VMCA_OID_STATE = 5;
      public static final int VMCA_OID_ORGANIZATION = 6;
      public static final int VMCA_OID_ORG_UNIT = 7;
      public static final int VMCA_OID_DNS = 8;
      public static final int VMCA_OID_URI = 9;
      public static final int VMCA_OID_EMAIL = 10;
      public static final int VMCA_OID_IPADDRESS = 11;
   };


   // Copied from VMCA.h : Line 109
   public static interface VMCA_ENUM_RETRUN_CODE {
      public static final int VMCA_ENUM_SUCCESS = 0;
      public static final int VMCA_ENUM_END = 1;
      public static final int VMCA_ENUM_ERROR = 2;
   };


   static {
      final String VMCA_LIB64_PATH = "/opt/vmware/lib64";
      final String VMCA_VC_LIB64_PATH = "/usr/lib/vmware-vmca/lib64";
      final String LIKEWISE_LIB64_PATH = "/opt/likewise/lib64";
      final String LIB_PATH = "/usr/lib";
      final String LIB_PATH_64 = "/usr/lib64";
      final String DEV_PATH ="/mnt/hgfs/VMsrc/lotus/main/vmca/build/client/.libs";
      final String ENCODING = "utf8";
      final String propName = "jna.library.path";

      System.setProperty("jna.encoding", ENCODING);
      String propValue = System.getProperty(propName);
      StringBuilder jnalibpath =
            new StringBuilder(propValue == null ? "" : propValue);

      String paths[] =
            {VMCA_VC_LIB64_PATH, DEV_PATH, LIB_PATH_64, LIB_PATH, LIKEWISE_LIB64_PATH, VMCA_LIB64_PATH};

      for (String path : paths) {
         File libDir = new File(path);

         if (libDir.exists() && libDir.isDirectory()) {
            if (jnalibpath.length() > 0) {
               jnalibpath.append(":");
            }

            jnalibpath.append(path);
         }
      }

      // Dynamically lookup where VMCA is installed and Let Java Load the Library Correctly on Windows.
      if (Platform.isWindows()){
        String InstallPath = Advapi32Util.registryGetStringValue(WinReg.HKEY_LOCAL_MACHINE,"SOFTWARE\\VMware, Inc.\\VMware Certificate Services", "InstallPath");
        if(InstallPath.length() > 0){
           jnalibpath.append(InstallPath);
        }
      }
      propValue = jnalibpath.substring(0);

      if (!propValue.isEmpty()) {
         System.setProperty(propName, propValue);
      }
   }

   public static String getServerVersion(String ServerName) throws VMCAException {
      Pointer p = null;
      PointerByReference pServerVersion = new PointerByReference();

      try {
         int error =
               VMCA.INSTANCE.VMCAGetServerVersionA(ServerName, pServerVersion);

         THROW_IF_NEEDED(error);

         p = pServerVersion.getValue();

         if (p == null) {
            throw new IllegalStateException("Invalid server version found");
         }

         return p.getString(0);
      } finally {
         if (p != null) {
            VMCA.INSTANCE.VMCAFreeVersion(p);
         }
      }
   }

   public static void AddRootCertificate(String ServerName, String Certificate,
         String PrivateKey) throws VMCAException {

         String PassPhrase = null;
         int error =
               VMCA.INSTANCE.VMCAAddRootCertificateA(ServerName, Certificate,
                     PassPhrase, PrivateKey);
         THROW_IF_NEEDED(error);

   }

   public static String GetRootCertificate(String ServerName) throws VMCAException {
      Pointer p = null;
      PointerByReference pCertificate = new PointerByReference();
      try {
         int error =
               VMCA.INSTANCE.VMCAGetRootCACertificateA(ServerName, pCertificate);
         THROW_IF_NEEDED(error);

         p = pCertificate.getValue();

         if (p == null) {
            throw new IllegalStateException(
                  "Unable to get the Root CA Certificate");
         }

         return p.getString(0);

      } finally {
         if (p != null) {
            VMCA.INSTANCE.VMCAFreeCertificate(p);
         }
      }
   }


   public static void SetValue(int Oid, PointerByReference data, String Value)
         throws VMCAException {

         int error =
               VMCA.INSTANCE.VMCASetCertValueA(Oid, data.getPointer(), Value);
         THROW_IF_NEEDED(error);


   }

   public static PointerByReference getPKCSData() throws VMCAException {
      PointerByReference pData = new PointerByReference();
         int error = VMCA.INSTANCE.VMCAAllocatePKCS10DataA(pData);
         THROW_IF_NEEDED(error);
         return pData;

   }

   public static String GetCSR(Pointer pData, String PrivateKey)
         throws VMCAException {
      Pointer p = null;
      PointerByReference pCSR = new PointerByReference();

      try {
         int error =
               VMCA.INSTANCE.VMCACreateSigningRequestA(pData, PrivateKey, null,
                     pCSR);
         THROW_IF_NEEDED(error);
         p = pCSR.getValue();

         if (p == null) {
            throw new IllegalStateException("Invalid CSR");
         }
         return p.getString(0);

      } finally {
         if (p != null) {
            VMCA.INSTANCE.VMCAFreeCSR(p);
         }
      }
   }

   public static String GetCertificate(String ServerName, Pointer Data,
         String PrivateKey, java.util.Date notBefore, java.util.Date notAfter)
         throws VMCAException {

      Pointer p = null;
      PointerByReference pCertificate = new PointerByReference();
      try {
         int error =
               VMCA.INSTANCE.VMCAGetSignedCertificateA(ServerName, Data,
                     PrivateKey, null, notBefore.getTime(), notAfter.getTime(),
                     pCertificate);
         THROW_IF_NEEDED(error);
         p = pCertificate.getValue();
         if (p == null) {
            throw new IllegalStateException("Expected Certificate not Found");
         }
         return p.getString(0);
      } finally {
         if (p != null) {
            VMCA.INSTANCE.VMCAFreeCertificate(p);
         }
      }

   }

   public static String GetSelfSignedCertificate(Pointer Data,
         String PrivateKey, java.util.Date notBefore, java.util.Date notAfter)
         throws VMCAException {

      Pointer p = null;
      PointerByReference pCertificate = new PointerByReference();
      try {
         int error =
               VMCA.INSTANCE.VMCACreateSelfSignedCertificateA(Data, PrivateKey,
                     null, notBefore.getTime(), notAfter.getTime(),
                     pCertificate);
         THROW_IF_NEEDED(error);
         p = pCertificate.getValue();
         if (p == null) {
            throw new IllegalStateException("Expected Certificate not Found");
         }
         return p.getString(0);
      } finally {
         if (p != null) {
            VMCA.INSTANCE.VMCAFreeCertificate(p);
         }
      }
   }

   public static void RevokeCertificate(String ServerName, String Certificate)
         throws VMCAException {
         int error =
               VMCA.INSTANCE.VMCARevokeCertificateA(ServerName, Certificate);
         THROW_IF_NEEDED(error);

   }

   public static boolean IsCACert(String Certificate) {
         int error = VMCA.INSTANCE.VMCAValidateCACertificateA(Certificate);
         if (error == 0) {
            return true;
         }
         return false;

   }

   public static void FreeData(Pointer p) {
      if (p != null) {
         VMCA.INSTANCE.VMCAFreePKCS10DataA(p);
      }

   }

   private static void THROW_IF_NEEDED(int error)
         throws VMCAException {
         String err = null;
         if ( error != 0) {
            try {
             err = VMCAAdapter.VMCAGetShortError(error);
            } catch(VMCAException ex) {
               err = "Unable to get error string from VMCA, Please use error code";
            }
            VMCAException exp = new VMCAException (err);
            exp.setErrorCode(error);
            throw exp;
         }
   }


   public static String VMCAJavaGenCert(String ServerName, String Name,
         String Country, String Locality, String State, String Organization,
         String OrgUnit, String DnsName, String URIName, String Email,
         String IPAddress, int KeyUsageConstraints, int isSelfSigned,
         String PrivateKey, long tmNotBefore, long tmNotAfter) throws VMCAException {


      Pointer p = null;
      PointerByReference pCertificate = new PointerByReference();
      try {

         int error =
               VMCA.INSTANCE.VMCAJavaGenCertA(ServerName, Name, Country,
                     Locality, State, Organization, OrgUnit, DnsName, URIName,
                     Email, IPAddress, KeyUsageConstraints, isSelfSigned,
                     PrivateKey, tmNotBefore, tmNotAfter, pCertificate);

         THROW_IF_NEEDED(error);
         p = pCertificate.getValue();
         if (p == null) {
            throw new IllegalStateException("Expected Certificate not Found");
         }
         return p.getString(0);

      } finally {
         if (p != null) {
            VMCA.INSTANCE.VMCAFreeCertificate(p);
         }

      }
   }

   public static String VMCAGetSignedCertificateFromCSR(String ServerName,
         String CSR, long tmNotBefore, long tmNotAfter) throws VMCAException {

      Pointer p = null;
      PointerByReference pCertificate = new PointerByReference();
      try {
         int error =
               VMCA.INSTANCE.VMCAGetSignedCertificateFromCSRA(ServerName, CSR,
                     tmNotBefore, tmNotAfter, pCertificate);
         THROW_IF_NEEDED(error);
         p = pCertificate.getValue();
         if (p == null) {
            throw new IllegalStateException("Expected Certificate not Found");
         }
         return p.getString(0);

      } finally {
         if (p != null) {
            VMCA.INSTANCE.VMCAFreeCertificate(p);
         }

      }

   }

   public static Pointer VMCAOpenEnumContext(String ServerName, int certFilter)
         throws VMCAException {
      Pointer p = null;
      PointerByReference pContext = new PointerByReference();
         int error =
               VMCA.INSTANCE.VMCAOpenEnumContextA(ServerName, certFilter,
                     pContext);

         THROW_IF_NEEDED(error);

         p = pContext.getValue();
         if (p == null) {
            throw new IllegalStateException("Expected context not Found");
         }
         return p;
   }


   public static String VMCAGetNextCertificate(Pointer ctx) throws VMCAException {
      IntByReference pCurrentIndex = new IntByReference();
      IntByReference pEnumStatus = new IntByReference();
      PointerByReference ppCertificate = new PointerByReference();
      Pointer pCert = null;
      try {
         int error =
               VMCA.INSTANCE.VMCAGetNextCertificate(ctx, ppCertificate,
                     pCurrentIndex, pEnumStatus);
         THROW_IF_NEEDED(error);

         if (pEnumStatus.getValue() == VMCA_ENUM_RETRUN_CODE.VMCA_ENUM_ERROR) {
            throw new RuntimeException(
                  "Certificate Enum Returned - VMCA_ENUM_ERROR");
         }
         if (pEnumStatus.getValue() == VMCA_ENUM_RETRUN_CODE.VMCA_ENUM_END) {
            return null; // Enumeration is done
         }

         if (pEnumStatus.getValue() == VMCA_ENUM_RETRUN_CODE.VMCA_ENUM_SUCCESS) {
            pCert = ppCertificate.getValue();
            if (pCert == null) {
               throw new IllegalStateException("Expected Certificate not Found");
            }
            String temp = pCert.getString(0);
            return temp; // return the certificate
         }
      } finally {
         if (pCert != null) {
            VMCA.INSTANCE.VMCAFreeCertificate(pCert);
         }
      }
      return null;
   }

   public static String VMCAGetShortError(int dwError) throws VMCAException
   {

      PointerByReference ppMsg = new PointerByReference();
      Pointer pMsg = null;
      try {
         int error = VMCA.INSTANCE.VMCAGetShortError(dwError, ppMsg);
         if ( error != 0) {
            VMCAException exp = new VMCAException ("Unable to get Error String from VMCA");
            exp.setErrorCode(error);
            throw exp;
         }
         pMsg= ppMsg.getValue();

         if (pMsg == null) {
            throw new IllegalStateException("Expected Error String not Found");
         }
         return pMsg.getString(0);

      } finally {
         if(pMsg!= null){
            VMCA.INSTANCE.VMCAFreeVersion(pMsg);
         }
      }

   }


   public static String VMCAGetErrorString(int dwError) throws VMCAException
   {

      PointerByReference ppMsg = new PointerByReference();
      Pointer pMsg = null;
      try {
         int error = VMCA.INSTANCE.VMCAGetErrorString(dwError, ppMsg);
         if ( error != 0) {
            VMCAException exp = new VMCAException ("Unable to get Error String from VMCA");
            exp.setErrorCode(error);
            throw exp;
         }

         pMsg= ppMsg.getValue();

         if (pMsg == null) {
            throw new IllegalStateException("Expected Error String not Found");
         }
         return pMsg.getString(0);

      } finally {
         if(pMsg!= null){
            VMCA.INSTANCE.VMCAFreeVersion(pMsg);
         }
      }

   }


   public static void VMCACloseEnumContext(Pointer ctx) {
      VMCA.INSTANCE.VMCACloseEnumContext(ctx);
   }


   public static String VMCAGetCRL(String ServerName,
            String existingCRL, String newCRL) throws VMCAException {

         try {
            int error =
                  VMCA.INSTANCE.VMCAGetCRLA(ServerName, existingCRL,
                        newCRL);
            THROW_IF_NEEDED(error);

         } finally {

         }
         return newCRL;
      }


   public static void VMCAPublishRoots(String ServerName) throws VMCAException
   {
      int error = VMCA.INSTANCE.VMCAPublishRootCertsA(ServerName);
      THROW_IF_NEEDED(error);
      return;
   }



   public interface VMCA extends Library {

      VMCA INSTANCE = (VMCA) Native.loadLibrary("vmcaclient", VMCA.class);

      int VMCAGetServerVersionA(String pszServerName,
            PointerByReference pServerVersion);

      int VMCAFreeVersion(Pointer p);

      int VMCAAddRootCertificateA(String pszServerName,
            String pszRootCertificate, String pszPassPhrase,
            String pszPrivateKey);

      int VMCAGetRootCACertificateA(String pszServerName,
            PointerByReference pCertificate);

      void VMCAFreeCertificate(Pointer pCertificate);

      int VMCACreatePrivateKeyA(String pszPassPhrase, Integer KeyLength,
            PointerByReference pszPrivateKey, PointerByReference pszPublicKey);

      void VMCAFreeKey(Pointer p);

      int VMCAAllocatePKCS10DataA(PointerByReference data);

      int VMCASetCertValueA(Integer Oid, Pointer data, String pszValue);

      int VMCASetKeyUsageConstraintsA(Pointer data, Integer KeyUsage);

      int VMCACreateSigningRequestA(Pointer data, String pszPrivateKey,
            String pszPassPhrase, PointerByReference pCsr);

      int VMCAFreeCSR(Pointer p);

      int VMCAFreePKCS10DataA(Pointer p);

      int VMCAGetSignedCertificateA(String ServerName, Pointer data,
            String PrivateKey, String PassPhrase, long tmNotBefore,
            long tmNotAfter, PointerByReference pCert);

      int VMCACreateSelfSignedCertificateA(Pointer data, String PrivateKey,
            String PassPhrase, long tmNotBefore, long tmNotAfter,
            PointerByReference pCert);

      int VMCARevokeCertificateA(String ServerName, String Certificate);

      int VMCAValidateCACertificateA(String pszCertificate);

      int VMCAJavaGenCertA(String ServerName, String Name, String Country,
            String Locality, String State, String Organization, String OrgUnit,
            String DnsName, String URIName, String Email, String IPAddress,
            int KeyUsageConstraints, int isSelfSigned, String PrivateKey,
            long tmNotBefore, long tmNotAfter, PointerByReference pCert);

      int VMCAGetSignedCertificateFromCSRA(String ServerName, String CSR,
            long tmNotBefore, long tmNotAfter, PointerByReference pCert);

      int VMCAOpenEnumContextA(String ServerName, int certFilter,
            PointerByReference pContext);

      int VMCAGetNextCertificate(Pointer ctx, PointerByReference pCert,
            IntByReference pCurrentIndex, IntByReference enumStatus);


      int VMCACloseEnumContext(Pointer ctx);

      int  VMCAGetShortError( int errcode, PointerByReference pMsg);
      int  VMCAGetErrorString( int errcode, PointerByReference pMsg);

      int VMCAGetCRLA(String ServerName, String existingCRLPAth, String NewCRLPath);
      int VMCAPublishRootCertsA(String ServerName);

   }
}
