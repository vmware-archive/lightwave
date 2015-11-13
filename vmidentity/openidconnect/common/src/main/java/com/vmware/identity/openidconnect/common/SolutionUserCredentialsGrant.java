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

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.lang3.Validate;

import com.nimbusds.oauth2.sdk.AuthorizationGrant;
import com.nimbusds.oauth2.sdk.GrantType;
import com.nimbusds.oauth2.sdk.ParseException;

/**
 * @author Yehia Zayour
 */
public class SolutionUserCredentialsGrant extends AuthorizationGrant {
    public static final GrantType GRANT_TYPE = new GrantType("urn:vmware:grant_type:solution_user_credentials");

    public SolutionUserCredentialsGrant() {
        super(GRANT_TYPE);
    }

    @Override
    public Map<String,String> toParameters() {
        Map<String,String> parameters = new HashMap<String, String>();
        parameters.put("grant_type", GRANT_TYPE.getValue());
        return parameters;
    }

    public static SolutionUserCredentialsGrant parse(Map<String,String> parameters) throws ParseException {
        Validate.notNull(parameters, "parameters");
        String grantTypeString = parameters.get("grant_type");
        if (!GRANT_TYPE.getValue().equals(grantTypeString)) {
            throw new IllegalArgumentException("unexpected grant_type: " + grantTypeString);
        }
        return new SolutionUserCredentialsGrant();
    }
}
