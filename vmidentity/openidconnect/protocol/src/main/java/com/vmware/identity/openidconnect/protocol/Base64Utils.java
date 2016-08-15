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

import org.apache.commons.lang3.Validate;

import com.nimbusds.jose.util.Base64;

/**
 * @author Yehia Zayour
 */
public final class Base64Utils {
    private Base64Utils() {
        // to prevent instantiation
    }

    public static String encodeToString(String string) {
        Validate.notEmpty(string, "string");
        return Base64.encode(string).toString();
    }

    public static String encodeToString(byte[] bytes) {
        Validate.notNull(bytes, "bytes");
        return Base64.encode(bytes).toString();
    }

    public static byte[] encodeToBytes(byte[] bytes) {
        Validate.notNull(bytes, "bytes");
        return Base64.encode(bytes).toString().getBytes();
    }

    public static String decodeToString(String string) {
        Validate.notEmpty(string, "string");
        return new Base64(string).decodeToString();
    }

    public static byte[] decodeToBytes(String string) {
        Validate.notEmpty(string, "string");
        return new Base64(string).decode();
    }
}