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

package com.vmware.identity.idm.server;

import static org.junit.Assert.*;
import junit.framework.Assert;

import com.vmware.identity.idm.ValidateUtil;

import org.junit.Test;

public class ValidateUtilsTest
{
    @Test
    public void testValidateBaseDN()
    {
        String dnString = "cn=Administrator,cn=users,dc=vsphere,dc=local";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "CN=Administrator,CN=Users,DC=ssolabs6,DC=com";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "CN=com";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "o=apple.com,o=email";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "O=apple.com,OU=ABC";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "o   = apple.com, o   = email";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "apple.com o=email";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "1.3.6.1.4.1.1466.1.2.3=ADMIN, 1.3.6.1.4.1.1466.1.2.3=users";
        Assert.assertTrue(String.format("dn=[%s] is valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "";
        Assert.assertFalse(String.format("dn=[%s] is NOT valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "abc@def.com";
        Assert.assertFalse(String.format("dn=[%s] is NOT valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "abc ,";
        Assert.assertFalse(String.format("dn=[%s] is NOT valid", dnString), ValidateUtil.isValidDNFormat(dnString));

        dnString = "def";
        Assert.assertFalse(String.format("dn=[%s] is NOT valid", dnString), ValidateUtil.isValidDNFormat(dnString));
    }
}
