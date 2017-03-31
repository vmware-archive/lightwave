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
package com.vmware.identity;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Locale;

import javax.servlet.http.HttpServletResponse;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.springframework.context.support.ResourceBundleMessageSource;
import org.springframework.ui.Model;
import org.springframework.validation.support.BindingAwareModelMap;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.idm.client.SAMLNames;
import com.vmware.identity.samlservice.Shared;

/**
 * MetadataController unit test
 *
 */
@Ignore // ignored due to IDM process to library change, see PR 1780279.
public class MetadataControllerTest {

    private static WebssoMetadataController controller;
    private static IDiagnosticsLogger log;
    private static String tenant;
    private static Model model;

    /**
     * @throws java.lang.Exception
     */
    @BeforeClass
    public static void setUp() throws Exception {
        log = DiagnosticsLoggerFactory.getLogger(MetadataControllerTest.class);

        controller = new WebssoMetadataController();
        ResourceBundleMessageSource ms = new ResourceBundleMessageSource();
        ms.setBasename("messages");
        controller.setMessageSource(ms);

        Shared.bootstrap();
        SharedUtils.bootstrap(false); // use real data
        tenant = ServerConfig.getTenant(0);
    }

    @AfterClass
    public static void cleanUp() throws Exception {
        SharedUtils.cleanupTenant();
    }

   /**
     * Test method for
     * {@link com.vmware.identity.MetadataController#metadata(java.util.Locale, java.lang.String, org.springframework.ui.Model, javax.servlet.http.HttpServletRequest, javax.servlet.http.HttpServletResponse)}
     * .
     *
     * @throws Exception
     */
    @Test
    public final void testMetadata() throws Exception {
        SharedUtils.bootstrap(false); // use real data

        // get destination
        String destination = ServerConfig.getTenantEntityId(tenant);
        StringBuffer sbRequestUrl = new StringBuffer();
        sbRequestUrl.append(destination);

        // print out complete GET url
        SharedUtils.logUrl(log, sbRequestUrl, null, null, null, null, null);

        model = new BindingAwareModelMap();

        // build mock response object
        StringWriter sw = new StringWriter();
        HttpServletResponse response = SharedUtils
                .buildMockResponseSuccessObject(sw,
                        Shared.METADATA_CONTENT_TYPE, false, "attachment; filename=vsphere.local.xml");

        assertMetadata(model, response);
        assertTrue(sw.getBuffer() != null && sw.getBuffer().length() > 0);
        // ensure we did not export private key
        assertFalse(sw.getBuffer().toString()
                .contains(SAMLNames.DS_RSAKEYVALUE));
        // ... and that we did export X509 cert
        assertTrue(sw.getBuffer().toString()
                .contains(SAMLNames.DS_X509CERTIFICATE));
        log.debug("Metadata response was \n" + sw.toString());
    }

    /**
     * Assert Metadata call worked correctly
     *
     * @param model
     * @param response
     * @throws IOException
     */
    private void assertMetadata(Model model, HttpServletResponse response)
            throws IOException {
        controller.metadata(Locale.US, tenant, model, response);
        assertEquals(tenant, model.asMap().get("tenant"));
        assertNull(model.asMap().get("serverTime"));
    }
}
