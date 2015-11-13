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

/**
 *
 */
package com.vmware.identity.idm;

import java.util.HashSet;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.NoSuchTenantException;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.client.CasIdmClient;

/**
 * Remove our demo data from IDM
 *
 */
public final class IdmDataRemover {
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
            .getLogger(IdmDataRemover.class);

    private static HashSet<String> tenantsToManage = new HashSet<String>();

    public static void addTenant(String tenant) {
        if (!tenantsToManage.contains(tenant)) {
            tenantsToManage.add(tenant);
        }
    }

    public static void removeData(CasIdmClient idmClient) throws Exception {
        logger.debug("IdmDataCreator.removeData called");

        Validate.notNull(idmClient);

        // remove all our tenants
        for (String tenant : tenantsToManage) {
        	try
        	{
        		Tenant existingTenant = idmClient.getTenant(tenant);
                if (existingTenant != null)
                {
                    idmClient.deleteTenant(tenant);
                }
        	}
        	catch(NoSuchTenantException ex)
        	{
        		// continue
        	}
        }

        tenantsToManage.clear();
    }
}
