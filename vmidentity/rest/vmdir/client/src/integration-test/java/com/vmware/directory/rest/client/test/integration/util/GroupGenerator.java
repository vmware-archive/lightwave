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
package com.vmware.directory.rest.client.test.integration.util;

import com.vmware.directory.rest.common.data.GroupDTO;
import com.vmware.directory.rest.common.data.GroupDetailsDTO;

public class GroupGenerator {

    public static GroupDTO generateGroup(String name, String domain, String description) {
        return new GroupDTO.Builder()
            .withName(name)
            .withDomain(domain)
            .withDetails(getGroupDetails(description))
            .build();
    }

    private static GroupDetailsDTO getGroupDetails(String description) {
        return new GroupDetailsDTO.Builder()
            .withDescription(description)
            .build();
    }

}
