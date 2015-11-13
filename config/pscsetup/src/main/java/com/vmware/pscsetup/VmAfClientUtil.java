/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.pscsetup;

import com.vmware.af.VmAfClient;

public class VmAfClientUtil
{
    private static final String AFD_SERVER = "localhost";

    public static final String getHostname()
    {
        VmAfClient client = new VmAfClient(AFD_SERVER);
        return client.getPNID();
    }

    public static final String getHostnameURL()
    {
        VmAfClient client = new VmAfClient(AFD_SERVER);
        return client.getPNIDUrl();
    }
}
