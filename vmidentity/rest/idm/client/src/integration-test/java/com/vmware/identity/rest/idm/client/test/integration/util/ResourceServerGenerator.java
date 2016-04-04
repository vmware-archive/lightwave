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
package com.vmware.identity.rest.idm.client.test.integration.util;

import java.util.Arrays;
import java.util.HashSet;

import com.vmware.identity.rest.idm.data.ResourceServerDTO;

/**
 * @author Yehia Zayour
 */
public class ResourceServerGenerator {

    public static ResourceServerDTO generateResourceServer() {
        return new ResourceServerDTO.Builder().
                withName("rs_integration_test").
                withGroupFilter(new HashSet<String>(Arrays.asList("domain\\group1", "domain\\group2"))).build();
    }
}
