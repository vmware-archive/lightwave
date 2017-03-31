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

public final class VMCAAdapter2 {

   // / enum values
   public static interface VMCA_OID {
      public static final int VMCA_OID_CN = 1;
      public static final int VMCA_OID_COUNTRY = 2;
      public static final int VMCA_OID_LOCALITY = 3;
      public static final int VMCA_OID_STATE = 4;
      public static final int VMCA_OID_ORGANIZATION = 5;
      public static final int VMCA_OID_ORG_UNIT = 6;
      public static final int VMCA_OID_DNS = 7;
      public static final int VMCA_OID_URI = 8;
      public static final int VMCA_OID_EMAIL = 9;
      public static final int VMCA_OID_IPADDRESS = 10;
   };

   // Copied from VMCA.h : Line 109
   public static interface VMCA_ENUM_RETURN_CODE {
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
      final String ENCODING = "utf8";
      final String propName = "jna.library.path";

      System.setProperty("jna.encoding", ENCODING);
      String propValue = System.getProperty(propName);
      StringBuilder jnalibpath =
            new StringBuilder(propValue == null ? "" : propValue);

      String paths[] =
            { VMCA_VC_LIB64_PATH, LIB_PATH_64, LIB_PATH, LIKEWISE_LIB64_PATH, VMCA_LIB64_PATH};

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
      if (Platform.isWindows()) {

        String InstallPath =
                Advapi32Util.registryGetStringValue(
                        WinReg.HKEY_LOCAL_MACHINE,
                        "SOFTWARE\\VMware, Inc.\\VMware Certificate Services",
                        "InstallPath");

        if (InstallPath.length() > 0) {
           jnalibpath.append(InstallPath);
        }
      }
      propValue = jnalibpath.substring(0);

      if (!propValue.isEmpty()) {
         System.setProperty(propName, propValue);
      }
   }

   public static VMCAServerContext
   getServerContext(
      String pszNetworkAddress,
      String pszUserName,
      String pszDomain,
      String pszPassword
      ) throws VMCAException
   {
       PointerByReference ppContext = new PointerByReference();

       int flags = 0;

       int error = VMCA.INSTANCE.VMCAOpenServerA(
                           pszNetworkAddress,
                           pszUserName,
                           pszDomain,
                           pszPassword,
                           flags,
                           Pointer.NULL, /* reserved */
                           ppContext);
       THROW_IF_NEEDED(error);

       return new VMCAServerContext(pszNetworkAddress, ppContext.getValue());
   }

   public static String
   getServerVersion(VMCAServerContext context) throws VMCAException
   {
       Pointer pServerVersion = Pointer.NULL;
       PointerByReference ppServerVersion = new PointerByReference();

       try
       {
               int error = VMCA.INSTANCE.VMCAGetServerVersionHA(
                                           context.getContext(),
                                           context.getNetworkAddress(),
                                           ppServerVersion);
             THROW_IF_NEEDED(error);

             pServerVersion = ppServerVersion.getValue();

             return pServerVersion.getString(0);

       }
       finally
       {
            if (pServerVersion != Pointer.NULL)
            {
                VMCA.INSTANCE.VMCAFreeVersion(pServerVersion);
            }
       }
   }

   public static void
   AddRootCertificate(
       VMCAServerContext context,
       String pszCertificate,
       String pszPrivateKey
       ) throws VMCAException
   {
         String pszPassPhrase = null;

         int error = VMCA.INSTANCE.VMCAAddRootCertificateHA(
                                         context.getContext(),
                                         context.getNetworkAddress(),
                                         pszCertificate,
                                         pszPassPhrase,
                                         pszPrivateKey);
         THROW_IF_NEEDED(error);
   }

