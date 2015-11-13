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
package com.vmware.identity.saml.config;

import com.vmware.identity.saml.NoSuchIdPException;

/**
 * Implementation of this interface will provide Configuration extractors per
 * tenant.
 */
public interface ConfigExtractorFactory {

   /**
    * @param tenantName
    *           cannot be null
    * @return configuration extractor per tenant.
    * @throws NoSuchIdPException
    *            when there is no such existing IdP
    * @throws SystemConfigurationException
    *            when configuration cannot be extracted because of unknown
    *            reasons - connectivity, transport between layers, etc..
    */
   ConfigExtractor getConfigExtractor(String tenantName)
      throws NoSuchIdPException, SystemConfigurationException;

}
