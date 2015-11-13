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

import com.nimbusds.oauth2.sdk.Scope;
import com.nimbusds.oauth2.sdk.token.AccessToken;
import com.nimbusds.oauth2.sdk.token.AccessTokenType;

/**
 * @author Yehia Zayour
 */
public class HolderOfKeyAccessToken extends AccessToken {
    public static final AccessTokenType ACCESS_TOKEN_TYPE = new AccessTokenType(TokenType.HOK.getName());
    private static final long serialVersionUID = 2015_05_04L;

    public HolderOfKeyAccessToken(String value, long lifeTimeSeconds) {
        super(ACCESS_TOKEN_TYPE, value, lifeTimeSeconds, (Scope) null);
    }

    @Override
    public String toAuthorizationHeader() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean equals(Object other) {
        boolean areEqual = false;
        if (other instanceof HolderOfKeyAccessToken) {
            HolderOfKeyAccessToken otherToken = (HolderOfKeyAccessToken) other;
            areEqual = otherToken.getValue().equals(this.getValue());
        }
        return areEqual;
    }
}
