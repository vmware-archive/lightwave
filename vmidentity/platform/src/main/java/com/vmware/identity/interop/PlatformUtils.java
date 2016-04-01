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

package com.vmware.identity.interop;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.Charset;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Platform;
import com.vmware.identity.interop.ldap.DirectoryStoreProtocol;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;

public class PlatformUtils
{
   private static final Log logger = LogFactory.getLog(PlatformUtils.class);
   public static final String CONFIG_DIRECTORY_LDAP_PORT_KEY = "LdapPort";
   public static final String CONFIG_DIRECTORY_LDAPS_PORT_KEY = "LdapsPort";
   public static final String PERFDATASINK_LOG
       = "com.vmware.identity.performanceSupport.PerfDataSink";

   private static Charset LDAP_SERVER_ENCODE_CHARSET  = Charset.forName("UTF-8");
   private static Charset WINDOWS_WCHAR_CHARSET  = Charset.forName("UTF-16LE");
   private static int LOGON32_LOGON_INTERACTIVE = 2;
   private static int LOGON32_PROVIDER_DEFAULT = 0;
   private static int DS_FORCE_REDISCOVERY_FLAG = 0x00000001;
   private static int DS_RETURN_DNS_NAME_FLAG = 0x40000000;
   private static int DS_RETURN_FLAT_NAME_FLAG = 0x80000000;

   /**
    * Convenience method creating URI with LDAP scheme
    * @param hostName
    * @param port
    * @return an URI object with the LDAP scheme, using the
    *         specified hostname and port.
    */
   public static URI getConnectionUriDefaultScheme(String hostName, int port)
   {
      return DirectoryStoreProtocol.LDAP.getUri(hostName, port);
   }

   /**
    * Create URI with specified scheme, hostname and port
    * @param scheme
    * @param hostName
    * @param port
    * @return an URI object, or null in case of URISyntaxException.
    */
   public static URI getConnectionUri(String scheme, String hostName, int port)
   {
      String uriStr = String.format("%s://%s:%d", scheme, hostName, port);
      try {
         return new URI(uriStr);
      }
      catch (URISyntaxException e)
      {
         logger.error(String.format("invalid uri string [%s] : %s", uriStr, e.getMessage()));
         return null;
      }
   }

   /**
    * Reading string value from registry
    * @param regAdapter
    * @param rootKey
    * @param key
    * @param value
    * @return a string value read from registry. An IllegalStateException will
    *         be thrown for exception encountered during registry read.
    */
   public static String getStringValue(
         IRegistryAdapter regAdapter,
         IRegistryKey     rootKey,
         String           key,
         String           value
         )
   {
      try
      {
         return regAdapter.getStringValue(
               rootKey,
               key,
               value,
               false);
      }
      catch(Exception ex)
      {
         throw new IllegalStateException(
               String.format(
                     "Failed to retrieve registry string value [key : %s] [value : %s]: Exception: %s",
                     key,
                     value,
                     ex.getMessage()));
      }
   }

   /**
    * Reading string value from registry
    * @param regAdapter
    * @param rootKey
    * @param key
    * @param value
    * @return an integer read from registry. An IllegalStateException will
    *         be thrown for exception encountered during registry read.
    */
   public static Integer getIntValue(
         IRegistryAdapter regAdapter,
         IRegistryKey     rootKey,
         String           key,
         String           value
         )
   {
         try
         {
            return regAdapter.getIntValue(
                  rootKey,
                  key,
                  value,
                  true);
         }
         catch(Exception ex)
         {
            throw new IllegalStateException(
                  String.format(
                        "Failed to retrieve registry int value [key : %s] [value : %s]: Exception: %s",
                        key,
                        value,
                        ex.getMessage()));
         }
   }

   public static void validateNotNull(Object object, String objectName)
   {
      if (object == null)
      {
         logAndThrow(String.format("value of %s is null", objectName));
      }
   }

   private static void logAndThrow(String msg)
   {
      logger.error(msg);
      throw new IllegalArgumentException(msg);
   }

   public static Charset getLdapServerCharSet()
   {
       // always use the "UTF-8 char set, independent of jre used.
       return LDAP_SERVER_ENCODE_CHARSET;
   }

   public static Charset getWindowsWcharCharSet()
   {
       assert Platform.isWindows();
       return WINDOWS_WCHAR_CHARSET;
   }

   public static int getWindowsLogonMode()
   {
       assert Platform.isWindows();
       return LOGON32_LOGON_INTERACTIVE;
   }

   public static int getWindowsLogonProvider()
   {
       assert Platform.isWindows();
       return LOGON32_PROVIDER_DEFAULT;
   }

   public static int getFlagsForGetDcInfo()
   {
       return DS_RETURN_DNS_NAME_FLAG ;
   }

   public static int getFlagsForGetDcInfoWithRediscovery()
   {
       return DS_RETURN_DNS_NAME_FLAG | DS_FORCE_REDISCOVERY_FLAG;
   }

   public static int getFlagsForGetFlatDcInfo()
   {
       return DS_RETURN_FLAT_NAME_FLAG;
   }

   public static boolean renameFile(File oldFile, File newFile)
   {
       if (oldFile == null || newFile == null)
           return false;;

       boolean renamed = false;
       if (Platform.isLinux())
       {
           renamed = oldFile.renameTo(newFile);
       }
       // On windows, File.renameTo does not actually work, implement a version of our own
       else if (Platform.isWindows())
       {
           renamed = renameFileOnWindows(oldFile, newFile);
       }

       return renamed;
   }

   private static boolean renameFileOnWindows(File oldFile, File newFile)
   {
       String sCurrentLine = "";

       try
       {
           BufferedReader bufReader = new BufferedReader(new FileReader(oldFile.getAbsoluteFile()));
           BufferedWriter bufWriter = new BufferedWriter(new FileWriter(newFile.getAbsoluteFile()));

           while ((sCurrentLine = bufReader.readLine()) != null)
           {
               bufWriter.write(sCurrentLine);
               bufWriter.newLine();
           }
           bufReader.close();
           bufWriter.close();
           oldFile.delete();

           return true;
       }
       catch(Exception e)
       {
           return false;
       }
   }

   public static String canonicalizeStringForLdapDN(String name) {
       //@see RFC 2253
       StringBuilder sb = new StringBuilder();
       if ((name.length() > 0) && ((name.charAt(0) == ' ') || (name.charAt(0) == '#'))) {
           sb.append('\\');
       }
       for (int i = 0; i < name.length(); i++) {
           char curChar = name.charAt(i);
           switch (curChar) {
               case '\\':
                   sb.append("\\\\");
                   break;
               case ',':
                   sb.append("\\,");
                   break;
               case '+':
                   sb.append("\\+");
                   break;
               case '"':
                   sb.append("\\\"");
                   break;
               case '<':
                   sb.append("\\<");
                   break;
               case '>':
                   sb.append("\\>");
                   break;
               case ';':
                   sb.append("\\;");
                   break;
               default:
                   sb.append(curChar);
           }
       }
       if ((name.length() > 1) && (name.charAt(name.length() - 1) == ' ')) {
           sb.insert(sb.length() - 1, '\\');
       }
       return sb.toString();
   }

}
