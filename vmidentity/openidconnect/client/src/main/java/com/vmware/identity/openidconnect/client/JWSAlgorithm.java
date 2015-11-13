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
 * JSON Web Signature algorithm.
 *
 * @author Jun Sun
 */
public enum JWSAlgorithm {
    RS256("RS256");

    private static final Map<String, JWSAlgorithm> map = new HashMap<String, JWSAlgorithm>();
    static {
        for (JWSAlgorithm v : JWSAlgorithm.values()) {
            map.put(v.getValue(), v);
        }
    }

    private final String value;

    private JWSAlgorithm(String value) {
        this.value = value;
    }

    /**
     * Get JWS algorithm value
     *
     * @return                          String value of JWS algorithm
     */
    public String getValue() {
        return this.value;
    }

    static JWSAlgorithm getJWSAlgorithm(String value) {
        return map.get(value);
    }
}
