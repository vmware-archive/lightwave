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

package com.vmware.identity.openidconnect.client;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class RefreshToken {
    private final com.vmware.identity.openidconnect.protocol.RefreshToken refreshToken;

    RefreshToken(com.vmware.identity.openidconnect.protocol.RefreshToken refreshToken) {
        Validate.notNull(refreshToken, "refreshToken");
        this.refreshToken = refreshToken;
    }

    public String getValue() {
        return this.refreshToken.serialize();
    }

    com.vmware.identity.openidconnect.protocol.RefreshToken getRefreshToken() {
        return this.refreshToken;
    }

    public static RefreshToken parse(String refreshTokenString) throws OIDCClientException {
        Validate.notNull(refreshTokenString, "refreshTokenString");
        try {
            return new RefreshToken(com.vmware.identity.openidconnect.protocol.RefreshToken.parse(refreshTokenString));
        } catch (ParseException e) {
            throw new OIDCClientException("failed to parse refresh token", e);
        }
    }
}