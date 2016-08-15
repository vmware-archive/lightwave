/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.identity.configure;

import com.vmware.af.VmAfClient;

public class VmAfClientUtil {
    private static final String AFD_SERVER = "localhost";

    public static final String getHostname() {
        VmAfClient client = new VmAfClient(AFD_SERVER);
        return client.getPNID();
    }

    public static final String getHostnameURL() {
        VmAfClient client = new VmAfClient(AFD_SERVER);
        return client.getPNIDUrl();
    }

    public static final int getRHTTPProxyPort() {
        VmAfClient client = new VmAfClient(AFD_SERVER);
        return client.getRHTTPProxyPort();
    }

    public static void setReverseProxyPort(int reverseProxyPort) {
        VmAfClient client = new VmAfClient(AFD_SERVER);
        client.setRHTTPProxyPort(reverseProxyPort);
    }
}
