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
package com.vmware.identity.rest.idm.server.test.resources;

import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.isA;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.security.GeneralSecurityException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.PrivateKey;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Locale;

import javax.ws.rs.BadRequestException;
import javax.ws.rs.container.ContainerRequestContext;
import javax.ws.rs.core.SecurityContext;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.CertificateType;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.InvalidArgumentException;
import com.vmware.identity.idm.NoSuchCertificateException;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.core.server.authorization.Config;
import com.vmware.identity.rest.core.server.authorization.Role;
import com.vmware.identity.rest.core.server.exception.client.NotFoundException;
import com.vmware.identity.rest.core.server.exception.server.InternalServerErrorException;
import com.vmware.identity.rest.core.util.CertificateHelper;
import com.vmware.identity.rest.idm.data.CertificateChainDTO;
import com.vmware.identity.rest.idm.data.PrivateKeyDTO;
import com.vmware.identity.rest.idm.data.TenantCredentialsDTO;
import com.vmware.identity.rest.idm.data.attributes.CertificateScope;
import com.vmware.identity.rest.idm.server.resources.CertificateResource;

/**
 * Tests related to CertificateResource
 */
public class CertificateResourceTest {

    // Constants
    private static final String TEST_TENANT = "test.local";
    private static final String TEST_FINGER_PRINT = "f5:67:cc:e6:53:9b:f5:1e:f3:0a:fc:56:52:d7:09:22:d6:af:b5:5f";
    private static final String TEST_CERT_LOC = "src/test/resources/test_cert.pem";
    private static final String CERTIFICATE_GRANULARITY = "CHAIN";

    private CertificateResource certificateResource;
    private IMocksControl mControl;
    private CasIdmClient mockCasIDMClient;
    private SecurityContext mockSecurityContext;
    private ContainerRequestContext request;

    @Before
    public void setUp() {
        mControl = EasyMock.createControl();

        request = EasyMock.createMock(ContainerRequestContext.class);
        EasyMock.expect(request.getLanguage()).andReturn(Locale.getDefault()).anyTimes();
        EasyMock.expect(request.getHeaderString(Config.CORRELATION_ID_HEADER)).andReturn("test").anyTimes();
        EasyMock.replay(request);

        mockCasIDMClient = mControl.createMock(CasIdmClient.class);
        mockSecurityContext = mControl.createMock(SecurityContext.class);
        mControl.createMock(PrivateKey.class);
        certificateResource = new CertificateResource(TEST_TENANT, request, mockSecurityContext);
        certificateResource.setIDMClient(mockCasIDMClient);
    }

    @Test
    public void testGetTenantCerts() throws Exception {
        String testPem = getTestPEMCert();
        Certificate testCert = CertificateHelper.convertToX509(testPem);
        List<Certificate> mockCertificates = new ArrayList<Certificate>();
        mockCertificates.add(testCert);
        mockCertificates.add(testCert);
        Collection<List<Certificate>> certChains = new ArrayList<List<Certificate>>();
        certChains.add(mockCertificates);
        expect(mockCasIDMClient.getTenantCertificates(TEST_TENANT)).andReturn(certChains);
        mControl.replay();

        Collection<CertificateChainDTO> tenantCerts = certificateResource.getCertificates(CertificateScope.TENANT.toString(), CERTIFICATE_GRANULARITY);
        assertEquals(1, tenantCerts.size());
        assertEquals(2, tenantCerts.iterator().next().getCertificates().size());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetCertificatesOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.getTenantCertificates(TEST_TENANT);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        certificateResource.getCertificates(CertificateScope.TENANT.toString(), CERTIFICATE_GRANULARITY);
        mControl.verify();

    }

