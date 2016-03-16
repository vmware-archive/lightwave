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

package com.vmware.identity.idm.server.clientcert;

import java.util.concurrent.ConcurrentHashMap;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.CrlDownloadException;

public class TenantCrlCache {

    private static ConcurrentHashMap<String, IdmCrlCache> tenantCrlCache = new ConcurrentHashMap<String, IdmCrlCache>();
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TenantCrlCache.class);

    static public ConcurrentHashMap<String, IdmCrlCache> get() {
        return tenantCrlCache;
    }
    public TenantCrlCache() {
    }

    /**
     * Update CRLs for given tenant
     */
    public void refreshCrl(String tenantName) {


        IdmCrlCache crlCache = tenantCrlCache.get(tenantName);
        if (null == crlCache) {
        	return;
        }
        try {
            if (logger.isDebugEnabled()) {
                logger.debug(String.format("TenantCrlCache.refreshCrl(): Start refreshing for tenant %s ... ", tenantName));
            }
            crlCache.refresh();
        } catch (CrlDownloadException e) {
            logger.error("TenantCrlCache fails to refresh some cache for tenant "+tenantName, e.getMessage());
        }
    }

}
