/* **********************************************************************
 * Copyright 2018 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.identity.heartbeat;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.Assert;

public class HeartbeatAdapterTest {
    private static PointerRef _pHeartbeatHandle;

    @BeforeClass
    public static void setUpBeforeClass() throws Exception {}

    @AfterClass
    public static void tearDownAfterClass() throws Exception {}

    @Before
    public void setUp() throws Exception {}

    @After
    public void tearDown() throws Exception {}

    @Test
    public void testStartStopHeartbeat() {
        try {
            VmAfdHeartbeat vmAfdHeartbeat = new VmAfdHeartbeat("JavaUnitTest", 7566);
            vmAfdHeartbeat.startBeating();
            vmAfdHeartbeat.stopBeating();
        } catch (Exception e) {
            Assert.fail(String.format("Failed to test heartbeat start/stop: %s", e.getMessage()));
            e.printStackTrace();
        }
    }

    @Test
    public void testGetHeartbeat() {
        VmAfdHeartbeat vmAfdHeartbeat = new VmAfdHeartbeat(null, null, null);
        HeartbeatStatus status = vmAfdHeartbeat.getHeartbeatStatus();
        Assert.assertTrue(status.isAlive);
        Assert.assertTrue((status.heartbeatInfo.size() > 0));
        for (HeartbeatInfo info : status.heartbeatInfo) {
            System.out.println(String.format("Service [%s], Alive: %b", info.serviceName, info.isAlive));
            Assert.assertTrue(info.isAlive);
        }
    }

}