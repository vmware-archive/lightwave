/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.idm.client;

import java.util.Properties;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Test;

import com.vmware.identity.idm.SsoHealthStatsData;

public class SsoHealthManagementTest {

	private final String CFG_KEY_IDM_HOSTNAME = "idm.server.hostname";

	private Properties _testProps;
	private CasIdmClient _idmClient;

	@After
	public void destroy() {
		_idmClient = null;
	}

	@Test
	public void testGetSsoStatistics() throws Exception {
		CasIdmClient idmClient = getIdmClient();

		String systemTenantName = idmClient.getSystemTenant();

		Assert.assertNotNull(systemTenantName);

		// make generated token count to 1.
		idmClient.incrementGeneratedTokens(systemTenantName);
		idmClient.incrementRenewedTokens(systemTenantName);

		SsoHealthStatsData stats = idmClient.getSsoStatistics(systemTenantName);

		Assert.assertNotNull(stats);
		Assert.assertNotNull(stats.getTenant());
		Assert.assertTrue(stats.getUptimeIDM() > 0);
		Assert.assertTrue(stats.getUptimeSTS() > 0);

		Assert.assertTrue("Generated tokens for this tenant should be positive.", stats.getGeneratedTokensForTenant() > 0);
		Assert.assertTrue("Renewed tokens for this tenant should be positive.", stats.getRenewedTokensForTenant() > 0);
		Assert.assertTrue("Total tokens generated should be positive.", stats.getTotalTokensGenerated() > 0);
		Assert.assertTrue("Total tokens renewed should be positive.", stats.getTotalTokensRenewed() > 0);
	}

	private synchronized CasIdmClient getIdmClient() throws Exception {
		if (_idmClient == null) {
			Properties props = getTestProperties();

			String hostname = props.getProperty(CFG_KEY_IDM_HOSTNAME);

			Assert.assertNotNull(hostname);

			_idmClient = new CasIdmClient(hostname);
		}

		return _idmClient;
	}

	private synchronized Properties getTestProperties() throws Exception {
		if (_testProps == null) {
			_testProps = new Properties();

			_testProps.load(getClass()
					.getResourceAsStream("/config.properties"));
		}

		return _testProps;
	}
}
