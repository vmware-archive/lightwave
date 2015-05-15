/**
 *
 * Copyright 2013 VMware, Inc.  All rights reserved.
 */

package com.vmware.vmevent;

import org.junit.Assert;
import org.junit.Test;

public class VmEventLogClientTest
{
    private static String _hostname = "localhost";
    private static int _eventType = 1;
    private static int _eventCategory = 1;
    private static String _eventText = "Test Event";

    @Test
    public void testAddEvent()
    {
        int result = VmEventClient.addEvent(_hostname, _eventType, _eventCategory, _eventText);
        Assert.assertEquals(result, 0);
    }
}
