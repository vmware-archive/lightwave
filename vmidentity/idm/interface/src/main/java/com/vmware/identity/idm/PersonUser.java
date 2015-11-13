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

import org.apache.commons.lang.Validate;

/**
 * Person user type representation.
 */
public final class PersonUser extends Principal implements Serializable {

    /**
     * Serial version uid
     */
    private static final long serialVersionUID = 4460349861596134343L;

    private final PersonDetail detail;
    private final boolean disabled;
    private final boolean locked;

    /**
     * Constructs person user type of principal with no alias.
     *
     * @param id
     *            user id; {@code not-null} value is required
     * @param details
     *            user detail; {@code not-null} value is required
     * @param disabled
     *            whether the user is disabled
     * @param locked
     *            whether the user is locked
     */
    public PersonUser(PrincipalId id, PersonDetail detail,
            boolean disabled, boolean locked) {

        this(id, null /* no alias */, null /* no objectSid */, detail, disabled, locked);
    }

    /**
     * Constructs person user type of principal.
     *
     * @param id
     *            principal id; {@code not-null} value is required
     * @param alias
     *            principal alias; {@code null} value when alias is not known or
     *            the corresponding domain has no alias specified; note that the
     *            alias should not be equal to the ID
     * @param objectSid
     *           principal objectSid; {@code null} value when objectSid is an existing
     *           attribute in the identity provider
     * @param details
     *            user details; {@code not-null} value is required
     * @param disabled
     *            whether the user is disabled
     * @param locked
     *            whether the user is locked
     */
    public PersonUser(PrincipalId id, PrincipalId alias, String objectSid,
            PersonDetail detail, boolean disabled, boolean locked) {

         super(id, alias, objectSid);

        Validate.notNull(detail, "NUll PersonDetail");
        Validate.notNull(id, "Null PrincipalId");
        this.disabled = disabled;
        this.locked = locked;
        this.detail = detail;
    }

    @Override
    public PersonDetail getDetail() {
        return this.detail;
    }

    /**
     * Specified whether the user is disabled.
     * <p>
     * Disabled users cannot acquire tokens from STS and their existing tokens
     * cannot be validated.
     *
     * @return whether the user is disabled
     */
    public boolean isDisabled() {
        return this.disabled;
    }

    /**
     * Specified whether the user is locked.
     * <p>
     * Users become locked on a number of unsuccessful authentication attempts
     * as specified at {@link LockoutPolicy}. Locked users cannot acquire token
     * from STS ( except in some cases by SSPI ) but their existing tokens can
     * still be validated.
     *
     * @return whether the user is locked
     */
    public boolean isLocked() {
        return this.locked;
    }
}
