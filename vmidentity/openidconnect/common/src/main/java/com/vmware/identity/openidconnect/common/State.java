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

import java.util.Objects;

import org.apache.commons.lang3.StringEscapeUtils;
import org.apache.commons.lang3.Validate;

/**
 * @author Jun Sun
 * @author Yehia Zayour
 */
public final class State extends Identifier {
    public State() {
        // Identifier will generate a new value
    }

    public State(String value) {
        super(value);
        Validate.isTrue(htmlFriendly(value), "state parameter must be html friendly");
    }

    @Override
    public boolean equals(Object other) {
        return
                other instanceof State &&
                ((State) other).getValue().equals(this.getValue());
    }

    public static State parse(String value) throws ParseException {
        Validate.notEmpty(value, "value");

        if (!htmlFriendly(value)) {
            throw new ParseException("state parameter must be html friendly");
        }

        return new State(value);
    }

    private static boolean htmlFriendly(String value) {
        return Objects.equals(value, StringEscapeUtils.escapeHtml4(value));
    }
}