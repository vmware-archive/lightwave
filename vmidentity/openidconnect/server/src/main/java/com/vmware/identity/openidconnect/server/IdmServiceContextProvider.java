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

package com.vmware.identity.openidconnect.server;

import org.apache.commons.lang3.StringUtils;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContext;
import com.vmware.identity.idm.IIdmServiceContext;
import com.vmware.identity.idm.IdmServiceContextFactory;
import com.vmware.identity.idm.client.IServiceContextProvider;

/**
 * @author Yehia Zayour
 */
public class IdmServiceContextProvider extends IServiceContextProvider {
    @Override
    public IIdmServiceContext getServiceContext() {
        IIdmServiceContext serviceContext = null;
        IDiagnosticsContext context = DiagnosticsContextFactory.getCurrentDiagnosticsContext();
        if (context != null && !StringUtils.isEmpty(context.getCorrelationId())) {
            serviceContext = IdmServiceContextFactory.getIdmServiceContext(context.getCorrelationId());
        }
        return serviceContext;
    }
}