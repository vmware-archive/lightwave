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
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.cert.CertificateException;

import com.vmware.identity.rest.core.data.CertificateDTO;

/**
 * Data provider for certificate resource integration tests
 *
 * @author Balaji Boggaram Ramanarayan
 */
public class CertificateDataGenerator {

    private static final String TEST_SELF_SIGNED_CERT_LOC = "src/integration-test/resources/test_self_signed_cert.pem";

    public static CertificateDTO createCertificateDTO() throws IOException, CertificateException {
        return CertificateDTO.builder().withEncoded(getDefaultTestPEMCert()).build();
    }

    public static String getDefaultTestPEMCert() throws IOException {
        return getTestPEMCert(TEST_SELF_SIGNED_CERT_LOC);
    }

    public static String getTestPEMCert(String certLocation) throws IOException {
        byte[] encoded = Files.readAllBytes(Paths.get(certLocation));
        return new String(encoded, Charset.defaultCharset());
    }
}

