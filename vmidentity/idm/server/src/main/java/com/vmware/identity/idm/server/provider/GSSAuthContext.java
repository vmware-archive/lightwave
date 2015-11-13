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

package com.vmware.identity.idm.server.provider;

import org.ietf.jgss.GSSContext;

import com.vmware.identity.idm.ValidateUtil;

public class GSSAuthContext {

    private final String _contextId;
    private final String _serviceName;
    private GSSContext _context;

    public GSSAuthContext(String serviceName, String contextId)
    {
        ValidateUtil.validateNotEmpty(contextId, "Context Id");
        ValidateUtil.validateNotEmpty(serviceName, "Service name");
        _contextId = contextId;
        _serviceName = serviceName;
    }

    public String getContextId()
    {
        return _contextId;
    }

    public String getServiceName()
    {
        return _serviceName;
    }

    public void setContext(GSSContext context)
    {
        _context = context;
    }

    public GSSContext context()
    {
        return _context;
    }
}
