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
public enum GrantType {
    AUTHORIZATION_CODE          ("authorization_code"),
    PASSWORD                    ("password"),
    REFRESH_TOKEN               ("refresh_token"),
    CLIENT_CREDENTIALS          ("client_credentials"),
    SOLUTION_USER_CREDENTIALS   ("urn:vmware:grant_type:solution_user_credentials"),
    GSS_TICKET                  ("urn:vmware:grant_type:gss_ticket"),
    PERSON_USER_CERTIFICATE     ("urn:vmware:grant_type:person_user_certificate"),
    SECURID                     ("urn:vmware:grant_type:securid");

    private static final Map<String, GrantType> stringToEnumMap = new HashMap<String, GrantType>();
    static {
        for (GrantType v : GrantType.values()) {
            stringToEnumMap.put(v.getValue(), v);
        }
    }

    private final String value;

    private GrantType(String value) {
        this.value = value;
    }

    public String getValue() {
        return this.value;
    }

    public static GrantType parse(String value) throws ParseException {
        Validate.notEmpty(value, "value");
        GrantType result = stringToEnumMap.get(value);
        if (result == null) {
            throw new ParseException(ErrorObject.unsupportedGrantType("unsupported grant_type parameter"));
        }
        return result;
    }
}