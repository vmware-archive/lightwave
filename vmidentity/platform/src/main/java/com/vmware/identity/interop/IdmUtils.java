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

import java.io.File;

import org.apache.commons.lang.SystemUtils;

import com.sun.jna.Platform;
import com.vmware.identity.interop.registry.IRegistryAdapter;
import com.vmware.identity.interop.registry.IRegistryKey;
import com.vmware.identity.interop.registry.RegKeyAccess;
import com.vmware.identity.interop.registry.RegistryAdapterFactory;

/**
 * A set of utility functions supporting VmIdentity.
 * Should not be used by external components.
 */
public class IdmUtils {

   private static final String CONFIG_ROOT_KEY =
         "Software\\VMware\\Identity\\Configuration";

   private static final String TC_LOCAL_PORT_KEY = "StsLocalTcPort";
   private static final String TC_PORT_KEY = "StsTcPort";

   private static final String DEFAULT_LOCAL_PORT = "7444";
   private static final String DEFAULT_PORT = "443";

   private static final String VMWARE_IDENTITY_SERVICES_ROOT_KEY =
           "Software\\VMware, Inc.\\VMware Identity Services";
   private static final String CONFIG_PATH_KEY = "ConfigPath";

   private static final String CONFIG_PATH_DEFAULT;
   private static final String LOG_PATH_DEFAULT;

   private static final String LOG_PATH_KEY = "LogsPath";

   static
   {
       if (Platform.isLinux())
       {
           CONFIG_PATH_DEFAULT = "/etc/vmware-sso/";
           LOG_PATH_DEFAULT = "/var/log/vmware/sso/";
       }
       else if (Platform.isWindows())
       {
           String programDataPath = System.getenv("ProgramData");
           if ( programDataPath.endsWith("\\") == false )
           {
               programDataPath = programDataPath + "\\";
           }
           CONFIG_PATH_DEFAULT = programDataPath + "\\VMware\\CIS\\cfg\\vmware-sso\\";
           LOG_PATH_DEFAULT = programDataPath + "\\VMware\\vCenterServer\\logs\\sso\\";
       }
       else
       {
           throw new RuntimeException( "Unsupported Os. The only supported ones are: Windows, Linux." );
       }
   }

   public static String getStsTomcatLocalPort()
   {
       return getPortValue( TC_LOCAL_PORT_KEY, DEFAULT_LOCAL_PORT );
   }

   public static String getStsTomcatPort()
   {
       return getPortValue( TC_PORT_KEY, DEFAULT_PORT );
   }

   /**
    * @return Returns the path of the configuration directory. It will be File.separator terminated.
    */
   public static String getIdentityServicesConfigDir()
   {
       String configDir = getKeyValue( VMWARE_IDENTITY_SERVICES_ROOT_KEY, CONFIG_PATH_KEY, CONFIG_PATH_DEFAULT );
       if ( configDir.endsWith(File.separator) == false )
       {
           configDir = configDir + File.separator;
       }
       return configDir;
   }

   /**
    * @return Returns the path of the log directory. It will be File.separator terminated.
    */
   public static String getIdentityServicesLogDir()
   {
       String logDir = getKeyValue( VMWARE_IDENTITY_SERVICES_ROOT_KEY, LOG_PATH_KEY, LOG_PATH_DEFAULT );
       if ( logDir.endsWith(File.separator) == false )
       {
           logDir = logDir + File.separator;
       }
       return logDir;
   }

   private static String getPortValue(String portKey, String defaultValue)
   {
       return getKeyValue(CONFIG_ROOT_KEY, portKey, defaultValue);
   }

   private static String getKeyValue(String configRootKey, String portKey, String defaultValue)
   {
       String value = null;
       IRegistryAdapter regAdapter =
               RegistryAdapterFactory.getInstance().getRegistryAdapter();

       IRegistryKey rootKey = regAdapter.openRootKey( (int) RegKeyAccess.KEY_READ);

       try
       {
           value = regAdapter.getStringValue(rootKey, configRootKey, portKey, true);
       }
       finally
       {
           rootKey.close();
       }

       if( value == null || value.isEmpty() )
       {
           value = defaultValue;
       }
       return value;
   }
}