    @Test(expected = BadRequestException.class)
    public void testGetCertificateOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.getTenantCertificates(TEST_TENANT);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        certificateResource.getCertificates(CertificateScope.TENANT.toString(), CERTIFICATE_GRANULARITY);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetCertificateOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.getTenantCertificates(TEST_TENANT);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        certificateResource.getCertificates(CertificateScope.TENANT.toString(), CERTIFICATE_GRANULARITY);
        mControl.verify();
    }

    @Test
    public void testGetTenantPrivateKeyWithAdminRole() throws Exception {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024);
        KeyPair keypair = keyGen.genKeyPair();
        PrivateKey privateKey = keypair.getPrivate();
        expect(mockCasIDMClient.getTenantPrivateKey(TEST_TENANT)).andReturn(privateKey);

        mControl.replay();
        PrivateKeyDTO resultKey = certificateResource.getPrivateKey();
        assertEquals("RSA", resultKey.getAlgorithm());

        Assert.assertEquals(privateKey,  resultKey.getPrivateKey());
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testGetPrivateKeyOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.getTenantPrivateKey(TEST_TENANT);
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        certificateResource.getPrivateKey();
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testGetPrivateKeyOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.getTenantPrivateKey(TEST_TENANT);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        certificateResource.getPrivateKey();
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testGetPrivateKeyOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.getTenantPrivateKey(TEST_TENANT);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        certificateResource.getPrivateKey();
        mControl.verify();
    }

    @Test
    public void testGetTenantPrivateKeyWithNonAdminRole() throws Exception {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024);
        KeyPair keypair = keyGen.genKeyPair();
        PrivateKey privateKey = keypair.getPrivate();
        expect(mockSecurityContext.isUserInRole(Role.ADMINISTRATOR.name())).andReturn(false);
        expect(mockCasIDMClient.getTenantPrivateKey(TEST_TENANT)).andReturn(privateKey);

        mControl.replay();
        certificateResource.getPrivateKey();
    }

    @Test
    public void testSetTenantPrivateKey() throws Exception {

        expect(mockCasIDMClient.getStsIssuersCertificates(TEST_TENANT)).andReturn(new ArrayList<Certificate>());
        mockCasIDMClient.setTenantCredentials(eq(TEST_TENANT), isA(Collection.class), isA(PrivateKey.class));
        mControl.replay();
        certificateResource.setTenantCredentials(getTestTenantCredentialsDTO());
    }

    @Test(expected = NotFoundException.class)
    public void testSetPrivateKeyOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        expect(mockCasIDMClient.getStsIssuersCertificates(TEST_TENANT)).andReturn(new ArrayList<Certificate>());
        mockCasIDMClient.setTenantCredentials(eq(TEST_TENANT), isA(Collection.class), isA(PrivateKey.class));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        certificateResource.setTenantCredentials(getTestTenantCredentialsDTO());
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testSetPrivateKeyOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        expect(mockCasIDMClient.getStsIssuersCertificates(TEST_TENANT)).andReturn(new ArrayList<Certificate>());
        mockCasIDMClient.setTenantCredentials(eq(TEST_TENANT), isA(Collection.class), isA(PrivateKey.class));
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        certificateResource.setTenantCredentials(getTestTenantCredentialsDTO());
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testSetPrivateKeyOnIDMError_ThrowsInternalServerError() throws Exception {
        expect(mockCasIDMClient.getStsIssuersCertificates(TEST_TENANT)).andReturn(new ArrayList<Certificate>());
        mockCasIDMClient.setTenantCredentials(eq(TEST_TENANT), isA(Collection.class), isA(PrivateKey.class));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        certificateResource.setTenantCredentials(getTestTenantCredentialsDTO());
        mControl.verify();
    }

    @Test
    public void testAddCertificate() throws Exception {
        String testPem = getTestPEMCert();
        CertificateDTO certToAdd = new CertificateDTO(testPem);
        mockCasIDMClient.addCertificate(eq(TEST_TENANT), eq(CertificateHelper.convertToX509(testPem)), eq(CertificateType.STS_TRUST_CERT));
        mControl.replay();
        certificateResource.addCertificate(certToAdd);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testAddCertificateOnNoSuchTenant_ThrowsNotFoundEx() throws Exception {
        CertificateDTO certToAdd = new CertificateDTO(getTestPEMCert());
        mockCasIDMClient.addCertificate(eq(TEST_TENANT), isA(Certificate.class), eq(CertificateType.STS_TRUST_CERT));
        expectLastCall().andThrow(new NoSuchTenantException("no such tenant"));
        mControl.replay();
        certificateResource.addCertificate(certToAdd);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testAddCertificateOnCertificateException_ThrowsBadRequestEx() throws Exception {
        CertificateDTO certToAdd = new CertificateDTO(getTestPEMCert());
        mockCasIDMClient.addCertificate(eq(TEST_TENANT), isA(Certificate.class), eq(CertificateType.STS_TRUST_CERT));
        expectLastCall().andThrow(new CertificateException("invalid certificate"));
        mControl.replay();
        certificateResource.addCertificate(certToAdd);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testAddCertificateOnIDMError_ThrowsInternalServerError() throws Exception {
        CertificateDTO certToAdd = new CertificateDTO(getTestPEMCert());
        mockCasIDMClient.addCertificate(eq(TEST_TENANT), isA(Certificate.class), eq(CertificateType.STS_TRUST_CERT));
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        certificateResource.addCertificate(certToAdd);
        mControl.verify();
    }

    @Test
    public void testDeleteCertificate() throws Exception {
        mockCasIDMClient.deleteCertificate(TEST_TENANT, TEST_FINGER_PRINT, CertificateType.STS_TRUST_CERT);
        mControl.replay();
        certificateResource.delete(TEST_FINGER_PRINT);
        mControl.verify();
    }

    @Test(expected = NotFoundException.class)
    public void testDeleteOnNoSuchCertificate_ThrowsNotFoundEx() throws Exception {
        mockCasIDMClient.deleteCertificate(TEST_TENANT, TEST_FINGER_PRINT, CertificateType.STS_TRUST_CERT);
        expectLastCall().andThrow(new NoSuchCertificateException("no such certificate"));
        mControl.replay();
        certificateResource.delete(TEST_FINGER_PRINT);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteOnInvalidArgument_ThrowsBadRequestEx() throws Exception {
        mockCasIDMClient.deleteCertificate(TEST_TENANT, TEST_FINGER_PRINT, CertificateType.STS_TRUST_CERT);
        expectLastCall().andThrow(new InvalidArgumentException("invalid argument"));
        mControl.replay();
        certificateResource.delete(TEST_FINGER_PRINT);
        mControl.verify();
    }

    @Test(expected = InternalServerErrorException.class)
    public void testDeleteOnIDMError_ThrowsInternalServerError() throws Exception {
        mockCasIDMClient.deleteCertificate(TEST_TENANT, TEST_FINGER_PRINT, CertificateType.STS_TRUST_CERT);
        expectLastCall().andThrow(new IDMException("IDM error"));
        mControl.replay();
        certificateResource.delete(TEST_FINGER_PRINT);
        mControl.verify();
    }

    @Test(expected = BadRequestException.class)
    public void testDeleteCertificateWithEmptyFingerPrint() {
        final String INVALID_FINGER_PRINT = "";
        certificateResource.delete(INVALID_FINGER_PRINT);
    }

    private static String getTestPEMCert() throws IOException {
        byte[] encoded = Files.readAllBytes(Paths.get(TEST_CERT_LOC));
        return new String(encoded, Charset.defaultCharset());
    }

    private PrivateKeyDTO getPrivateKeyDTO() throws IOException, GeneralSecurityException {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024);
        KeyPair keypair = keyGen.genKeyPair();
        PrivateKey privateKey = keypair.getPrivate();
        return new PrivateKeyDTO(privateKey);
    }

    private TenantCredentialsDTO getTestTenantCredentialsDTO() throws IOException, GeneralSecurityException {
        PrivateKeyDTO privateKeyDTO = getPrivateKeyDTO();
        final CertificateDTO certToAdd = new CertificateDTO(getTestPEMCert());
        List<CertificateDTO> certificatesToAdd = new ArrayList<CertificateDTO>() {
            {
                add(certToAdd);
            }
        };
        return new TenantCredentialsDTO(privateKeyDTO, certificatesToAdd);
    }
}
