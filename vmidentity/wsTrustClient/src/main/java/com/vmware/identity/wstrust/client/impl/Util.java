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
package com.vmware.identity.wstrust.client.impl;

import java.util.UUID;

import com.vmware.vim.sso.client.SamlToken;

/**
 * Various utility methods.
 */
public final class Util {

    /**
     * Generate a random UUID with a "_" prefix to be of type NCName
     *
     * @return
     */
    public static String randomNCNameUUID() {
        return "_" + UUID.randomUUID();
    }

    /**
     * Creates a description of the token without any private information
     *
     * @param token
     *            required, must be validated
     * @return description, not null
     */
    public static String createRedactedDescription(SamlToken token) {
        // relies on TokenDelegate objects in chain, having toString()
        return String.format("%s [subject=%s, groups=%s, delegactionChain=%s, startTime=%s, "
                + "expirationTime=%s, renewable=%s, delegable=%s, isSolution=%s," + "confirmationType=%s]", token
                .getClass().getSimpleName(), getTokenSubjectForLog(token), token.getGroupList(), token
                .getDelegationChain(), token.getStartTime(), token.getExpirationTime(), token.isRenewable(), token
                .isDelegable(), token.isSolution(), token.getConfirmationType());
    }

    static String getTokenSubjectForLog(SamlToken token) {
        String tokenSubject = null;
        if (token != null) {
            if (token.getSubject() != null) {
                tokenSubject = token.getSubject().toString();
            } else if (token.getSubjectNameId() != null) {
                tokenSubject = token.getSubjectNameId().toString();
            }
        }
        if (tokenSubject == null) {
            tokenSubject = "NULL";
        }
        return tokenSubject;
    }

    private Util() {
        // prevent instantiation
    }

}
