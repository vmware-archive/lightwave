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

import java.text.ParseException;

import org.apache.commons.lang3.Validate;

import com.vmware.identity.idm.PrincipalId;

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
            throw new ParseException("username is not of the format name@domain", 0);
        }

        String name = parts[0];
        String domain = parts[1];

        return new PersonUser(new PrincipalId(name, domain), tenant);
    }

    @Override
    public boolean equals(Object other) {
        boolean areEqual = false;
        if (other instanceof PersonUser) {
            PersonUser otherPersonUser = (PersonUser) other;
            areEqual =
                    super.getPrincipalId().equals(otherPersonUser.getPrincipalId()) &&
                    super.getTenant().equals(otherPersonUser.getTenant());
        }
        return areEqual;
    }

    @Override
    public int hashCode() {
        return super.getPrincipalId().hashCode() + super.getTenant().hashCode();
    }
}
