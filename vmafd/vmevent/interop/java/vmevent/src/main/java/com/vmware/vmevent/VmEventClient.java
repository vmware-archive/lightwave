/**
 *
 * Copyright 2013 VMware, Inc.  All rights reserved.
 */

package com.vmware.vmevent;

import com.vmware.vmevent.interop.VmEventClientAdapter;

public class VmEventClient
{
    public static int addEvent(String hostName, int eventType, int eventCategory, String eventText)
    {
        return VmEventClientAdapter.addEvent(hostName, eventType, eventCategory, eventText);
    }
}
