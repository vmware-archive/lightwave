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
package com.vmware.identity.rest.core.server.test.util;

import static org.junit.Assert.assertEquals;

import org.junit.Test;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.rest.core.server.exception.client.BadRequestException;
import com.vmware.identity.rest.core.server.util.PrincipalUtil;

public class PrincipalUtilTest {

    private static final String PRINCIPAL_NAME = "test";
    private static final String PRINCIPAL_DOMAIN = "vmware.local";
    private static final String PRINCIPAL_UPN = PRINCIPAL_NAME + "@" + PRINCIPAL_DOMAIN;
    private static final String PRINCIPAL_NETBIOS = PRINCIPAL_DOMAIN + "\\" + PRINCIPAL_NAME;

    @Test
    public void testFromNameInUPN() {
        PrincipalId idInUPN = PrincipalUtil.fromName(PRINCIPAL_UPN);
        assertEquals(PRINCIPAL_NAME, idInUPN.getName());
        assertEquals(PRINCIPAL_DOMAIN, idInUPN.getDomain());
    }

    @Test
    public void testFromNameInNetBiosFormat() {
        PrincipalId idInNB = PrincipalUtil.fromName(PRINCIPAL_NETBIOS);
        assertEquals(PRINCIPAL_NAME, idInNB.getName());
        assertEquals(PRINCIPAL_DOMAIN, idInNB.getDomain());
    }

    @Test(expected = BadRequestException.class)
    public void testFromNameWithInvalidUPNFormat() {
        String invalidformat = "test@tst@local";
        PrincipalUtil.fromName(invalidformat);
    }

    @Test(expected = BadRequestException.class)
    public void testFromNameWithInvalidNetBiosFormat() {
        String invalidformat = "test\\\tst\\local";
        PrincipalUtil.fromName(invalidformat);
    }
}
