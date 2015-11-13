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

package com.vmware.identity.idm.server.config;

import com.vmware.identity.idm.server.ServerUtils;
import com.vmware.identity.idm.server.config.directory.DirectoryConfigStore;


/**
 * Created by IntelliJ IDEA. User: snambakam Date: 12/23/11 Time: 1:59 PM To
 * change this template use File | Settings | File Templates.
 */
public class ConfigStoreFactory implements IConfigStoreFactory
{

   private ConfigStoreType _configStoreType = ConfigStoreType.UNKNOWN;

   public ConfigStoreFactory()
   {
       IdmServerConfig settings = IdmServerConfig.getInstance();
       this._configStoreType = settings.getTypeOfConfigurationStore();
   }

   @Override
   public IConfigStore getConfigStore()
   {
      switch (_configStoreType) {
          case VMWARE_DIRECTORY:

              return buildDirectoryConfigStore();

          default:

              throw new IllegalStateException(String.format(
               "Unrecognized config store type [%s]",
               _configStoreType.toString()));
      }
   }

   protected DirectoryConfigStore buildDirectoryConfigStore()
   {
      IdmServerConfig settings = IdmServerConfig.getInstance();
      return new DirectoryConfigStore(
            settings.getSystemDomainConnectionInfo(),
            ServerUtils.getDomainDN(settings.getDirectoryConfigStoreDomain()));
   }
}
