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

package com.vmware.identity.rest.idm.data;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class AboutInfo {

    // Constants
    private static final String PRODUCT_RELEASE_LIGHTWAVE = "lightwave";
    private static final String PRODUCT_RELEASE_VSPHERE = "Vsphere";
    private static final String PRODUCT_NAME_IDM = "idm";
    private static final String STS_RPM_VSPHERE = "vmware-identity-sts";
    private static final String STS_RPM_LIGHTWAVE = "vmware-sts";

    private String release;
    private String productName;
    private String version;

    public AboutInfo() throws IOException {

        if(isLightwave()) {
            release = PRODUCT_RELEASE_LIGHTWAVE;
            version = getReleaseVersion(STS_RPM_LIGHTWAVE);
        } else {
            release = PRODUCT_RELEASE_VSPHERE;
            version = getReleaseVersion(STS_RPM_VSPHERE);
        }
        productName = PRODUCT_NAME_IDM;
    }

    private String getReleaseVersion(String rpmName) throws IOException {
        String command = "rpm -q --qf %{VERSION} " + rpmName;
        Process p = Runtime.getRuntime().exec(command);
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(
                p.getInputStream()))) {
            version = reader.readLine();
        }
        return version;
    }

    // Decision factor : The RPM name is vmware-sts in lightwave and vmware-identity-sts in Vsphere.
    // TODO : Add the product release in registry entry.
    private boolean isLightwave() throws IOException {

        boolean lightwave = true;
        Process p = Runtime.getRuntime().exec("rpm -qa vmware-sts");
        String rpmInfo;
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(
                p.getInputStream()))) {
            rpmInfo = reader.readLine();
        }
        if(rpmInfo == null || rpmInfo.isEmpty()) {
            lightwave = false;
        }
        return lightwave;
    }

    public String getRelease() {
        return release;
    }

    public String getProductName() {
        return productName;
    }

    public String getVersion() {
        return version;
    }

}
