/**
 *
 * Copyright 2013 VMware, Inc.  All rights reserved.
 */

package com.vmware.identity.interop;

import junit.framework.Assert;

import org.junit.Test;

import com.vmware.identity.interop.idm.IdmClientLibraryFactory;
import com.vmware.identity.interop.idm.UserInfo;

public class IdmAdaptertest {

    @Test
    public void testAuthenticateByPassword()
    {
        String pszUserName = null;
        String pszDomainName = null;
        String pszPassword = null;

        UserInfo userInfo =
                IdmClientLibraryFactory.getInstance().getLibrary().AuthenticateByPassword(
                                            pszUserName,
                                            pszDomainName,
                                            pszPassword);

        Assert.assertNotNull(userInfo);
    }

    @Test
    public void testAuthenticateBySSPI()
    {
    }
}
