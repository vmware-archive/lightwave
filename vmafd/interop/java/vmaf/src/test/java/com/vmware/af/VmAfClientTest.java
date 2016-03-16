/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.af;

import org.junit.Assert;
import org.junit.Test;

public class VmAfClientTest
{
	private static String _hostname = "localhost";
	private static String _domainName = "acme.com";
	private static String _LDU = "3F2504E0-4F89-11D3-9A0C-0305E82C3301";
	private static String _CMLocation = "bolts.acme.com";
	private static String _DCName = "thunder.acme.com";
	private static String _LSLocation = "https://thunder.acme.com/lookupservice/sdk";

	@Test
	public void testDomainNameConfig()
	{
		VmAfClient.setDomainName(_hostname, _domainName);

		String domainName = VmAfClient.getDomainName(_hostname);

		Assert.assertEquals(domainName, _domainName);
	}

	@Test
	public void testLDUConfig()
	{
		VmAfClient.setLDU(_hostname, _LDU);

		String ldu = VmAfClient.getLDU(_hostname);

		Assert.assertEquals(ldu, _LDU);
	}

	@Test
	public void testDomainControllerConfig()
	{
		VmAfClient.setDomainController(_hostname, _DCName);

		String dcName = VmAfClient.getDomainController(_hostname);

		Assert.assertEquals(dcName, _DCName);
	}

	@Test
	public void testCMLocationConfig()
	{
		VmAfClient.setCMLocation(_hostname, _CMLocation);

		String cmLocation = VmAfClient.getCMLocation(_hostname);

		Assert.assertEquals(cmLocation, _CMLocation);
	}

	@Test
	public void testLSLocationConfig()
	{
		VmAfClient.setDomainController(_hostname, _DCName);

		String lsLocation = VmAfClient.getLSLocation(_hostname);

		Assert.assertEquals(lsLocation, _LSLocation);
	}
}
