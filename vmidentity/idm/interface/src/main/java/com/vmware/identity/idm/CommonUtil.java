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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.math.BigInteger;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Enumeration;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Provides common util functions for idm
 * 
 */
public final class CommonUtil {

    private static final IDiagnosticsLogger _logger = DiagnosticsLoggerFactory.getLogger(CommonUtil.class);
    private static final String HOSTNAME_FILE_NAME = "hostname.txt";

    /**
     * Returns host IP address
     * @return
     * @throws SocketException
     * @throws IOException
     */
    public static synchronized String getHostIPAddress(String configDirPath)
            throws SocketException, IOException
    {
        ValidateUtil.validateNotEmpty(configDirPath, "configDirPath");

        String ipAddress = null;
        FileInputStream reader = null;
        BufferedReader br = null;
        String configFileName = configDirPath;
        if (configFileName.endsWith(File.separator) == false)
        {
            configFileName += File.separator;
        }
        configFileName += HOSTNAME_FILE_NAME;
        _logger.debug(String.format(
                "Reading host name from hostname file [%s]", configFileName));
        try
        {
            reader = new FileInputStream(configFileName);
            br = new BufferedReader(new InputStreamReader(reader));
            String strLine;

            // get hostname ip
            while ((strLine = br.readLine()) != null) {
                ipAddress = strLine.trim();
            }
        }
        catch (Exception ex)
        {
            _logger.warn(
                String.format(
                    "Failed to read host name from hostname file [%s]. Will fallback to ip address.",
                    configFileName
                )
            );
            // failed to retrieve ipaddress from hostname.txt, fall back to
            // original solution
            ipAddress = null;
            ipAddress = findInetAddress(
                new Predicate<InetAddress>() {
                    @Override
                    public boolean matches(InetAddress addr)
                    {
                        return ( (addr != null) && (addr.getClass() == Inet4Address.class) );
                    }
                }
            );

            if (ipAddress == null)
            {
                _logger.info("Failed to find local IPV4 Address.");

                ipAddress = findInetAddress(
                    new Predicate<InetAddress>() {
                        @Override
                        public boolean matches(InetAddress addr)
                        {
                            return ( (addr != null) && (addr.getClass() == Inet6Address.class) );
                        }
                    }
                );

                if (ipAddress == null)
                {
                    _logger.info("Failed to find local IPV6 Address.");
                }
            }

            if (ipAddress == null)
            {
                logAndThrow("Error : Failed to find local either IPV4 or IPV6 Inet Address.");
            }
        }
        finally
        {
            if (reader != null)
            {
                reader.close();
            }
            if (br != null)
            {
                br.close();
            }
        }

        return ipAddress;
    }

    private CommonUtil() {
        // prevent instantiation
    }

    private static void logAndThrow(String msg) {
        _logger.error(msg);
        throw new IllegalStateException(msg);
    }

    public static String Computesha256Hash(File secretFile) throws IOException,
            NoSuchAlgorithmException {
        FileInputStream fs = null;
        try {
            fs = new FileInputStream(secretFile);
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            DigestInputStream dis = new DigestInputStream(fs, md);
            while ((dis.read()) != -1)
                ;
            BigInteger bi = new BigInteger(md.digest());
            return bi.toString(64);
        } finally {
            if (fs != null) {
                fs.close();
            }
        }
    }


   private static interface Predicate<T>
   {
       public boolean matches(T object);
   }

   private static String findInetAddress(Predicate<InetAddress> match) throws SocketException
   {
       String ipAddress = null;
       Enumeration<NetworkInterface> it = NetworkInterface.getNetworkInterfaces();

       while ((ipAddress == null) && it.hasMoreElements())
       {
           NetworkInterface iface = it.nextElement();
           if ( (!iface.isLoopback()) && (iface.isUp()) )
           {
               Enumeration<InetAddress> it2 = iface.getInetAddresses();

               while (it2.hasMoreElements())
               {
                  InetAddress addr = it2.nextElement();
                  if ( match.matches(addr) )
                  {
                     ipAddress = addr.getHostAddress();
                     break;
                  }
               }
           }
       }

       return ipAddress;
   }
}
