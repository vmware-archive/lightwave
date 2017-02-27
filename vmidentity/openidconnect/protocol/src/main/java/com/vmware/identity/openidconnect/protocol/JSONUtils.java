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

package com.vmware.identity.openidconnect.protocol;

import java.net.URI;

import net.minidev.json.JSONArray;
import net.minidev.json.JSONObject;

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.util.JSONObjectUtils;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class JSONUtils {
    private JSONUtils() {
    }

    public static long getLong(JSONObject json, String key) throws ParseException {
        Validate.notNull(json, "json");
        Validate.notEmpty(key, "key");

        Object objectValue = json.get(key);
        if (objectValue == null) {
            throw new ParseException(String.format("json is missing %s member", key));
        }

        if (!(objectValue instanceof Long)) {
            throw new ParseException(String.format("json has non-long %s member", key));
        }

        Long longValue = (Long) objectValue;
        return longValue.longValue();
    }

    public static String getString(JSONObject json, String key) throws ParseException {
        Validate.notNull(json, "json");
        Validate.notEmpty(key, "key");

        Object objectValue = json.get(key);
        if (objectValue == null) {
            throw new ParseException(String.format("json is missing %s member", key));
        }

        if (!(objectValue instanceof String)) {
            throw new ParseException(String.format("json has non-string %s member", key));
        }

        String stringValue = (String) objectValue;
        if (stringValue.isEmpty()) {
            throw new ParseException(String.format("json has empty %s member", key));
        }

        return stringValue;
    }

    public static String[] getStringArray(JSONObject json, String key) throws ParseException {
        Validate.notNull(json, "json");
        Validate.notEmpty(key, "key");

        Object objectValue = json.get(key);
        if (objectValue == null) {
            throw new ParseException(String.format("json is missing %s member", key));
        }

        if (!(objectValue instanceof JSONArray)) {
            throw new ParseException(String.format("json has non-array %s member", key));
        }

        String[] stringArrayValue;
        try {
            stringArrayValue = ((JSONArray) objectValue).toArray(new String[0]);
        } catch (ArrayStoreException e) {
            throw new ParseException(String.format("json has non-string-array %s member", key), e);
        }

        if (stringArrayValue.length == 0) {
            throw new ParseException(String.format("json has empty %s member", key));
        }

        return stringArrayValue;
    }

    public static URI getURI(JSONObject json, String key) throws ParseException {
        Validate.notNull(json, "json");
        Validate.notEmpty(key, "key");

        String stringValue = JSONUtils.getString(json, key);
        try {
            return URIUtils.parseURI(stringValue);
        } catch (ParseException e) {
            throw new ParseException(String.format("json has invalid %s member", key), e);
        }
    }

    public static SignedJWT getSignedJWT(JSONObject json, String key) throws ParseException {
        Validate.notNull(json, "json");
        Validate.notEmpty(key, "key");

        String stringValue = JSONUtils.getString(json, key);
        try {
            return SignedJWT.parse(stringValue);
        } catch (java.text.ParseException e) {
            throw new ParseException(String.format("failed to parse %s member", key), e);
        }
    }

    public static JSONObject parseJSONObject(String jsonString) throws ParseException {
        Validate.notEmpty(jsonString, "jsonString");

        try {
            return JSONObjectUtils.parseJSONObject(jsonString);
        } catch (java.text.ParseException e) {
            throw new ParseException("failed to parse JSONObject", e);
        }
    }
}