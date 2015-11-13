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

package com.vmware.identity.rest.idm.server.test.integration.util;

import com.vmware.identity.idm.IIdentityStoreData;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * Helper for identity provider related operations
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class IdentityProviderHelper {

    private CasIdmClient idmClient;

    public IdentityProviderHelper(CasIdmClient client) {
        this.idmClient = client;
    }

    public IIdentityStoreData getIdentityProvider(String tenantName, String identityProviderName) throws Exception {
        return idmClient.getProvider(tenantName, identityProviderName);
    }

    public void updateIdentityProvider(String tenantName, IIdentityStoreData identityStore) throws Exception {
        idmClient.setProvider(tenantName, identityStore);
    }

}
