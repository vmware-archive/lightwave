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

import org.apache.commons.lang.Validate;

public class Group extends Principal {

    /**
     * Serial version uid
     */
    private static final long serialVersionUID = 2653572898791121811L;
    private final GroupDetail detail;

    /**
     * Constructs group principal with no alias
     *
     * @param id
     *           group id; {@code not-null} value is required
     * @param details
     *           group details; {@code null}
     */
    public Group(PrincipalId id, GroupDetail detail) {

       // No alias
       this(id, null /*no alias*/, null /*no objectSid*/, detail);
    }

    /**
     * Constructs group principal
     *
     * @param id
     *           principal id; {@code not-null} value is required
     * @param alias
     *           principal alias; {@code null} value when alias is not known or
     *           the corresponding domain has no alias specified; note that the
     *           alias should not be equal to the ID
     * @param objectId
     *           principal objectSid; {@code null} value when objectSid is an existing
     *           attribute in the identity provider
     * @param details
     *           group details; {@code null}
     */
    public Group(PrincipalId id, PrincipalId alias, String objectId, GroupDetail detail) {
       super(id, alias, objectId);

       Validate.notNull(id, "Group id");
       this.detail = detail;
    }

    @Override
    public GroupDetail getDetail() {
       return this.detail;
    }

    public String getFQDN()
    {
        return String.format("%s@%s", this.id.getName(), this.id.getDomain());
    }

    public String getNetbios()
    {
      return String.format("%s\\%s", this.id.getDomain(), this.id.getName());
    }

    public String getName()
    {
        return this.id.getName();
    }

    public String getDomain()
    {
        return this.id.getDomain();
    }
}
