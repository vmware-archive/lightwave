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

import org.apache.commons.lang3.Validate;

/**
 * @author Jun Sun
 * @author Yehia Zayour
 */
public enum ResponseTypeValue {
    AUTHORIZATION_CODE("code"),
    ID_TOKEN("id_token"),
    ACCESS_TOKEN("token");

    private static final Map<String, ResponseTypeValue> stringToEnumMap = new HashMap<String, ResponseTypeValue>();
    static {
        for (ResponseTypeValue v : ResponseTypeValue.values()) {
            stringToEnumMap.put(v.getValue(), v);
        }
    }

    private final String value;

    private ResponseTypeValue(String value) {
        this.value = value;
    }

    public String getValue() {
        return this.value;
    }

    public static ResponseTypeValue parse(String value) throws ParseException {
        Validate.notEmpty(value, "value");
        ResponseTypeValue result = stringToEnumMap.get(value);
        if (result == null) {
            throw new ParseException("invalid response_type value");
        }
        return result;
    }
}