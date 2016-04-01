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

package com.vmware.identity.openidconnect.server;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.openidconnect.common.ParseException;

/**
 * @author Yehia Zayour
 */
public class PersonUser extends User {
    public PersonUser(PrincipalId principalId, String tenant) {
        super(principalId, tenant);
    }

    public static PersonUser parse(String username, String tenant) throws ParseException {
        Validate.notEmpty(username, "username");
        Validate.notEmpty(tenant, "tenant");

        String[] parts = username.split("@");
        if (parts.length != 2) {
            throw new ParseException("username is not of the format name@domain");
        }

        String name = parts[0];
        String domain = parts[1];

        return new PersonUser(new PrincipalId(name, domain), tenant);
    }
}