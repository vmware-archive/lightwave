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

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang3.Validate;

/**
 * @author Yehia Zayour
 */
public final class ResponseType {
    private final Set<ResponseTypeValue> valueSet;

    private ResponseType(Set<ResponseTypeValue> valueSet) {
        this.valueSet = Collections.unmodifiableSet(valueSet);
    }

    public static ResponseType authorizationCode() {
        return new ResponseType(Collections.singleton(ResponseTypeValue.AUTHORIZATION_CODE));
    }

    public static ResponseType idToken() {
        return new ResponseType(Collections.singleton(ResponseTypeValue.ID_TOKEN));
    }

    public static ResponseType idTokenAccessToken() {
        return new ResponseType(new HashSet<ResponseTypeValue>(Arrays.asList(ResponseTypeValue.ID_TOKEN, ResponseTypeValue.ACCESS_TOKEN)));
    }

    public boolean contains(ResponseTypeValue value) {
        Validate.notNull(value, "value");
        return this.valueSet.contains(value);
    }

    @Override
    public boolean equals(Object other) {
        return
                other instanceof ResponseType &&
                ((ResponseType) other).valueSet.equals(this.valueSet);
    }

    @Override
    public int hashCode() {
        return this.valueSet.hashCode();
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (ResponseTypeValue value : this.valueSet) {
            if (sb.length() > 0) {
                sb.append(' ');
            }
            sb.append(value.getValue());
        }
        return sb.toString();
    }

    public static ResponseType parse(String responseTypeString) throws ParseException {
        Validate.notEmpty(responseTypeString, "responseTypeString");

        Set<ResponseTypeValue> valueSet = new HashSet<ResponseTypeValue>();
        String[] parts = responseTypeString.split(" ");
        for (String part : parts) {
            ResponseTypeValue value = ResponseTypeValue.parse(part);
            valueSet.add(value);
        }

        ResponseType result;
        if (valueSet.size() == 1 && valueSet.contains(ResponseTypeValue.AUTHORIZATION_CODE)) {
            result = ResponseType.authorizationCode();
        } else if (valueSet.size() == 1 && valueSet.contains(ResponseTypeValue.ID_TOKEN)) {
            result = ResponseType.idToken();
        } else if (valueSet.size() == 2 && valueSet.contains(ResponseTypeValue.ID_TOKEN) && valueSet.contains(ResponseTypeValue.ACCESS_TOKEN)) {
            result = ResponseType.idTokenAccessToken();
        } else {
            throw new ParseException(ErrorObject.unsupportedResponseType("unsupported response_type"));
        }
        return result;
    }
}