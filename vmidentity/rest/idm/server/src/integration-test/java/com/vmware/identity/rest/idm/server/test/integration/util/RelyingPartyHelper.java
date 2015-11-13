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

import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.idm.server.test.integration.util.data.RelyingPartyDataGenerator;

/**
 * RelyingParty utility which helps calling IDM directly. This helper is mostly used in two phases while running relying party resource integration tests :
 * <li> Preparing test set-up - Before running integration tests </li>
 * <li> Cleaning up set-up - After running integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class RelyingPartyHelper {

    private CasIdmClient idmClient;

    public RelyingPartyHelper(CasIdmClient client) {
        this.idmClient = client;
    }

    public void addRelyingParty(String tenantName, String relyingPartyName, String relyingPartyURI) throws Exception {
        RelyingParty rpToAdd = RelyingPartyDataGenerator.generateRelyingParty(relyingPartyName, relyingPartyURI);
        idmClient.addRelyingParty(tenantName, rpToAdd);
    }

    public void deleteRelyingParty(String tenantName, String relyingPartyName) throws Exception{
        idmClient.deleteRelyingParty(tenantName, relyingPartyName);
    }
}
