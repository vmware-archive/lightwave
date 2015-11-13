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
package com.vmware.identity.rest.core.server.util;

import java.util.Collection;

import com.vmware.identity.rest.core.server.exception.client.BadRequestException;

/**
 * This class assists in validating arguments.
 *
 * <p>The class is based along the lines of Apache commons-lang. If an argument value
 * is deemed in valid, a {@link BadRequestException} is thrown.
 */
public class Validate {

    public static void isEmpty(Collection<?> collection) {
        isEmpty(collection, "The validated collection is not empty");
    }

    public static void isEqual(Object actual, Object expected) {
        isEqual(actual, expected, "The validated objects are not equal");
    }

    public static void isNull(Object object) {
        isNull(object, "The validated object is not null");
    }

    public static void isTrue(boolean expression) {
        isTrue(expression, "The validated expression is false");
    }

    public static void notEmpty(Collection<?> collection) {
        notEmpty(collection, "The validated collection is empty");
    }

    public static void notEmpty(String string) {
        notEmpty(string, "The validated string is empty");
    }

    public static void notEqual(Object actual, Object expected) {
        notEqual(actual, expected, "The validated objects are equal");
    }

    public static void notNull(Object object) {
        notNull(object, "The validated object is null");
    }

    public static void isEmpty(Collection<?> collection, String message) {
        if (collection != null && collection.size() > 0) {
            throw new BadRequestException(message);
        }
    }

    public static void isEqual(Object actual, Object expected, String message) {
        if (actual == null && expected == null) {
            return;
        }

        if (expected != null && expected.equals(actual)) {
            return;
        }

        throw new BadRequestException(message);
    }

    public static void isNull(Object object, String message) {
        if (object != null) {
            throw new BadRequestException(message);
        }
    }

    public static void isTrue(boolean expression, String message) {
        if (expression == false) {
            throw new BadRequestException(message);
        }
    }

    public static void notEmpty(Collection<?> collection, String message) {
        if (collection == null || collection.size() == 0) {
            throw new BadRequestException(message);
        }
    }

    public static void notEqual(Object actual, Object expected, String message) {
        if (actual == null && expected == null) {
            throw new BadRequestException(message);
        }

        if (expected != null && expected.equals(actual)) {
            throw new BadRequestException(message);
        }
    }

    public static void notEmpty(String string, String message) {
        if (string == null || string.length() == 0) {
            throw new BadRequestException(message);
        }
    }

    public static void notNull(Object object, String message) {
        if (object == null) {
            throw new BadRequestException(message);
        }
    }

}
