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

public abstract class Principal implements Serializable{

    /**
     * Serial version uid
     */
    private static final long serialVersionUID = 1479846010334203684L;
    protected PrincipalId id;
    protected PrincipalId alias;
    protected String objectId;

    /**
     * Constructs principal by primary and alternative IDs
     *
     * @param id
     *           principal id; {@code not-null} value is required
     * @param alias
     *           principal alias; {@code null} value when alias is not known or
     *           the corresponding domain has no alias specified
     * @param objectId
     *           principal objectId (could be objectSid Or EntryUUID); {@code null} value when objectId is an existing
     *           attribute in the identity provider
     */
    protected Principal(PrincipalId pid, PrincipalId palias, String objectId) {
       this.id = pid;
       this.alias = palias;
       this.objectId = objectId;
    }

    /**
     * @return the id; {@code not-null} value
     */
    public final PrincipalId getId() {
       return this.id;
    }

    /**
     * @return the alias; might be {@code null}
     */
    public final PrincipalId getAlias() {
       return this.alias;
    }

    /**
     * @return the objectSid; might be {@code null}
     */
    public final String getObjectId() {
       return this.objectId;
    }

    /**
     * @return the details; {@code not-null} value
     */
    public abstract PrincipalDetail getDetail();

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean equals(Object obj) {
       if (obj == this) {
          return true;
       }

       if (obj == null || getClass() != obj.getClass()) {
          return false;
       }

       Principal other = (Principal) obj;
       return this.id.equals(other.id);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final int hashCode() {
       return this.id.hashCode();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final String toString() {

       return String.format("IdmPrincipal: %s, %s, details {%s}", getClass()
          .getSimpleName(), this.id, getDetail());
    }

}
