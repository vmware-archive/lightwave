/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.identity.cdc;

import junit.framework.Assert;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.net.InetAddress;
import java.util.List;

public class CdcSessionTest {
    private CdcSession _ipcSession;
    @BeforeClass
    public static void setUpBeforeClass() throws Exception {}

    @AfterClass
    public static void tearDownAfterClass() throws Exception {}

    @Before
    public void setUp() throws Exception {
        if (_ipcSession == null) {
            _ipcSession = CdcFactory.createCdcSessionViaIPC();
        }
    }

    @After
    public void tearDown() throws Exception {
        if (_ipcSession != null) {
            _ipcSession.close();
        }
        _ipcSession = null;
    }

    @Test
    public void testGetDCName() {
        String domainName = System.getProperty("domainName");
        CdcDCEntry dcEntry = _ipcSession.getAffinitizedDC(domainName, 0);
        Assert.assertNotNull(dcEntry);
    }

    /*
     * Assumes test machine is promoted DC with CDC enabled
     */
    @Test
    public void testEnumDCInfo() {
        List <DCStatusInfo> infoList = _ipcSession.enumDCStatusInfo();

        boolean dcFound = false;
        String hostname = null;
        try {
            hostname = InetAddress.getLocalHost().getHostName();
        } catch (Exception e) {}
        Assert.assertNotNull("Hostname retrieved is null", hostname);

        String fqdn = hostname + "." + System.getProperty("domainName");

        Assert.assertNotSame("DC info list expected",0, infoList.size());
        for (DCStatusInfo info: infoList) {
            if (info.dcName.equals(fqdn)) {
                dcFound = true;
                Assert.assertTrue("DC " + fqdn + " should be alive", info.isAlive);
            }

            Assert.assertNotSame(0, info.heartbeatInfo.size());
        }
        Assert.assertTrue("DC " + fqdn + " should be in DCStatusInfo list", dcFound);
    }

    @Test
    public void testEnableClientAffinity() {
        _ipcSession.enableClientAffinity();
    }

    @Test
    public void testDisableClientAffinity() {
        _ipcSession.disableClientAffinity();
    }
}