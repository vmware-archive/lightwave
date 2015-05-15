/**
 *
 * Copyright 2013 VMware, Inc.  All rights reserved.
 */

package com.vmware.vmevent.interop;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.WString;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.vmevent.VmEventClientNativeException;

public class VmEventClientAdapter extends NativeAdapter
{
    static String _endPoint = "2020";

    public interface VmEventClientLibrary extends Library
    {
        VmEventClientLibrary INSTANCE =
                (VmEventClientLibrary) Native.loadLibrary(
                                                "vmeventclient",
                                                VmEventClientLibrary.class);

        int
        EventLogAdd(
            String         pszServerName,
            String         pszServerEndPoint,
            int            eventID,
            int            eventType,
            String         eventText
            );
    }

    public static int addEvent(String hostname, int eventType, int eventCategory, String eventText)
    {
        PointerByReference ppszDomain = new PointerByReference();

        try
        {
            int errCode = VmEventClientLibrary.INSTANCE.EventLogAdd(
                                                        hostname,
                                                        _endPoint,
                                                        eventType,
                                                        eventCategory,
                                                        eventText);
            if (errCode != 0)
            {
                throw new VmEventClientNativeException(errCode);
            }
            return errCode;
        }
        finally
        {
        }
    }
}