   public static String
   GetRootCertificate(VMCAServerContext context) throws VMCAException
   {
      Pointer pCertificate = Pointer.NULL;

      PointerByReference ppCertificate = new PointerByReference();

      try
      {
         int error = VMCA.INSTANCE.VMCAGetRootCACertificateHA(
                                           context.getContext(),
                                           context.getNetworkAddress(),
                                           ppCertificate);
         THROW_IF_NEEDED(error);

         pCertificate = ppCertificate.getValue();

         return pCertificate.getString(0);

      }
      finally
      {
         if (pCertificate != null)
         {
            VMCA.INSTANCE.VMCAFreeCertificate(pCertificate);
         }
      }
   }

   public static void
   SetValue(
      int Oid,
      PointerByReference data,
      String Value
      ) throws VMCAException
   {
         int error = VMCA.INSTANCE.VMCASetCertValueA(
                                         Oid,
                                         data.getPointer(),
                                         Value);
         THROW_IF_NEEDED(error);
   }

   public static String
   GetCSR(Pointer pData, String privateKey) throws VMCAException
   {
      Pointer pCSR = Pointer.NULL;

      PointerByReference ppCSR = new PointerByReference();

      try
      {
         int error = VMCA.INSTANCE.VMCACreateSigningRequestA(
                                         pData,
                                         privateKey,
                                         null,
                                         ppCSR);
         THROW_IF_NEEDED(error);

         pCSR = ppCSR.getValue();

         return pCSR.getString(0);
      }
      finally
      {
         if (pCSR != Pointer.NULL)
         {
            VMCA.INSTANCE.VMCAFreeCSR(pCSR);
         }
      }
   }

   public static String
   GetCertificate(
      VMCAServerContext context,
      Pointer           data,
      String            privateKey,
      java.util.Date    notBefore,
      java.util.Date    notAfter
      ) throws VMCAException
   {
      Pointer pCertificate = Pointer.NULL;

      PointerByReference ppCertificate = new PointerByReference();

      try
      {
         int error = VMCA.INSTANCE.VMCAGetSignedCertificateHA(
                                         context.getContext(),
                                         context.getNetworkAddress(),
                                         data,
                                         privateKey,
                                         null,
                                         notBefore.getTime(),
                                         notAfter.getTime(),
                                         ppCertificate);
         THROW_IF_NEEDED(error);

         pCertificate = ppCertificate.getValue();

         return pCertificate.getString(0);
      }
      finally
      {
         if (pCertificate != Pointer.NULL)
         {
            VMCA.INSTANCE.VMCAFreeCertificate(pCertificate);
         }
      }
   }

   public static String
   GetSelfSignedCertificate(
      Pointer        data,
      String         privateKey,
      java.util.Date notBefore,
      java.util.Date notAfter
      ) throws VMCAException
   {
      Pointer pCertificate = Pointer.NULL;

      PointerByReference ppCertificate = new PointerByReference();

      try
      {
         int error = VMCA.INSTANCE.VMCACreateSelfSignedCertificateA(
                                         data,
                                         privateKey,
                                         null,
                                         notBefore.getTime(),
                                         notAfter.getTime(),
                                         ppCertificate);
         THROW_IF_NEEDED(error);

         pCertificate = ppCertificate.getValue();

         return pCertificate.getString(0);
      }
      finally
      {
         if (pCertificate != Pointer.NULL)
         {
            VMCA.INSTANCE.VMCAFreeCertificate(pCertificate);
         }
      }
   }

   public static void
   RevokeCertificate(
      VMCAServerContext context,
      String pszCertificate
      ) throws VMCAException
   {
      int error = VMCA.INSTANCE.VMCARevokeCertificateHA(
                                      context.getContext(),
                                      context.getNetworkAddress(),
                                      pszCertificate);
      THROW_IF_NEEDED(error);
   }

   public static boolean IsCACert(String pszCertificate)
   {
         int error = VMCA.INSTANCE.VMCAValidateCACertificate(pszCertificate);
         return (error == 0);
   }

   public static void FreeData(Pointer p)
   {
      if (p != Pointer.NULL)
      {
         VMCA.INSTANCE.VMCAFreePKCS10DataA(p);
      }
   }

