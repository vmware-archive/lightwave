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

package com.vmware.identity.openidconnect.common;

import java.util.HashMap;
import java.util.Map;

import javax.servlet.http.HttpServletResponse;

/**
 * @author Yehia Zayour
 */
public enum StatusCode {
    OK                      (HttpServletResponse.SC_OK),
    FOUND                   (HttpServletResponse.SC_FOUND),
    BAD_REQUEST             (HttpServletResponse.SC_BAD_REQUEST),
    FORBIDDEN               (HttpServletResponse.SC_FORBIDDEN),
    UNAUTHORIZED            (HttpServletResponse.SC_UNAUTHORIZED),
    INTERNAL_SERVER_ERROR   (HttpServletResponse.SC_INTERNAL_SERVER_ERROR);

    private static final Map<Integer, StatusCode> intToEnumMap = new HashMap<Integer, StatusCode>();
    static {
        for (StatusCode v : StatusCode.values()) {
            intToEnumMap.put(v.getValue(), v);
        }
    }

    private final int value;

    private StatusCode(int value) {
        this.value = value;
    }

    public int getValue() {
        return this.value;
    }

    public static StatusCode parse(int value) throws ParseException {
        StatusCode result = intToEnumMap.get(value);
        if (result == null) {
            throw new ParseException("unrecognized status code");
        }
        return result;
    }
}