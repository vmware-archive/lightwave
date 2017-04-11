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

/**
 * VMware Identity Service
 *
 * Native Adapter
 *
 * @author:  Sriram Nambakam <snambakam@vmware.com>
 *
 * @version: 1.0
 * @since:   2011-12-7
 *
 */

package com.vmware.identity.interop;

import java.io.File;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.vmware.identity.interop.accountmanager.LinuxAccountAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.WinRegistryAdapter;

public abstract class NativeAdapter
{
    private static final Log logger = LogFactory.getLog(LinuxAccountAdapter.class);

    static
    {
        final String propName = "jna.library.path";

        final String LINUX_VMWARE_LIB64_PATH   = "/opt/vmware/lib64";
        final String LINUX_VMDIR_LIB64_PATH    = "/usr/lib/vmware-vmdir/lib64";
        final String LINUX_VMAFD_LIB64_PATH    = "/usr/lib/vmware-vmafd/lib64";
        final String LINUX_LIKEWISE_LIB64_PATH = "/opt/likewise/lib64";

      final String LINUX_OPENSSL_LIB64_PATH = "/usr/lib64";

      final String WIN_REG_VMDIR_PATH = "SOFTWARE\\VMware, Inc.\\VMware Directory Services";
      final String WIN_REG_INSTALL_KEY = "InstallPath";
      final String WIN_REG_VMAFD_PATH = "SOFTWARE\\VMware, Inc.\\VMware afd Services";
      final String WIN_ENV_OPENSSL_BIN = "VMWARE_OPENSSL_BIN";

      String WIN_VMWARE_CIS_VMDIRD_PATH =
              "C:" + File.separator + "Program Files" + File.separator +
              "VMware" + File.separator + "CIS" + File.separator + "vmdird";

      String WIN_VMWARE_CIS_VMAFD_PATH =
                "C:" + File.separator + "Program Files" + File.separator +
                "VMware" + File.separator + "CIS" + File.separator + "vmafdd";

      List<String> paths = null;
      if (SystemUtils.IS_OS_LINUX)
      {
         paths = Arrays.asList(LINUX_VMWARE_LIB64_PATH, LINUX_VMDIR_LIB64_PATH, LINUX_VMAFD_LIB64_PATH, LINUX_LIKEWISE_LIB64_PATH, LINUX_OPENSSL_LIB64_PATH);
      }
      else if (SystemUtils.IS_OS_WINDOWS)
      {
         WinRegistryAdapter regAdapter = WinRegistryAdapter.getInstance();

         // No default if the environment variable is not present
         String opensslPath = null;

            IRegistryKey rootKey = regAdapter
                    .openRootKey((int) RegKeyAccess.KEY_READ);
            try {

                WIN_VMWARE_CIS_VMDIRD_PATH = regAdapter.getStringValue(rootKey, WIN_REG_VMDIR_PATH,
                        WIN_REG_INSTALL_KEY, false);

                logger.info(
                        String.format(
                                    "vmdir install path= [%s]",
                                    WIN_VMWARE_CIS_VMDIRD_PATH));

                WIN_VMWARE_CIS_VMAFD_PATH = regAdapter.getStringValue(rootKey, WIN_REG_VMAFD_PATH,
                        WIN_REG_INSTALL_KEY, false);

                logger.info(
                        String.format(
                                    "vmafd install path= [%s]",
                                    WIN_VMWARE_CIS_VMAFD_PATH));

                String opensslBin = System.getenv(WIN_ENV_OPENSSL_BIN);
                // Strip the executable so we have only the install path...
                if (opensslBin != null && !opensslBin.isEmpty()) {
                    File executable = new File(opensslBin);
                    opensslPath = executable.getParent();
                }

                logger.info(String.format("OpenSSL install path= [%s]", opensslPath));
            }
            catch (Exception e) {

                logger.error("Failed to open install path registry for vmdir or afd:"+e.getMessage());

            } finally {
                if(rootKey != null)
                rootKey.close();
            }

            if (opensslPath != null) {
                paths = Arrays.asList(WIN_VMWARE_CIS_VMDIRD_PATH, WIN_VMWARE_CIS_VMAFD_PATH, opensslPath);
            } else {
                paths = Arrays.asList(WIN_VMWARE_CIS_VMDIRD_PATH, WIN_VMWARE_CIS_VMAFD_PATH);
            }
      }
      else
      {
          throw new IllegalStateException("Only Windows and Linux platforms are supported");
      }

      String propValue = System.getProperty(propName);

      StringBuilder jnalibpath = new StringBuilder(propValue == null ? "" : propValue);

      for (String path : paths )
      {
          File libDir = new File(path);

          if (libDir.exists() && libDir.isDirectory())
          {
              if (jnalibpath.length() > 0 && jnalibpath.charAt(jnalibpath.length()-1) != java.io.File.pathSeparatorChar)
              {
                  jnalibpath.append(File.pathSeparator);
              }

              jnalibpath.append(path);
          }
      }

      propValue = jnalibpath.substring(0);

      if (!propValue.isEmpty())
      {
          System.setProperty(propName, propValue);
      }
    }
}
