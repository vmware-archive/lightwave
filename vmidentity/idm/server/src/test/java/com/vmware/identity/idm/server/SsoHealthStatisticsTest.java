/*
 *
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
 *
 */

package com.vmware.identity.idm.server;

import java.util.Calendar;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import com.vmware.identity.idm.SsoHealthStatsData;

public class SsoHealthStatisticsTest {

	private static SsoHealthStatistics statisticsService;
	private static final String TENANT = "example.com";

	@Before
	public void init() throws Exception {
        statisticsService = new SsoHealthStatistics();
        statisticsService.setUpTimeIDMService();
        try {
            Thread.sleep(2000);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
        }
	}

	@After
	public void destroy() throws Exception {
	    statisticsService.removeTenantStats(TENANT);
		statisticsService = null;
	}

	@Test
	public void testGetUpTimeIDMService() throws Exception {
	    SsoHealthStatsData statsData = statisticsService.getSsoStatistics(TENANT);

		Assert.assertTrue("Uptime IDM service should not be 0", statsData.getUptimeIDM() > 0);
	}

	@Test
	public void testGetUpTimeSTSService() throws Exception {

	    statisticsService.incrementGeneratedTokens(TENANT);
	    try {
            Thread.sleep(2000);
        } catch (InterruptedException ex) {
            Thread.currentThread().interrupt();
        }
	    SsoHealthStatsData statsData = statisticsService.getSsoStatistics(TENANT);

		Assert.assertTrue("Uptime STS service should not be 0", statsData.getUptimeSTS() > 0);
	}

	@Test
	public void testGetRenewedTokens() throws Exception {

		statisticsService.incrementRenewedTokens(TENANT);

		SsoHealthStatsData statsData = statisticsService.getSsoStatistics(TENANT);
		Assert.assertEquals("Renewed tokens for this tenant should be 1", 1,
		        statsData.getRenewedTokensForTenant());
		Assert.assertTrue("Total renewed tokens should be >=1 ",
		        statsData.getTotalTokensRenewed() >= 1);

		Assert.assertTrue("Uptime STS service should not be 0",
		        statsData.getUptimeIDM() > 0);

		statsData = statisticsService.getSsoStatistics("fakeTenant");
		Assert.assertEquals("Renewed tokens for fake tenant should be 0", 0,
		        statsData.getRenewedTokensForTenant());
	}

	@Test
	public void testGetTotalGeneratedTokens() throws Exception {

		statisticsService.incrementGeneratedTokens(TENANT);
		statisticsService.incrementGeneratedTokens(TENANT);
		statisticsService.incrementGeneratedTokens(TENANT);

		SsoHealthStatsData statsData = statisticsService.getSsoStatistics(TENANT);
		Assert.assertEquals("Generated tokens for this tenant should be 3", 3,
		        statsData.getGeneratedTokensForTenant());
		Assert.assertTrue("Total Generated tokens should be >= 3",
		        statsData.getTotalTokensGenerated() >= 3);

		Assert.assertTrue("Uptime STS service should not be 0",
		        statsData.getUptimeIDM() > 0);

		// Add some more tokens for generated
		statisticsService.incrementGeneratedTokens("coke.com");
		statisticsService.incrementGeneratedTokens("coke.com");

		statsData = statisticsService.getSsoStatistics("coke.com");
		Assert.assertEquals("Generated tokens for this tenant should be 2", 2,
		        statsData.getGeneratedTokensForTenant());
		Assert.assertTrue("Total Generated tokens should be >= 5",
		        statsData.getTotalTokensGenerated() >= 5);

		statsData = statisticsService.getSsoStatistics("fakeTenant");
		Assert.assertEquals("Generated tokens for this tenant should be 0", 0,
		        statsData.getGeneratedTokensForTenant());
	}

	@Test
	public void testGetTotalRenewedTokens() throws Exception {

		statisticsService.incrementRenewedTokens(TENANT);
		statisticsService.incrementRenewedTokens(TENANT);
		statisticsService.incrementRenewedTokens(TENANT);

		SsoHealthStatsData statsData = statisticsService.getSsoStatistics(TENANT);
		Assert.assertEquals("Renewed tokens for this tenant should be 3", 3,
		        statsData.getRenewedTokensForTenant());
		Assert.assertTrue("Total Renewed tokens should be >= 3",
		        statsData.getTotalTokensRenewed() >= 3);

		Assert.assertTrue("Uptime STS service should not be 0",
		        statsData.getUptimeIDM() > 0);

		// Add some more tokens for generated
		statisticsService.incrementRenewedTokens("coke.com");
		statisticsService.incrementRenewedTokens("coke.com");

		statsData = statisticsService.getSsoStatistics("coke.com");
		Assert.assertEquals("Renewed tokens for this tenant should be 2", 2,
		        statsData.getRenewedTokensForTenant());
		Assert.assertTrue("Total Renewed tokens should be >= 5",
		        statsData.getTotalTokensRenewed() >= 5);

		statsData = statisticsService.getSsoStatistics("fakeTenant");
		Assert.assertEquals("Renewed tokens for this tenant should be 0", 0,
		        statsData.getRenewedTokensForTenant());
	}
}
