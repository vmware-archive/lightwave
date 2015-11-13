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
 * The {@code UserAttributeId} enum contains a set of known attribute mappings for use with
 * {@link SchemaObjectMappingDTO}. These attributes are tied to the {@link ObjectClass#User}
 * object class and should be taken from {@link UserAttributeId#getAttributeName()}.
 *
 * @author Balaji Boggaram Ramanarayan
 */
public enum UserAttributeId {

    /**
     * Attribute containing the user's account name.
     * For example: {@code sAMAccountName} in Active Directory or {@code uid} in OpenLDAP.
     */
    AccountName("UserAttributeAccountName"),

    /**
     * Attribute containing the user's common name.
     * For example: {@code cn} in Active Directory or OpenLDAP.
     */
    CommonName("UserAttributeCommonName"),

    /**
     * Attribute containing the user's last name (family name).
     * For example: {@code sn} in Active Directory or OpenLDAP.
     */
    LastName("UserAttributeLastName"),

    /**
     * Attribute containing the user's first name (given name).
     * For example: {@code givenName} in Active Directory or OpenLDAP.
     */
    FirstName("UserAttributeFirstName"),

    /**
     * Attribute containing the user's description.
     * For example: {@code description} in Active Directory or OpenLDAP.
     */
    Description("UserAttributeDescription"),

    /**
     * Attribute containing the user's display name.
     * For example: {@code displayName} in Active Directory or OpenLDAP.
     */
    DisplayName("UserAttributeDisplayName"),

    /**
     * Attribute containing the user's email address.
     * For example: {@code mail} in Active Directory or OpenLDAP.
     */
    Email("UserAttributeEmail"),

    /**
     * Attribute containing the user's object identifier.
     * For example: {@code objectSid} in Active Directory or {@code entryUUID} in OpenLDAP.
     */
    ObjectId("UserAttributeObjectId"),

    /**
     * Attribute containing the user's User Principal Name (UPN).
     * For example: {@code userPrincipalName} in Active Directory.
     *
     * @see <a href="https://www.ietf.org/rfc/rfc0822.txt">
     *  RFC 882 - Standard for the Format of ARPA Internet Messages
     *  </a>
     */
    PrincipalName("UserAttributePrincipalName"),

    /**
     * Attribute containing the user's account control flags.
     * For example: {@code userAccountControl} in Active Directory or OpenLDAP.
     */
    AcountControl("UserAttributeAcountControl"),

    /**
     * Attribute containing the distinguished names of the groups to which the user belongs.
     * For example: {@code memberOf} in Active Directory.
     */
    MemberOf("UserAttributeMemberOf"),

    /**
     * Attribute containing the relative identifier (RID) of the primary group for the user.
     * For example: {@code primaryGroupID} in Active Directory.
     */
    PrimaryGroupId("UserAttributePrimaryGroupId"),

    /**
     * Attribute containing the date and time (UTC) at which the user account was locked out.
     * For example: {@code lockoutTime} in Active Directory.
     */
    LockoutTime("UserAttributeLockoutTime"),

    /**
     * Attribute containing the password settings applied to the user.
     * For example: {@code msDS-ResultantPSO} in Active Directory.
     */
    PasswordSettingsObject("UserAttributePasswordSettingsObject"),

    /**
     * Attribute containing the date and time (UTC) that at which the user's
     * password was last set.
     * For example: {@code pwdLastSet} in Active Directory.
     */
    PasswordLastSet("UserAttributePwdLastSet"),

    /**
     * The user's attribute which is referenced by {@link GroupAttributeId#MembersList}.
     * For example: {@code dn} in Active Directory or OpenLDAP.
     */
    GroupMembersListLink("UserAttributeGroupMembersListLink");

    private String attributeName;

    /**
     * Construct a {@code UserAttributeId} with an attribute name.
     *
     * @param attributeName the name of the attribute in IDM.
     */
    private UserAttributeId(String attributeName) {
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
