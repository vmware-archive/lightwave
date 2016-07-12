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

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public enum LoginMethod {
    PASSWORD("Basic"),
    PERSON_USER_CERTIFICATE("TLSClient"),
    GSS_TICKET("Negotiate"),
    SECURID("RSAAM");

    private static final Map<String, LoginMethod> stringToEnumMap = new HashMap<String, LoginMethod>();
    static {
        for (LoginMethod v : LoginMethod.values()) {
            stringToEnumMap.put(v.getValue(), v);
        }
    }

    private final String value;

    private LoginMethod(String value) {
        this.value = value;
    }

    public String getValue() {
        return this.value;
    }

    public static LoginMethod parse(String value) throws ParseException {
        Validate.notEmpty(value, "value");
        LoginMethod result = stringToEnumMap.get(value);
        if (result == null) {
            throw new ParseException("invalid login method");
        }
        return result;
    }
}