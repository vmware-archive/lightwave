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

package com.vmware.identity.installer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class ReleaseUtil {

    // Decision factor : The RPM name is vmware-sts in lightwave and vmware-identity-sts in Vsphere.
    // TODO : Add the product release in registry entry.
    public static boolean isLightwave() throws IOException {

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
}
