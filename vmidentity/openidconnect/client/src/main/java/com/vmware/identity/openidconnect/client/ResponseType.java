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

import java.io.Serializable;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

/**
 * Response types.
 *
 * @author Jun Sun
 */
public class ResponseType implements Serializable {

    private static final long serialVersionUID = 1L;

    private final Set<ResponseValue> value = new HashSet<ResponseValue>();

    /**
     * Constructor
     *
     * @param value                     A set of response values
     */
    public ResponseType(Set<ResponseValue> value) {
        this.value.addAll(value);
    }

    /**
     * Get response type
     *
     * @return                          A set of response values
     */
    public Set<ResponseValue> getResponseTypeSet() {
        return Collections.unmodifiableSet(this.value);
    }
}
