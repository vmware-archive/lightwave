/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * *********************************************************************/
package com.vmware.identity.rest.core.client.test.integration.util;

public class UsernameUtil {

    public static String getUsernameFromUPN(String upn) {
        return upn.split("@")[0];
    }

    public static String getDomainFromUPN(String upn) {
        return upn.split("@")[1];
    }

    public static String buildUPN(String name, String domain) {
        return name + "@" + domain;
    }

}
