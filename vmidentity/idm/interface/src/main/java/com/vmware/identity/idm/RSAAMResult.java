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
package com.vmware.identity.idm;

import java.io.Serializable;

import org.apache.commons.lang.Validate;

public class RSAAMResult implements Serializable {
    /**
     *
     */
    private static final long serialVersionUID = 1L;
    private final String rsaSessionID;
    private final PrincipalId principalId;

    public RSAAMResult(String rsaSessionID) {
        Validate.notEmpty(rsaSessionID, "Null or Empty RSA sessionID");
        this.rsaSessionID = rsaSessionID;
        this.principalId = null;
    }

    public RSAAMResult(PrincipalId principalId) {
        Validate.notNull("Null principal!");
        this.rsaSessionID = null;
        this.principalId = principalId;

    }

    public String getRsaSessionID() {
        return rsaSessionID;
    }

    public PrincipalId getPrincipalId() {
        return principalId;
    }

    /**
     * Returns whether the secureID authentication is completed.
     *
     * @return Returns whether the secureID authentication is completed.
     */
    public boolean complete() {
       return getPrincipalId() != null;
    }

}
