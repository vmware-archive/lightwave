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
package com.vmware.identity.rest.idm.server.test.integration.resources;

import java.security.InvalidParameterException;

import org.junit.Rule;
import org.junit.rules.MethodRule;
import org.junit.rules.TestWatchman;
import org.junit.runners.model.FrameworkMethod;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.rest.idm.server.test.integration.util.CertificateResourceHelper;
import com.vmware.identity.rest.idm.server.test.integration.util.GroupHelper;
import com.vmware.identity.rest.idm.server.test.integration.util.IdentityProviderHelper;
import com.vmware.identity.rest.idm.server.test.integration.util.RelyingPartyHelper;
import com.vmware.identity.rest.idm.server.test.integration.util.SolutionUserHelper;
import com.vmware.identity.rest.idm.server.test.integration.util.TenantHelper;
import com.vmware.identity.rest.idm.server.test.integration.util.UserHelper;

/**
 * Base class for running integration tests. It beholds responsibilities such as lazy initialization of idmclient, resource helpers etc
 *
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 *     
 */
public class TestBase {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(TestBase.class);

    public static final String PROPERTY_HOST = "host";
    public static final String DEFAULT_TENANT = "vsphere.local";
    public static final String DEFAULT_PROVIDER = "localos";
    public static final String DEFAULT_SYSTEM_DOMAIN = "vsphere.local";
    public static final String VSPHERE_LOCAL = "vsphere.local";

    protected CasIdmClient idmClient;
    protected UserHelper userHelper;
    protected GroupHelper groupHelper;
    protected SolutionUserHelper solutionUserHelper;
    protected TenantHelper tenantHelper;
    protected CertificateResourceHelper certificateResourceHelper;
    protected IdentityProviderHelper providerHelper;
    protected RelyingPartyHelper relyingPartyHelper;
    private final String targetHost;

    public TestBase() {
        this.targetHost = System.getProperty(PROPERTY_HOST);
        if (targetHost == null || targetHost.length() == 0) {
            String errMessage = "Failed to retrieve hostname or IP address of sso server to run integration tests against";
            log.error(errMessage);
            throw new InvalidParameterException(errMessage);
        }
        idmClient = new CasIdmClient(targetHost);
        userHelper = new UserHelper(idmClient);
        groupHelper = new GroupHelper(idmClient);
        solutionUserHelper = new SolutionUserHelper(idmClient);
        tenantHelper = new TenantHelper(idmClient);
        certificateResourceHelper = new CertificateResourceHelper(idmClient);
        providerHelper = new IdentityProviderHelper(idmClient);
        relyingPartyHelper = new RelyingPartyHelper(idmClient);
    }

    /**
     * Hostname or IPaddress of Single Sign On server used as target to run integration tests
     * against
     */
    public String getTargetHost() {
        return targetHost;
    }

    @Rule
    public MethodRule watchman = new TestWatchman() {
        @Override
        public void starting(FrameworkMethod method) {
            System.out.print("Running test: " + method.getName());
        }

        @Override
        public void succeeded(FrameworkMethod method) {
            System.out.println(", Result : success");
        }
    };
}
