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

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.openidconnect.common.GrantType;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public final class GSSTicketGrant extends AuthorizationGrant {
    private static final GrantType GRANT_TYPE = GrantType.GSS_TICKET;

    private final String contextId;
    private final byte[] gssTicket;

    public GSSTicketGrant(String contextId, byte[] gssTicket) {
        super(GRANT_TYPE);

        Validate.notEmpty(contextId, "contextId");
        Validate.notNull(gssTicket, "gssTicket");

        this.contextId = contextId;
        this.gssTicket = gssTicket;
    }

    public String getContextID() {
        return this.contextId;
    }

    public byte[] getGSSTicket() {
        return this.gssTicket;
    }

    @Override
    public Map<String, String> toParameters() {
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("grant_type", GRANT_TYPE.getValue());
        parameters.put("context_id", this.contextId);
        parameters.put("gss_ticket", Base64Utils.encodeToString(this.gssTicket));
        return parameters;
    }

    public static GSSTicketGrant parse(Map<String, String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");

        GrantType grantType = GrantType.parse(ParameterMapUtils.getString(parameters, "grant_type"));
        if (grantType != GRANT_TYPE) {
            throw new ParseException("unexpected grant_type: " + grantType.getValue());
        }

        String contextIdString = ParameterMapUtils.getString(parameters, "context_id");
        byte[] gssTicketBytes = Base64Utils.decodeToBytes(ParameterMapUtils.getString(parameters, "gss_ticket"));

        return new GSSTicketGrant(contextIdString, gssTicketBytes);
    }
}