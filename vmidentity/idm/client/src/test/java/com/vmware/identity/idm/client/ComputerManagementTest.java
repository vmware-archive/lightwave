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

import java.util.Collection;
import java.util.Properties;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.idm.VmHostData;

public class ComputerManagementTest {

    private final String CFG_KEY_IDM_HOSTNAME = "idm.server.hostname";

    private Properties _testProps;
    private CasIdmClient _idmClient;

    @Test
    public void testGetComputers() throws Exception {
       CasIdmClient idmClient = getIdmClient();

       String systemTenantName = idmClient.getSystemTenant();

       Assert.assertNotNull(systemTenantName);

       Collection<VmHostData> hosts = idmClient.getComputers(systemTenantName, false);

       Assert.assertNotNull(hosts);

       for (VmHostData host : hosts) {
          Assert.assertNotNull(host.getHostName());
       }
    }

    @Test
    public void testGetDomainControllersOnly() throws Exception {
       CasIdmClient idmClient = getIdmClient();

       String systemTenantName = idmClient.getSystemTenant();

       Assert.assertNotNull(systemTenantName);

       Collection<VmHostData> hosts = idmClient.getComputers(systemTenantName, true);

       Assert.assertNotNull(hosts);

       for (VmHostData host : hosts) {
          Assert.assertNotNull(host.getHostName());
          Assert.assertTrue(host.isDomainController());
       }
    }

    private synchronized CasIdmClient getIdmClient() throws Exception
    {
        if (_idmClient == null)
        {
            Properties props = getTestProperties();

            String hostname = props.getProperty(CFG_KEY_IDM_HOSTNAME);

            Assert.assertNotNull(hostname);

            _idmClient = new CasIdmClient(hostname);
        }

        return _idmClient;
    }

    private synchronized Properties getTestProperties() throws Exception
    {
        if (_testProps == null)
        {
            _testProps = new Properties();

            _testProps.load(getClass()
                    .getResourceAsStream("/config.properties"));
        }

        return _testProps;
    }
}
