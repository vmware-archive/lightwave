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

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.id.Subject;
import com.vmware.identity.idm.PrincipalId;

/**
 * @author Yehia Zayour
 */
public abstract class User {
    private final PrincipalId principalId;
    private final String tenant;

    public User(PrincipalId principalId, String tenant) {
        Validate.notNull(principalId, "principalId");
        Validate.notEmpty(tenant, "tenant");
        this.principalId = principalId;
        this.tenant = tenant;
    }

    public PrincipalId getPrincipalId() {
        return this.principalId;
    }

    public String getTenant() {
        return this.tenant;
    }

    public Subject getSubject() {
        return new Subject(this.principalId.getUPN());
    }
}
