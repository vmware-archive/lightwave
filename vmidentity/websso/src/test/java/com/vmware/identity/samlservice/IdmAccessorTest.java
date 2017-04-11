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

package com.vmware.identity.samlservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.security.cert.CertPath;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.vmware.identity.SharedUtils;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IdmDataCreator;
import com.vmware.identity.idm.ServerConfig;

@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class IdmAccessorTest {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(IdmAccessorTest.class);

	static DefaultIdmAccessorFactory factory;
	static IdmAccessor idmAccessor;
	static String tenant;
	static String relyingParty;

	@BeforeClass
	public static void setUp() throws Exception {
		IdmDataCreator.setForceCleanup(true);
        SharedUtils.bootstrap(false); // use real data
		factory = new DefaultIdmAccessorFactory();
		idmAccessor = factory.getIdmAccessor();
		tenant = ServerConfig.getTenant(0);
		String rpName = ServerConfig.getRelyingParty(tenant, 0);
		relyingParty = ServerConfig.getRelyingPartyUrl(rpName);
	}

    @AfterClass
    public static void cleanUp() throws Exception {
        SharedUtils.cleanupTenant();
    }

	@Test
	public final void testSetDefaultTenant() {
		idmAccessor.setDefaultTenant();
		assertEquals(idmAccessor.getTenant(), tenant);
	}

	@Test
	public final void testSetTenant() {
		idmAccessor.setTenant(tenant);
		assertTrue(idmAccessor.getIdpEntityId().contains(tenant));
	}

	@Test
	public final void testGetTenant() {
		idmAccessor.setTenant(tenant);
		assertEquals(idmAccessor.getTenant(), tenant);
	}

	@Test
	public final void testgetCertificatesForRelyingParty() {
		CertPath certPath = idmAccessor.getCertificatesForRelyingParty(relyingParty);
		assertNotNull(certPath);
	}

    @Ignore("bugzilla#1175962")
    @Test
    public final void getAcsForRelyingParty() {
        // test for success (default ACS)
        String acs = idmAccessor.getAcsForRelyingParty(relyingParty, null,
                null, null, true);
        assertNotNull(acs);

        // test for success (index 0)
        acs = idmAccessor.getAcsForRelyingParty(relyingParty, 0, null, null,
                false);
        assertNotNull(acs);

        // test for success (binding)
        acs = idmAccessor.getAcsForRelyingParty(relyingParty, null, null,
                OasisNames.HTTP_POST, false);
        assertNotNull(acs);
        log.debug("IDM accessor test passed!");
    }
}
