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
 * a ScopeValue is either one of the following predefined constants or a resource server name
 * we can't use an enum here because we want to represent any resource server name (any string that starts with rs_)
 * this class is still instance controlled though, meaning you can safely do (scopeValue == ScopeValue.OPENID)
 *
 * @author Yehia Zayour
 */
public final class ScopeValue {
    public static final ScopeValue OPENID                       = new ScopeValue("openid");
    public static final ScopeValue OFFLINE_ACCESS               = new ScopeValue("offline_access");
    public static final ScopeValue ID_TOKEN_GROUPS              = new ScopeValue("id_groups");
    public static final ScopeValue ID_TOKEN_GROUPS_FILTERED     = new ScopeValue("id_groups_filtered");
    public static final ScopeValue ACCESS_TOKEN_GROUPS          = new ScopeValue("at_groups");
    public static final ScopeValue ACCESS_TOKEN_GROUPS_FILTERED = new ScopeValue("at_groups_filtered");
    public static final ScopeValue RESOURCE_SERVER_ADMIN_SERVER = new ScopeValue("rs_admin_server");

    private static final Map<String, ScopeValue> stringToEnumMap = new HashMap<String, ScopeValue>();
    static {
        stringToEnumMap.put(OPENID.getValue(),                          OPENID);
        stringToEnumMap.put(OFFLINE_ACCESS.getValue(),                  OFFLINE_ACCESS);
        stringToEnumMap.put(ID_TOKEN_GROUPS.getValue(),                 ID_TOKEN_GROUPS);
        stringToEnumMap.put(ID_TOKEN_GROUPS_FILTERED.getValue(),        ID_TOKEN_GROUPS_FILTERED);
        stringToEnumMap.put(ACCESS_TOKEN_GROUPS.getValue(),             ACCESS_TOKEN_GROUPS);
        stringToEnumMap.put(ACCESS_TOKEN_GROUPS_FILTERED.getValue(),    ACCESS_TOKEN_GROUPS_FILTERED);
        stringToEnumMap.put(RESOURCE_SERVER_ADMIN_SERVER.getValue(),    RESOURCE_SERVER_ADMIN_SERVER);
    }

    private final String value;

    private ScopeValue(String value) {
        this.value = value;
    }

    public String getValue() {
        return this.value;
    }

    public boolean denotesResourceServer() {
        return denotesResourceServer(this.value);
    }

    @Override
    public boolean equals(Object other) {
        return
                other instanceof ScopeValue &&
                ((ScopeValue) other).value.equals(this.value);
    }

    @Override
    public int hashCode() {
        return this.value.hashCode();
    }

    public static ScopeValue parse(String value) throws ParseException {
        Validate.notEmpty(value);

        ScopeValue result = stringToEnumMap.get(value);
        if (result == null) {
            if (denotesResourceServer(value)) {
                result = new ScopeValue(value);
            } else {
                throw new ParseException(ErrorObject.invalidScope("unrecognized scope value: " + value));
            }
        }
        return result;
    }

    private static boolean denotesResourceServer(String value) {
        return
                value.startsWith("rs_") &&
                value.length() > ("rs_").length();
    }
}