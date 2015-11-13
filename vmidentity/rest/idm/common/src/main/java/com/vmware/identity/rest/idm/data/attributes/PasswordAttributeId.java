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
 * The {@code PasswordAttributeId} enum contains a set of known attribute mappings for use with
 * {@link SchemaObjectMappingDTO}. These attributes are tied to the {@link ObjectClass#PasswordSettings}
 * object class and should be taken from {@link GroupAttributeId#getAttributeName()}.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public enum PasswordAttributeId {

    /**
     * Attribute containing the maximum age of a user account's password.
     * For example: {@code msDS-MaximumPasswordAge} in Active Directory.
     */
    MaximumPasswordAge("PasswordSettingsAttributeMaximumPwdAge"),

    /**
     * Attribute containing the duration of a user account lockout.
     * For example: {@code pwdLockoutDuration} in OpenLDAP with Password Policy Overlay support.
     */
    LockoutDuration("PwdPolicyAttributePwdLockoutDuration");

    private String attributeName;

    /**
     * Construct a {@code PasswordAttributeId} with an attribute name.
     *
     * @param attributeName the name of the attribute in IDM.
     */
    private PasswordAttributeId(String attributeName) {
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
