/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client;

import com.vmware.vim.sso.client.SamlToken;

/**
 * Credential to acquire new token using an old token for authentication.
 */
public class TokenCredential extends Credential {
    private SamlToken token;
    private boolean isExternal;

    /**
     *
     * If the token parameter is a delegated token then the following rules
     * apply for the requested token attributes:<br>
     * <ul>
     * <li>Lifetime: The validity period of the new token cannot start before or
     * extend beyond that of the current token.</li>
     * <li>Renewing: <br>
     * 1. If delegated token is not renewable then issued token can not be
     * renewable.<br>
     * 2. If delegated token is renewable then renewable token can be issued. In
     * this case the renew restrictions of authentication token are copied over.
     * <li>Delegation:<br>
     * 1. The current list of delegates is always copied from the authentication
     * token<br>
     * 2. Delegation is only permitted if authentication token is delegable.<br>
     * 3. If the authentication token is non-delegable then the STS can not
     * issue a delegable token.
     * <li>Audience restriction: Can be completely changed (replaced by new
     * list)
     * <li>Advice list:<br>
     * 1. Any advice from the authentication token is always copied to the new
     * token.<br>
     * 2. If advice is included in the request then it will be added.
     * </ul>
     *
     * @param token
     *            The authorization token. Cannot be null. Cannot be
     *            {@link ConfirmationType#BEARER}
     */
    public TokenCredential(SamlToken token) {
        this(token, false);
    }

    public TokenCredential(SamlToken token, boolean isExternal) {
        ValidateUtil.validateNotNull(token, "Token");
        this.token = token;
        this.isExternal = isExternal;
    }

    public SamlToken getToken() {
        return token;
    }

    public boolean isExternal() {
        return isExternal;
    }

    @Override
    public String toString() {
        String tokenSubject = "NULL";
        if (token != null) {
            if (token.getSubject() != null) {
                tokenSubject = token.getSubject().toString();
            } else if (token.getSubjectNameId() != null) {
                tokenSubject = token.getSubjectNameId().toString();
            }
        }

        return String.format("TokenCredential [tokenSubject=%s]", tokenSubject);
    }
}
