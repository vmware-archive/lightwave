/*
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
 */
package com.vmware.identity.saml.config.impl;

import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.saml.NoSuchIdPException;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.config.SystemConfigurationException;

public final class ConfigExtractorFactoryImpl implements ConfigExtractorFactory {

   private final CasIdmClient idmClient;

   public ConfigExtractorFactoryImpl(String idmHostName) {
      assert idmHostName != null;

      this.idmClient = new CasIdmClient(idmHostName);
   }

   @Override
   public ConfigExtractor getConfigExtractor(String tenantName)
      throws NoSuchIdPException, SystemConfigurationException {
      assert tenantName != null;
      return new ConfigExtractorImpl(tenantName, idmClient);
   }

}
