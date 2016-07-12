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
package com.vmware.directory.rest.server.test.integration.datagenerator;

import java.io.IOException;
import java.security.cert.CertificateException;

import com.vmware.directory.rest.common.data.PrincipalDTO;
import com.vmware.directory.rest.common.data.SolutionUserDTO;
import com.vmware.identity.rest.core.data.CertificateDTO;

/**
 * Data provider for solution user resource integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class SolutionUserDataGenerator {

    private static final boolean DISABLED = false;
    private static final String ALIAS = "_alias";
    private static final String SOLUTION_USER_DESC = "A solution user who is been created for the purpose of integration testing";

    public static SolutionUserDTO createTestSolutionUserDTO(String solutionUsername, String domain, boolean disabled) throws IOException, CertificateException {
        return new SolutionUserDTO.Builder()
        .withName(solutionUsername)
        .withDomain(domain)
        .withDescription(SOLUTION_USER_DESC)
        .withAlias(new PrincipalDTO(solutionUsername + ALIAS, domain))
        .withCertificate(new CertificateDTO(CertificateDataGenerator.getDefaultTestPEMCert()))
        .withDisabled(DISABLED)
        .build();
    }

}
