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
package com.vmware.identity.rest.idm.server.test.integration.resources;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;

import java.util.Collection;
import java.util.Locale;

import javax.ws.rs.container.ContainerRequestContext;

import org.easymock.EasyMock;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.experimental.categories.Category;

import com.vmware.identity.idm.CertificateUtil;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.util.CertificateHelper;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;
import com.vmware.identity.rest.idm.server.resources.CertificateResource;
import com.vmware.identity.rest.idm.server.test.annotation.IntegrationTest;
import com.vmware.identity.rest.idm.server.test.integration.util.data.CertificateDataGenerator;

/**
 * Integration tests for Certificate Resource
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
@Category(IntegrationTest.class)
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class CertificateResourceIT extends TestBase {

    private static final String CERTIFICATE_GRANULARITY_CHAIN = "chain";
    private static final String CERTIFICATE_GRANULARITY_LEAF = "leaf";

    private CertificateResource certificateResource;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        certificateResource = new CertificateResource(DEFAULT_TENANT, request, null);
        certificateResource.setIDMClient(idmClient);
    }

    @Test
    public void testGetCertificates_Chain() {
        Collection<CertificateChainDTO> certChains = certificateResource.getCertificates(CertificateScope.TENANT.toString(), CERTIFICATE_GRANULARITY_CHAIN);
        assertEquals(1, certChains.size());
        assertEquals(2, certChains.iterator().next().getCertificates().size());
    }

    @Test
    public void testGetCertificates_Leaf() {
        Collection<CertificateChainDTO> certChains = certificateResource.getCertificates(CertificateScope.TENANT.toString(), CERTIFICATE_GRANULARITY_LEAF);
        assertEquals(1, certChains.size());
        assertEquals(1, certChains.iterator().next().getCertificates().size());
    }

    @Test(expected = NotFoundException.class)
    public void testGetCertificates_WithNonExistentTenant_ThrowsNotFoundEx() {
        /*
         * Currently, IDM is throwing InvalidArgumentException If tenant doesn't exist.
         * Ideally it should throw NoSuchTenantException. We should fix this in IDM and hence
         * re-visit this integration test.
         */
        certificateResource = new CertificateResource("unknown.local", request, null);
        certificateResource.setIDMClient(idmClient);
        certificateResource.getCertificates(CertificateScope.TENANT.toString(), CERTIFICATE_GRANULARITY_CHAIN);
    }

    @Test
    public void testGetPrivateKey() {
        PrivateKeyDTO privateKeyDTO = certificateResource.getPrivateKey();
        assertNotNull(privateKeyDTO);
    }

    @Test(expected = NotFoundException.class)
    public void testGetPrivateKey_WithNonExistentTenant_ThrowsNotFoundEx() {
        certificateResource = new CertificateResource("unknown.local", request, null);
        certificateResource.setIDMClient(idmClient);
        certificateResource.getPrivateKey();
    }

    @Test
    public void testAddCertificate() throws Exception {
        String fingerprint = null;
        try {

            String certInPEM = CertificateDataGenerator.getDefaultTestPEMCert();
            fingerprint = CertificateUtil.generateFingerprint(CertificateHelper.convertToX509(certInPEM));
            CertificateDTO certToAdd = CertificateDTO.builder().withEncoded(certInPEM).build();

            int totalCertsBefore = certificateResourceHelper.getAllStsCertificates(DEFAULT_TENANT).size();
            certificateResource.addCertificate(certToAdd);
            int totalCertsAfter = certificateResourceHelper.getAllStsCertificates(DEFAULT_TENANT).size();

            assertEquals(totalCertsBefore + 1, totalCertsAfter);
        } finally {
            certificateResourceHelper.delete(DEFAULT_TENANT, fingerprint);
        }
    }

    @Test(expected = BadRequestException.class)
    public void testAddCertificate_WithNonExistentTenant_ThrowsNotFoundEx() throws Exception {
        /*
         * Currently, IDM is throwing InvalidArgumentException If tenant doesn't exist.
         * Ideally it should throw NoSuchTenantException. We should fix this in IDM and hence
         * re-visit this integration test.
         */
        String certInPEM = CertificateDataGenerator.getDefaultTestPEMCert();
        CertificateDTO certToAdd = CertificateDTO.builder().withEncoded(certInPEM).build();
        certificateResource = new CertificateResource("unknown.local", request, null);
        certificateResource.setIDMClient(idmClient);
        certificateResource.addCertificate(certToAdd);
    }

    @Test
    public void testDeleteCertificate() throws Exception {
        // Test setup [add certificate]
        String certInPEM = CertificateDataGenerator.getDefaultTestPEMCert();
        String fingerprint = CertificateUtil.generateFingerprint(CertificateHelper.convertToX509(certInPEM));
        certificateResourceHelper.createCertificate(DEFAULT_TENANT, CertificateHelper.convertToX509(certInPEM));

        // Delete STS trusted certificate
        certificateResource.delete(fingerprint);

        assertNull(certificateResourceHelper.getSTSCertificate(DEFAULT_TENANT, fingerprint));
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteCertificate_WithNonExistentTenant_ThrowsNotFoundEx() {
        /*
         * Currently, IDM is throwing InvalidArgumentException If tenant doesn't exist.
         * Ideally it should throw NoSuchTenantException. We should fix this in IDM and hence
         * re-visit this integration test.
         */
        certificateResource = new CertificateResource("unknown.local", request, null);
        certificateResource.setIDMClient(idmClient);
        certificateResource.delete("Fingerprint_Foo");
    }

}
