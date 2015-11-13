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
 * The {@code ObjectClass} enum contains a set of known object classes for use with
 * {@link SchemaObjectMappingDTO}. These object classes are tied to a set of attributes
 * for the specific class and should be taken from {@link ObjectClass#getAttributeName()}.
 */
public enum ObjectClass {

    /**
     * An object representing a user.
     * For example: {@code user} in Active Directory or {@code inetOrgPerson} in OpenLDAP.
     *
     * @see UserAttributeId
     */
    User("ObjectIdUser"),

    /**
     * An object representing a group.
     * For example: {@code group} in Active Directory or {@code groupOfUniqueNames} in OpenLDAP.
     *
     * @see GroupAttributeId
     */
    Group("ObjectIdGroup"),

    /**
     * An object representing the password settings for user accounts.
     * For example: {@code msDS-PasswordSettings} in Active Directory.
     *
     * @see PasswordAttributeId
     */
    PasswordSettings("ObjectIdPasswordSettings"),

    /**
     * An object representing a domain.
     * For example: {@code domain} in Active Directory.
     *
     * @see DomainAttributeId
     */
    Domain("ObjectIdDomain");

    private String attributeName;

    /**
     * Construct a {@code ObjectClass} with an attribute name.
     *
     * @param attributeName the name of the attribute in IDM.
     */
    private ObjectClass(String attributeName) {
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
