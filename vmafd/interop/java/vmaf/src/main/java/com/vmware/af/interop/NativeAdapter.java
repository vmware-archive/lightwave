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

package com.vmware.af.interop;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.platform.win32.Advapi32Util;
import com.sun.jna.platform.win32.WinReg;

public abstract class NativeAdapter
{
    static
    {
        final String propName = "jna.library.path";

        final String LINUX_VMDIR_LIB64_PATH    = "/opt/vmware/lib64";
        final String LINUX_VMDIR_VC_LIB64_PATH    = "/usr/lib/vmware-vmafd/lib64";
        final String LINUX_LIKEWISE_LIB64_PATH = "/opt/likewise/lib64";

        final String WIN_REG_VMDIR_PATH =
                "SOFTWARE\\VMWare, Inc.\\VMware Directory Services";
        final String WIN_REG_VMAFD_PATH =
                "SOFTWARE\\VMWare, Inc.\\VMware afd Services";
        final String WIN_REG_INSTALL_KEY = "InstallPath";


        String WIN_VMWARE_CIS_VMDIRD_PATH =
                "C:" + File.separator + "Program Files" + File.separator +
                "VMware" + File.separator + "CIS" + File.separator + "vmdird";
        String WIN_VMWARE_CIS_VMAFD_PATH =
                "C:" + File.separator + "Program Files" + File.separator +
                "VMware" + File.separator + "CIS" + File.separator + "vmafdd";

        List<String> paths = null;
        if (SystemUtils.IS_OS_LINUX)
        {
            paths = Arrays.asList(
                        LINUX_VMDIR_VC_LIB64_PATH,
                        LINUX_VMDIR_LIB64_PATH,
                        LINUX_LIKEWISE_LIB64_PATH);
        }
        else if (SystemUtils.IS_OS_WINDOWS)
        {

            if (winRegistryValueExists(WIN_REG_VMDIR_PATH, WIN_REG_INSTALL_KEY))
            {
                WIN_VMWARE_CIS_VMDIRD_PATH =
                        Advapi32Util.registryGetStringValue(
                                WinReg.HKEY_LOCAL_MACHINE,
                                WIN_REG_VMDIR_PATH,
                                WIN_REG_INSTALL_KEY);
            }

            if (winRegistryValueExists(WIN_REG_VMAFD_PATH, WIN_REG_INSTALL_KEY))
            {

                WIN_VMWARE_CIS_VMAFD_PATH =
                        Advapi32Util.registryGetStringValue(
                                WinReg.HKEY_LOCAL_MACHINE,
                                WIN_REG_VMAFD_PATH,
                                WIN_REG_INSTALL_KEY);
            }

            paths = Arrays.asList(
                        WIN_VMWARE_CIS_VMDIRD_PATH,
                        WIN_VMWARE_CIS_VMAFD_PATH);
        }
        else
        {
            throw new IllegalStateException("Only Windows and Linux platforms are supported");
        }

        // Check if the paths exist
        //for (String pathString : paths)
        //{
            //Path path = Paths.get(pathString);
            //if (Files.notExists(path))
            //{
                //throw new IllegalStateException("Path \"" + pathString + "\" does not exist");
            //}
        //}

        String propValue = System.getProperty(propName);

        StringBuilder jnalibpath = new StringBuilder(propValue == null ? "" : propValue);

        for (String path : paths )
        {
            File libDir = new File(path);

            if (libDir.exists() && libDir.isDirectory())
            {
                if (jnalibpath.length() > 0)
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

    /**
     * Returns true if both the key and value exist in the Windows registry
     * @param key registry key where the value is contained
     * @param value value contained in the registry key
     * @return true if the value exists; false if the registry
     *         key or the value do not exist
     */
    private static boolean winRegistryValueExists(String key, String value) {
        return Advapi32Util.registryKeyExists(WinReg.HKEY_LOCAL_MACHINE, key) &&
               Advapi32Util.registryValueExists(WinReg.HKEY_LOCAL_MACHINE, key, value);
    }
}
