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

import com.vmware.identity.idm.SolutionDetail;
import com.vmware.identity.idm.SolutionUser;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.util.CertificateHelper;
import com.vmware.identity.rest.idm.server.test.integration.util.data.CertificateDataGenerator;

/**
 * Solution user utility which helps calling IDM directly. This helper is mostly used in two phases while running solution user resource integration tests :
 * <li> Preparing test set-up - Before running integration tests </li>
 * <li> Cleaning up set-up - After running integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 */
public class SolutionUserHelper {

    private CasIdmClient idmClient;

    public SolutionUserHelper(CasIdmClient client) {
        this.idmClient = client;
    }

    public void deleteSolutionUser(String domain, String solutionUsername) throws Exception {
        idmClient.deletePrincipal(domain, solutionUsername);
    }

    public void createSolutionUser(String domain, String solutionUsername) throws Exception {
        SolutionDetail solnUserdetails = new SolutionDetail(CertificateHelper.convertToX509(CertificateDataGenerator.getDefaultTestPEMCert()));
        idmClient.addSolutionUser(domain, solutionUsername, solnUserdetails);
    }

    public SolutionUser findSolutionUser(String domain, String solutionUsername) throws Exception {
        return idmClient.findSolutionUser(domain, solutionUsername);
    }
}