   private static void THROW_IF_NEEDED(int error) throws VMCAException
   {
         String err = null;
         if ( error != 0)
         {
            try
            {
                err = VMCAAdapter.VMCAGetShortError(error);
            }
            catch(VMCAException ex)
            {
               err = "Unable to get error string from VMCA, Please use error code";
            }
            VMCAException exp = new VMCAException (err);
            exp.setErrorCode(error);
            throw exp;
         }
   }

   public static String VMCAJavaGenCert(VMCAServerContext context, String Name,
         String Country, String Locality, String State, String Organization,
         String OrgUnit, String DnsName, String URIName, String Email,
         String IPAddress, int KeyUsageConstraints, int isSelfSigned,
         String PrivateKey, long tmNotBefore, long tmNotAfter) throws VMCAException {


      Pointer p = null;
      PointerByReference pCertificate = new PointerByReference();
      try {

         int error =
               VMCA.INSTANCE.VMCAJavaGenCertHA(context.getContext(), context.getNetworkAddress(), Name, Country,
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

   public static String
   VMCAGetSignedCertificateFromCSR(
      VMCAServerContext context,
      String pszCSR,
      long tmNotBefore,
      long tmNotAfter
      ) throws VMCAException
   {
      Pointer pCertificate = null;
      PointerByReference ppCertificate = new PointerByReference();
      try
      {
         int error = VMCA.INSTANCE.VMCAGetSignedCertificateFromCSRHA(
                                         context.getContext(),
                                         context.getNetworkAddress(),
                                         pszCSR,
                                         tmNotBefore,
                                         tmNotAfter,
                                         ppCertificate);
         THROW_IF_NEEDED(error);

         pCertificate = ppCertificate.getValue();

         return pCertificate.getString(0);
      }
      finally
      {
         if (pCertificate != null)
         {
            VMCA.INSTANCE.VMCAFreeCertificate(pCertificate);
         }
      }
   }

   public static String
   VMCAGetSignedCertificateForHost(
      VMCAServerContext context,
      String pszHostName,
      String pszIpAddress,
      String pszCSR,
      long tmNotBefore,
      long tmNotAfter
      ) throws VMCAException
   {
      Pointer pCertificate = null;
      PointerByReference ppCertificate = new PointerByReference();
      try
      {
         int error = VMCA.INSTANCE.VMCAGetSignedCertificateForHostA(
                                         context.getContext(),
                                         pszHostName,
                                         pszIpAddress,
                                         pszCSR,
                                         tmNotBefore,
                                         tmNotAfter,
                                         ppCertificate);
         THROW_IF_NEEDED(error);

         pCertificate = ppCertificate.getValue();

         return pCertificate.getString(0);
      }
      finally
      {
         if (pCertificate != null)
         {
            VMCA.INSTANCE.VMCAFreeCertificate(pCertificate);
         }
      }
   }

   public static Pointer
   VMCAOpenEnumContext(
      VMCAServerContext context,
      int certFilter
      ) throws VMCAException
   {
      PointerByReference ppContext = new PointerByReference();

      int error = VMCA.INSTANCE.VMCAOpenEnumContextHA(
                                      context.getContext(),
                                      context.getNetworkAddress(),
                                      certFilter,
                                      ppContext);
      THROW_IF_NEEDED(error);

      Pointer pEnumContext = ppContext.getValue();

      return pEnumContext;
   }

   public static String VMCAGetNextCertificate(Pointer ctx) throws VMCAException
   {
      IntByReference pCurrentIndex = new IntByReference();
      IntByReference pEnumStatus = new IntByReference();
      PointerByReference ppCertificate = new PointerByReference();
      Pointer pCert = null;
      try
      {
         int error = VMCA.INSTANCE.VMCAGetNextCertificate(
                                         ctx,
                                         ppCertificate,
                                         pCurrentIndex,
                                         pEnumStatus);
         THROW_IF_NEEDED(error);

         if (pEnumStatus.getValue() == VMCA_ENUM_RETURN_CODE.VMCA_ENUM_ERROR)
         {
            throw new RuntimeException(
                  "Certificate Enum Returned - VMCA_ENUM_ERROR");
         }
         if (pEnumStatus.getValue() == VMCA_ENUM_RETURN_CODE.VMCA_ENUM_END)
         {
            return null; // Enumeration is done
         }

         if (pEnumStatus.getValue() == VMCA_ENUM_RETURN_CODE.VMCA_ENUM_SUCCESS)
         {
            pCert = ppCertificate.getValue();

            String pszCertificate = pCert.getString(0);

            return pszCertificate;
         }
      }
      finally
      {
         if (pCert != null)
         {
            VMCA.INSTANCE.VMCAFreeCertificate(pCert);
         }
      }

      return null;
   }

   public static String VMCAGetShortError(int dwError) throws VMCAException
   {
      PointerByReference ppMsg = new PointerByReference();
      Pointer pMsg = null;
      try
      {
         int error = VMCA.INSTANCE.VMCAGetShortError(dwError, ppMsg);

         if ( error != 0)
         {
            VMCAException exp = new VMCAException ("Unable to get Error String from VMCA");
            exp.setErrorCode(error);
            throw exp;
         }

         pMsg= ppMsg.getValue();

         assert (pMsg != null);

         return pMsg.getString(0);
      }
      finally
      {
         if (pMsg!= null)
         {
            VMCA.INSTANCE.VMCAFreeVersion(pMsg);
         }
      }
   }


   public static String VMCAGetErrorString(int dwError) throws VMCAException
   {
      PointerByReference ppMsg = new PointerByReference();
      Pointer pMsg = null;

      try
      {
         int error = VMCA.INSTANCE.VMCAGetErrorString(dwError, ppMsg);

         if ( error != 0)
         {
            VMCAException exp = new VMCAException ("Unable to get Error String from VMCA");
            exp.setErrorCode(error);
            throw exp;
         }

         pMsg= ppMsg.getValue();

         assert (pMsg != null);

         return pMsg.getString(0);
      }
      finally
      {
         if (pMsg != null)
         {
            VMCA.INSTANCE.VMCAFreeVersion(pMsg);
         }
      }
   }

   public static void VMCACloseEnumContext(Pointer ctx) {
       if (ctx != Pointer.NULL)
       {
           VMCA.INSTANCE.VMCACloseEnumContext(ctx);
       }
   }

   public static String
   VMCAGetCRL(
       VMCAServerContext context,
       String existingCRL,
       String newCRL
       ) throws VMCAException
   {
       int error = VMCA.INSTANCE.VMCAGetCRLHA(
                                          context.getContext(),
                                          context.getNetworkAddress(),
                                          existingCRL,
                                          newCRL);
       THROW_IF_NEEDED(error);

       return newCRL;
   }

   public static void
   VMCAPublishRoots(VMCAServerContext context) throws VMCAException
   {
      int error = VMCA.INSTANCE.VMCAPublishRootCertsHA(
                                      context.getContext(),
                                      context.getNetworkAddress());
      THROW_IF_NEEDED(error);
   }

   public interface VMCA extends Library {

      VMCA INSTANCE = (VMCA) Native.loadLibrary("vmcaclient", VMCA.class);

      int
      VMCAOpenServerA(
          String pszNetworkAddress,
          String pszUserName,
          String pszDomain,
          String pszPassword,
          int     dwFlags,
          Pointer pReserved,
          PointerByReference ppServerContext
          );

      int
      VMCAGetServerVersionHA(
          Pointer pBinding,
          String pszServerName,
          PointerByReference ppszServerVersionString
      );

      int VMCAFreeVersion(Pointer p);

      int
      VMCAGetRootCACertificateHA(
          Pointer pBinding,
          String pszServerName,
          PointerByReference ppCertificate
      );

      int
      VMCAAddRootCertificateHA(
          Pointer pBinding,
          String pwszServerName,
          String  pszRootCertificate,
          String pszPassPhrase,
          String  pszPrivateKey
      );

      int
      VMCACreateSelfSignedCertificateA(
          Pointer data,
          String PrivateKey,
          String PassPhrase,
          long tmNotBefore,
          long tmNotAfter,
          PointerByReference ppCertificate
          );

      int
      VMCACreatePrivateKeyA(
          String            pwszPassPhrase,
          Integer            keyLength,
          PointerByReference pszPrivateKey,
          PointerByReference pszPublicKey
          );

      int VMCAAllocatePKCS10DataA(PointerByReference ppCertRequest);

      int
      VMCAGetSignedCertificateHA(
          Pointer pBinding,
          String pwszServerName,
          Pointer pCertRequest,
          String  pPrivateKey,
          String pszPassPhrase,
          long tmNotBefore,
          long tmNotAfter,
          PointerByReference ppCertificate
          );

      int
      VMCAGetSignedCertificateFromCSRHA(
          Pointer pBinding,
          String  pszServerName,
          String  pszCertRequest,
          long    tmNotBefore,
          long    tmNotAfter,
          PointerByReference ppCertificate
      );

      int
      VMCAGetSignedCertificateForHostA(
          Pointer pBinding,
          String  pszHostName,
          String  pszIpAddress,
          String  pszCertRequest,
          long    tmNotBefore,
          long    tmNotAfter,
          PointerByReference ppCertificate
      );

      int
      VMCARevokeCertificateHA(
          Pointer pBinding,
          String pszServerName,
          String  pszCertificate
      );

      int
      VMCAGetCertificateCountHA(
          Pointer pBinding,
          String pwszServerName,
          int     dwStatus,
          IntByReference pdwNumCertificates
      );

      int
      VMCAOpenEnumContextHA(
          Pointer pBinding,
          String pszServerName,
          int     certFilter,
          PointerByReference ppContext
      );

      int
      VMCAGetNextCertificate(
          Pointer            ctx,
          PointerByReference pCert,
          IntByReference     pCurrentIndex,
          IntByReference     enumStatus
          );

      int VMCACloseEnumContext(Pointer pContext);

      int
      VMCAGetCRLHA(
          Pointer pBinding,
          String pwszServerName,
          String pszExistingCRLFileName,
          String pszNewCRLFileName
          );

      int  VMCAReGenCRLHA(Pointer pBinding, String pwszServerName);

      int  VMCAPublishRootCertsHA(Pointer pBinding, String pwszServername);

      int  VMCACloseServer(Pointer pServerContext);

      void VMCAFreeCertificate(Pointer pCertificate);

      void VMCAFreeKey(Pointer p);

      int VMCASetCertValueA(Integer Oid, Pointer data, String pszValue);

      int VMCASetKeyUsageConstraintsA(Pointer data, Integer KeyUsage);

      int VMCAValidateCACertificate(String pszCertificate);

      int
      VMCACreateSigningRequestA(
              Pointer data,
              String  pszPrivateKey,
              String pszPassPhrase,
              PointerByReference pCsr
              );

      int VMCAFreeCSR(Pointer p);

      int VMCAFreePKCS10DataA(Pointer p);

      int VMCAJavaGenCertHA(Pointer pContext, String ServerName, String Name, String Country,
            String Locality, String State, String Organization, String OrgUnit,
            String DnsName, String URIName, String Email, String IPAddress,
            int KeyUsageConstraints, int isSelfSigned, String PrivateKey,
            long tmNotBefore, long tmNotAfter, PointerByReference pCert);

      int  VMCAGetShortError( int errcode, PointerByReference pMsg);
      int  VMCAGetErrorString( int errcode, PointerByReference pMsg);
   }
}
