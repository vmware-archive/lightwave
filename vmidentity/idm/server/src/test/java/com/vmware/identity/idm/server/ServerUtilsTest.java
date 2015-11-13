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

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.idm.DuplicateTenantException;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.PasswordExpiredException;
import com.vmware.identity.idm.UserAccountLockedException;

public class ServerUtilsTest {

    @Test
    public void getRemoteExceptionTest() {

        IDMException idmEx = new IDMException("Test exception", new IllegalArgumentException("abc"));
        DuplicateTenantException dte = new DuplicateTenantException("Test DuplicateTenantException exception", new IllegalArgumentException("abc"));

        UserAccountLockedException uace1 = new UserAccountLockedException("UserAccountLockedException exception test 1.");
        UserAccountLockedException uace2 = new UserAccountLockedException(new IllegalArgumentException("abc"));
        PasswordExpiredException pee1 = new PasswordExpiredException("PasswordExpiredException exception test 1.");
        PasswordExpiredException pee2 = new PasswordExpiredException(new IllegalArgumentException("abc"));

        IDMException ex = null;

        ex = ServerUtils.getRemoteException(idmEx);

        Assert.assertNotNull("exception should not be null", ex);
        Assert.assertTrue("exception should be IDMException", ex.getClass() == idmEx.getClass());
        Assert.assertEquals("Exception message should match", idmEx.getMessage(), ex.getMessage());
        Assert.assertNull("Cause should be null", ex.getCause());

        ex = ServerUtils.getRemoteException(dte);

        Assert.assertNotNull("exception should not be null", ex);
        Assert.assertTrue("exception should be DuplicateTenantException", ex.getClass() == dte.getClass());
        Assert.assertEquals("Exception message should match", dte.getMessage(), ex.getMessage());
        Assert.assertNull("Cause should be null", ex.getCause());

        ex = ServerUtils.getRemoteException(uace1);

        Assert.assertNotNull("exception should not be null", ex);
        Assert.assertTrue("exception should be UserAccountLockedException", ex.getClass() == uace1.getClass());
        Assert.assertEquals("Exception message should match", uace1.getMessage(), ex.getMessage());
        Assert.assertNull("Cause should be null", ex.getCause());
        Assert.assertEquals("Exception object should be the same actually...", uace1, ex);

        ex = ServerUtils.getRemoteException(uace2);

        Assert.assertNotNull("exception should not be null", ex);
        Assert.assertTrue("exception should be UserAccountLockedException", ex.getClass() == uace2.getClass());
        Assert.assertEquals("Exception message should match", uace2.getMessage(), ex.getMessage());
        Assert.assertNull("Cause should be null", ex.getCause());

        ex = ServerUtils.getRemoteException(pee1);

        Assert.assertNotNull("exception should not be null", ex);
        Assert.assertTrue("exception should be PasswordExpiredException", ex.getClass() == pee1.getClass());
        Assert.assertEquals("Exception message should match", pee1.getMessage(), ex.getMessage());
        Assert.assertNull("Cause should be null", ex.getCause());
        Assert.assertEquals("Exception object should be the same actually...", pee1, ex);

        ex = ServerUtils.getRemoteException(pee2);

        Assert.assertNotNull("exception should not be null", ex);
        Assert.assertTrue("exception should be PasswordExpiredException", ex.getClass() == pee2.getClass());
        Assert.assertEquals("Exception message should match", pee2.getMessage(), ex.getMessage());
        Assert.assertNull("Cause should be null", ex.getCause());

    }

    @Test
    public void testIsEqualsOnIgnoreCaseSucceed(){
        String str1 = "Vmware";
        String str2 = "vMwARE";
        Assert.assertTrue(ServerUtils.isEquals(str1, str2));
    }
}
