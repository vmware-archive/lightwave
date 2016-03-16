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
 * @author Yehia Zayour
 */
public enum ErrorCode {
    INVALID_REQUEST("invalid_request"),
    INVALID_SCOPE("invalid_scope"),
    INVALID_GRANT("invalid_grant"),
    INVALID_CLIENT("invalid_client"),
    UNAUTHORIZED_CLIENT("unauthorized_client"),
    UNSUPPORTED_RESPONSE_TYPE("unsupported_response_type"),
    UNSUPPORTED_GRANT_TYPE("unsupported_grant_type"),
    ACCESS_DENIED("access_denied"),
    SERVER_ERROR("server_error");

    private static final Map<String, ErrorCode> stringToEnumMap = new HashMap<String, ErrorCode>();
    static {
        for (ErrorCode v : ErrorCode.values()) {
            stringToEnumMap.put(v.getValue(), v);
        }
    }

    private final String value;

    private ErrorCode(String value) {
        this.value = value;
    }

    public String getValue() {
        return this.value;
    }

    public static ErrorCode parse(String value) throws ParseException {
        Validate.notEmpty(value, "value");
        ErrorCode result = stringToEnumMap.get(value);
        if (result == null) {
            throw new ParseException("invalid error code value");
        }
        return result;
    }
}