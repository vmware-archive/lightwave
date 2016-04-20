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

package com.vmware.identity.openidconnect.common;

import java.net.URI;
import java.util.HashMap;
import java.util.Map;

import org.junit.Assert;
import org.junit.Test;

/**
 * @author Yehia Zayour
 */
public class URIUtilsTest {
    @Test
    public void testParseSuccessOnIP() throws ParseException {
        String uriString = "https://10.11.12.13/file?key=value";
        URI uri = URIUtils.parseURI(uriString);
        Assert.assertEquals("uriString", uriString, uri.toString());
    }

    @Test
    public void testParseSuccessOnFQDN() throws ParseException {
        String uriString = "https://identity.vmware.com";
        URI uri = URIUtils.parseURI(uriString);
        Assert.assertEquals("uriString", uriString, uri.toString());
    }

    @Test
    public void testParseURISuccessOnLocalHostname() throws ParseException {
        String uriString = "https://hostname";
        URI uri = URIUtils.parseURI(uriString);
        Assert.assertEquals("uriString", uriString, uri.toString());
    }

    @Test
    public void testParseFailOnInvalidScheme() {
        String uriString = "http://identity.vmware.com"; // should be https
        try {
            URIUtils.parseURI(uriString);
            Assert.fail("expecting ParseException");
        } catch (ParseException e) {
            Assert.assertEquals("e.getMessage()", "uri is not a valid https url", e.getMessage());
        }
    }

    @Test
    public void testChangePathComponent() throws ParseException {
        URI oldUri = URIUtils.parseURI("https://identity.vmware.com/file?p1=v1&p2=v2");
        URI newUri = URIUtils.changePathComponent(oldUri, "/path/to/newfile");
        Assert.assertEquals("newUri", "https://identity.vmware.com/path/to/newfile?p1=v1&p2=v2", newUri.toString());
    }

    @Test
    public void testChangeHostComponent() throws ParseException {
        URI oldUri = URIUtils.parseURI("https://identity.vmware.com/file?p1=v1&p2=v2");
        URI newUri = URIUtils.changeHostComponent(oldUri, "sso.vmware.com");
        Assert.assertEquals("newUri", "https://sso.vmware.com/file?p1=v1&p2=v2", newUri.toString());
    }

    @Test
    public void testChangeSchemeComponent() throws ParseException {
        URI oldUri = URIUtils.parseURI("https://identity.vmware.com/file?p1=v1&p2=v2");
        URI newUri = URIUtils.changeSchemeComponent(oldUri, "http");
        Assert.assertEquals("newUri", "http://identity.vmware.com/file?p1=v1&p2=v2", newUri.toString());
    }

    @Test
    public void testAppendQueryParameter() throws ParseException {
        URI oldUri = URIUtils.parseURI("https://identity.vmware.com/file?p1=v1&p2=v2");
        URI newUri = URIUtils.appendQueryParameter(oldUri, "p3", "v3");
        Assert.assertEquals("newUri", "https://identity.vmware.com/file?p1=v1&p2=v2&p3=v3", newUri.toString());
    }

    @Test
    public void testAppendQueryParameters() throws ParseException {
        URI oldUri = URIUtils.parseURI("https://identity.vmware.com/file?p1=v1&p2=v2");
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("p3", "v3");
        URI newUri = URIUtils.appendQueryParameters(oldUri, parameters);
        Assert.assertEquals("newUri", "https://identity.vmware.com/file?p1=v1&p2=v2&p3=v3", newUri.toString());
    }

    @Test
    public void testAppendFragmentParameters() throws ParseException {
        URI oldUri = URIUtils.parseURI("https://identity.vmware.com/file");
        Map<String, String> parameters = new HashMap<String, String>();
        parameters.put("p1", "v1");
        URI newUri = URIUtils.appendFragmentParameters(oldUri, parameters);
        Assert.assertEquals("newUri", "https://identity.vmware.com/file#p1=v1", newUri.toString());
    }
}