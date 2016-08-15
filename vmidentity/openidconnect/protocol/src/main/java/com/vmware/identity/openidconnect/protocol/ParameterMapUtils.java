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
import java.util.Map;

import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.Validate;

import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class ParameterMapUtils {
    private ParameterMapUtils() {
    }

    public static long getLong(Map<String, String> parameters, String key) throws ParseException {
        Validate.notNull(parameters, "parameters");
        Validate.notEmpty(key, "key");

        String stringValue = ParameterMapUtils.getString(parameters, key);
        try {
            return Long.parseLong(stringValue);
        } catch (NumberFormatException e) {
            throw new ParseException(String.format("invalid %s parameter", key), e);
        }
    }

    public static String getString(Map<String, String> parameters, String key) throws ParseException {
        Validate.notNull(parameters, "parameters");
        Validate.notEmpty(key, "key");

        String result = parameters.get(key);
        if (StringUtils.isEmpty(result)) {
            throw new ParseException(String.format("missing %s parameter", key));
        }
        return result;
    }

    public static URI getURI(Map<String, String> parameters, String key) throws ParseException {
        Validate.notNull(parameters, "parameters");
        Validate.notEmpty(key, "key");

        String stringValue = ParameterMapUtils.getString(parameters, key);
        try {
            return URIUtils.parseURI(stringValue);
        } catch (ParseException e) {
            throw new ParseException(String.format("invalid %s parameter", key), e);
        }
    }

    public static SignedJWT getSignedJWT(Map<String, String> parameters, String key) throws ParseException {
        Validate.notNull(parameters, "parameters");
        Validate.notEmpty(key, "key");

        String stringValue = ParameterMapUtils.getString(parameters, key);
        try {
            return SignedJWT.parse(stringValue);
        } catch (java.text.ParseException e) {
            throw new ParseException(String.format("failed to parse %s parameter", key), e);
        }
    }
}