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

public class GroupDetail extends PrincipalDetail {

    /**
     * Serial version id
     */
    private static final long serialVersionUID = 8551465840255387130L;

    /**
     * Creates a {@link GroupDetails} instance
     */
    public GroupDetail() {
       super(null);
    }

    /**
     * Creates a {@link GroupDetails} instance
     *
     * @param description
     *           the description to set. cannot be <code>null</code>
     */
    public GroupDetail(String description) {
       super(description);
       // description is currently an optional field in lotus
       // check if later on need to make this a mandatory field
       // Validate.notNull(description, "Description");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Object[] getDetailFields() {
        return new Object[] { getDescription() };
    }
}
