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

import java.security.SecureRandom;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang3.Validate;

/**
 * @author Yehia Zayour
 */
abstract class Identifier {
    private static final int BYTE_LENGTH = 32;
    private static final SecureRandom secureRandom = new SecureRandom();

    private final String value;

    public Identifier() {
        byte[] randomBytes = new byte[BYTE_LENGTH];
        Identifier.secureRandom.nextBytes(randomBytes);
        this.value = Base64.encodeBase64URLSafeString(randomBytes);
    }

    public Identifier(String value) {
        Validate.notEmpty(value, "value");
        this.value = value;
    }

    public String getValue() {
        return this.value;
    }

    @Override
    public int hashCode() {
        return this.value.hashCode();
    }

    @Override
    public abstract boolean equals(Object other);
}