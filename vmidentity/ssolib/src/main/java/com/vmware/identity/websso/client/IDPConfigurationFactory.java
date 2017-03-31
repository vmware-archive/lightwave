/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client;

import java.security.cert.X509Certificate;
import java.util.List;

/**
 * @author schai
 *
 */
public class IDPConfigurationFactory {

    private static final int DEFAULT_CLOCK_TOLERANCE_SEC = 600; //in seconds

	/**
     * create an instance of IDPConfiguration that not using site-affinity.
     *
     * @param alias
     * @param entityID
     * @param signingCertificate
     * @param nameIDFormats
     * @param singleSignOnServices
     * @param singleLogoutServices
     * @param clockTolerance
     * @return IDPConfiguration
     */
    public static IDPConfiguration createIDPConfigurationWithoutSiteAffinity(
            String alias, String entityID, X509Certificate signingCertificate,
            List<String> nameIDFormats, List<SingleSignOnService> singleSignOnServices,
            List<SingleLogoutService> singleLogoutServices)
    {
        return new IDPConfiguration(alias, entityID, signingCertificate,
                nameIDFormats, singleSignOnServices, singleLogoutServices, DEFAULT_CLOCK_TOLERANCE_SEC,
                false);
    }

    /**
     * create an instance of 'site-affinitized' IDPConfiguration
     *
     * @param alias
     * @param entityID
     * @param signingCertificate
     * @param nameIDFormats
     * @param singleSignOnServices
     * @param singleLogoutServices
     * @param clockTolerance
     * @return IDPConfiguration
     */
    public static IDPConfiguration createAffinitizedIDPConfiguration(String alias, String entityID, X509Certificate signingCertificate,
            List<String> nameIDFormats, List<SingleSignOnService> singleSignOnServices,
            List<SingleLogoutService> singleLogoutServices)
    {
        return new IDPConfiguration(alias, entityID,
                signingCertificate,
                nameIDFormats, singleSignOnServices, singleLogoutServices, DEFAULT_CLOCK_TOLERANCE_SEC,
                true);
    }

}
