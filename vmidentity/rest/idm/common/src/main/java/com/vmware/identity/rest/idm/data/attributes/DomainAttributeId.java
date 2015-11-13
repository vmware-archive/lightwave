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
 * The {@code DomainAttributeId} enum contains a set of known attribute mappings for use with
 * {@link SchemaObjectMappingDTO}. These attributes are tied to the {@link ObjectClass#Domain}
 * object class and should be taken from {@link DomainAttributeId#getAttributeName()}.
 *
 * @author Balaji Boggaram Ramanarayan
 */
public enum DomainAttributeId {

    /**
     * Attribute for the maximum amount of time a password is valid.
     * For example: {@code maxPwdAge} in Active Directory.
     */
    MaximumPasswordAge("DomainAttributeMaxPwdAge");

    private String attributeName;

    /**
     * Construct a {@code DomainAttributeId} with an attribute name.
     *
     * @param attributeName the name of the attribute in IDM.
     */
    private DomainAttributeId(String attributeName) {
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
