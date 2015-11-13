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
import java.util.Arrays;

public abstract class PrincipalDetail implements Serializable{

    /**
     * Serial version uid
     */
    private static final long serialVersionUID = -4232496678462655646L;
    /**
     * A descriptive name of the user
     */
    public String description;

    /**
     * Create principal details
     *
     * @param description
     *            the description to set; accepts <code>null</code> value
     */
    protected PrincipalDetail(String desc) {
        this.description = desc;
    }

    /**
     * Return the description
     *
     * @return the description or <code>null</code> value
     */
    public String getDescription() {
        return description;
    }

    /**
     * @param description
     *            the user's description
     */
    public void setDescription(String description) {
        this.description = description;
    }

    /**
     * {@inheritDoc}
     * Note: Currently, we have no use cases and code will actually do equals check
     * for the detail classes defined here in the IDM layer, such as SolutionDetail, PersonDetail
     * and GroupDetail. All comparison should be done in the admin layer, which has its own corresponding
     * detail classes. We implement here for completeness in case in the future
     * there will be cases that will need to compare two detail object defined in the IDM layer.
     */
    @Override
    public boolean equals(Object obj) {
        if (obj == this) {
            return true;
        }

        if (obj == null || this.getClass() != obj.getClass()) {
            return false;
        }

        PrincipalDetail other = (PrincipalDetail) obj;
        return Arrays.equals(getDetailFields(), other.getDetailFields());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int hashCode() {
        return Arrays.hashCode(getDetailFields());
    }

    /**
     * Obtain all the fields for this detail object.
     *
     * @return all detail fields for this object
     */
    protected abstract Object[] getDetailFields();
}
