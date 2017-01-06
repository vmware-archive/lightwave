/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.websso.client;

import java.io.Serializable;
import java.util.Collection;
import java.util.Collections;

/**
 * Attribute is an immutable structure which holds information about subject
 * attribute. It has a number of properties with getters.
 * 
 */
public class Attribute implements Serializable {
    private static final long serialVersionUID = 1L;
    private String name;
    private String friendlyName;
    private Collection<String> values;

    protected Attribute() {
    }

    /**
     * Construct Attribute object. This object correspond to SAML assertion
     * protocal AtributeType.
     * 
     * @param name
     *            Required. Name of attribute
     * @param friendlyName
     *            Optional. Friendlyname of the attribute
     * @param values
     *            Optional. Value of the attribute.
     */
    public Attribute(String name, String friendlyName, Collection<String> values) {

        this.name = name;
        this.friendlyName = friendlyName;
        this.values = values;
    }

    public String getName() {
        return name;
    }

    public String getFriendlyName() {
        return friendlyName;
    }

    public Collection<String> getValues() {
        return Collections.unmodifiableCollection(values);
    }

}
