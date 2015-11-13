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

package com.vmware.identity.openidconnect.client;

import java.util.HashMap;
import java.util.Map;

/**
 * Client authentication method.
 *
 * @author Jun Sun
 */
public enum ClientAuthenticationMethod {
    PRIVATE_KEY_JWT("private_key_jwt"),
    NONE("none");

    private static final Map<String, ClientAuthenticationMethod> map = new HashMap<String, ClientAuthenticationMethod>();
    static {
        for (ClientAuthenticationMethod rv : ClientAuthenticationMethod.values()) {
            map.put(rv.getValue(), rv);
        }
    }

    private String value;

    private ClientAuthenticationMethod(String value) {
        this.value = value;
    }

    /**
     * Get client authentication method value
     *
     * @return                          String value of client authentication method
     */
    public String getValue() {
        return this.value;
    }

    static ClientAuthenticationMethod getClientAuthenticationMethod(String value) {
        return map.get(value);
    }
}
