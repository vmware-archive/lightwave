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

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang3.Validate;

/**
 * @author Yehia Zayour
 */
public final class Scope {
    public static final Scope OPENID = new Scope(Collections.singleton(ScopeValue.OPENID));

    private final Set<ScopeValue> valueSet;

    public Scope(Set<ScopeValue> valueSet) {
        Validate.notEmpty(valueSet, "valueSet");
        this.valueSet = Collections.unmodifiableSet(valueSet);
    }

    public Set<ScopeValue> getScopeValues() {
        return this.valueSet;
    }

    public boolean contains(ScopeValue value) {
        Validate.notNull(value, "value");
        return this.valueSet.contains(value);
    }

    @Override
    public boolean equals(Object other) {
        return
                other instanceof Scope &&
                ((Scope) other).valueSet.equals(this.valueSet);
    }

    @Override
    public int hashCode() {
        return this.valueSet.hashCode();
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (ScopeValue value : this.valueSet) {
            if (sb.length() > 0) {
                sb.append(' ');
            }
            sb.append(value.getValue());
        }
        return sb.toString();
    }

    public static Scope parse(String scopeString) throws ParseException {
        Validate.notEmpty(scopeString, "scopeString");

        Set<ScopeValue> valueSet = new HashSet<ScopeValue>();
        String[] parts = scopeString.split(" ");
        for (String part : parts) {
            if (part.isEmpty()) {
                throw new ParseException(ErrorObject.invalidScope("scope must be a sequence of single-space-delimited values"));
            }
            ScopeValue value = ScopeValue.parse(part);
            valueSet.add(value);
        }

        validate(valueSet);

        return new Scope(valueSet);
    }

    private static void validate(Set<ScopeValue> valueSet) throws ParseException {
        if (!valueSet.contains(ScopeValue.OPENID)) {
            throw new ParseException(ErrorObject.invalidScope("missing openid scope value"));
        }

        if (valueSet.contains(ScopeValue.ID_TOKEN_GROUPS) && valueSet.contains(ScopeValue.ID_TOKEN_GROUPS_FILTERED)) {
            throw new ParseException(ErrorObject.invalidScope("id_groups together with id_groups_filtered is not allowed"));
        }

        if (valueSet.contains(ScopeValue.ACCESS_TOKEN_GROUPS) && valueSet.contains(ScopeValue.ACCESS_TOKEN_GROUPS_FILTERED)) {
            throw new ParseException(ErrorObject.invalidScope("at_groups together with at_groups_filtered is not allowed"));
        }

        boolean resourceServerRequested = false;
        for (ScopeValue scopeValue : valueSet) {
            if (scopeValue.denotesResourceServer()) {
                resourceServerRequested = true;
                break;
            }
        }

        if (valueSet.contains(ScopeValue.ID_TOKEN_GROUPS_FILTERED) && !resourceServerRequested) {
            throw new ParseException(ErrorObject.invalidScope("id_groups_filtered requested but no resource server requested"));
        }

        if (valueSet.contains(ScopeValue.ACCESS_TOKEN_GROUPS_FILTERED) && !resourceServerRequested) {
            throw new ParseException(ErrorObject.invalidScope("at_groups_filtered requested but no resource server requested"));
        }
    }
}