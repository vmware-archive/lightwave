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
package com.vmware.identity.rest.idm.data.attributes;

import com.vmware.identity.rest.idm.data.SchemaObjectMappingDTO;

/**
 * The {@code GroupAttributeId} enum contains a set of known attribute mappings for use with
 * {@link SchemaObjectMappingDTO}. These attributes are tied to the {@link ObjectClass#Group}
 * object class and should be taken from {@link GroupAttributeId#getAttributeName()}.
 *
 * @author Balaji Boggaram Ramanarayan
 */
public enum GroupAttributeId {

    /**
     * Attribute containing the group's account name.
     * For example: {@code sAMAccountName} in Active Directory or {@code cn} in OpenLDAP.
     */
    AccountName("GroupAttributeAccountName"),

    /**
     * Attribute containing the group's description.
     * For example: {@code description} in Active Directory or OpenLDAP.
     */
    Description("GroupAttributeDescription"),

    /**
     * Attribute containing the group's object identifier.
     * For example: {@code objectSid} in Active Directory or "entryUUID" in OpenLDAP.
     */
    ObjectId("GroupAttributeObjectId"),

    /**
     * Attribute containing the distinguished name of the groups to which this
     * group belongs.
     * For example: {@code memberOf} in Active Directory.
     */
    MemberOf("GroupAttributeMemberOf"),

    /**
     * Attribute containing the list of users that belong to this group.
     * For example: {@code member} in Active Directory or {@code uniqueMember} in OpenLDAP.
     */
    MembersList("GroupAttributeMembersList"),

    /**
     * Attribute referenced by {@link GroupAttributeId#MembersList}.
     * For example: {@code dn} in Active Directory or OpenLDAP.
     */
    ListLink("GroupAttributeGroupMembersListLink");

    private String attributeName;

    /**
     * Construct a {@code GroupAttributeId} with an attribute name.
     *
     * @param attributeName the name of the attribute in IDM.
     */
    private GroupAttributeId(String attributeName) {
        this.attributeName = attributeName;
    }

    /**
     * Get the underlying attribute name.
     *
     * @return the attribute name.
     */
    public String getAttributeName() {
        return this.attributeName;
    }
}
