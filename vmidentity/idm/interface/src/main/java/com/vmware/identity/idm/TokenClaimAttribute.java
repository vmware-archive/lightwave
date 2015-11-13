/*
 *
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
 *
 */

package com.vmware.identity.idm;

import java.io.Serializable;

/**
 * Immutable data type for representing attribute name and value in an external token claim.
 */
public final class TokenClaimAttribute implements Serializable {

    private static final long serialVersionUID = 1L;

    private final String claimName;

    private final String claimValue;

    public TokenClaimAttribute (String claimName, String claimValue) {
        ValidateUtil.validateNotEmpty(claimName, "Claim name is null or empty");
        ValidateUtil.validateNotEmpty(claimValue, "Claim value is null or empty");
        // we use '#' to delimit claim name and value in internal identity store. Thus claim name should not contain '#'
        if (claimName.contains("#")) {
            throw new IllegalArgumentException(String.format("Encountered invalid claim name {%s}. Claim name contains '#'.",
                    claimName));
        }
        this.claimName = claimName;
        this.claimValue = claimValue;
    }

    public String getClaimName() {
        return this.claimName;
    }

    public String getClaimValue() {
        return this.claimValue;
    }

    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }

        if (!(o instanceof TokenClaimAttribute)) {
            return false;
        }

        TokenClaimAttribute t = (TokenClaimAttribute) o;

        // case insensitive
        return this.claimName.equalsIgnoreCase(t.getClaimName()) && this.claimValue.equalsIgnoreCase(t.getClaimValue());
    }

    @Override
    public int hashCode() {
        int hash = 3;
        hash = 53 * hash + this.getClaimName().hashCode();
        hash = 53 * hash + this.getClaimValue().hashCode();
        return hash;
    }

    @Override
    public String toString() {
        return "Claim Name: " + this.getClaimName() + System.lineSeparator()
                + "Claim Value: " + this.getClaimValue();
    }
}
