/**
 *
 * Copyright 2011 VMware, Inc.  All rights reserved.
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

package com.vmware.vmevent.interop;

import java.io.File;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.SystemUtils;

public abstract class NativeAdapter
{
    static
    {
        final String propName = "jna.library.path";

        final String LINUX_VMEVENT_LIB64_PATH    = "/opt/vmware/lib64";
        final String LINUX_LIKEWISE_LIB64_PATH = "/opt/likewise/lib64";

        final String WIN_VMWARE_CIS_VMEVENT_PATH =
                "C:" + File.separator + "Program Files" + File.separator +
                "VMware" + File.separator + "CIS" + File.separator + "vmafdd";

        List<String> paths = null;
        if (SystemUtils.IS_OS_LINUX)
        {
            paths = Arrays.asList(
                        LINUX_VMEVENT_LIB64_PATH,
                        LINUX_LIKEWISE_LIB64_PATH);
        }
        else if (SystemUtils.IS_OS_WINDOWS)
        {
            paths = Arrays.asList(
                        WIN_VMWARE_CIS_VMEVENT_PATH);
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
}
