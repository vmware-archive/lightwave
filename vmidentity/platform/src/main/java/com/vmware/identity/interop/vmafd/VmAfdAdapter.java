/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
package com.vmware.identity.interop.vmafd;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.ptr.IntByReference;
import com.vmware.identity.interop.NativeAdapter;

class VmAfdAdapter extends NativeAdapter implements IVmAfdClient
{
    private interface VmAfdClientLibrary extends Library
    {
        VmAfdClientLibrary INSTANCE =
                (VmAfdClientLibrary) Native.loadLibrary(
                Platform.isWindows()?"libvmafdclient.dll":"libvmafdclient",
                        VmAfdClientLibrary.class);

        //DWORD
        //VmAfdGetStatusA(
        //    PCSTR         pszServerName,  /* IN     OPTIONAL */
        //    PVMAFD_STATUS pStatus         /* IN OUT          */
        //    );
        //
        int
        VmAfdGetStatusA(
            String pszServerName,
            IntByReference pStatus
            );
    }

    @Override
    public int GetStatus()
    {
        IntByReference pStatus = new IntByReference(0);
        checkError(VmAfdClientLibrary.INSTANCE.VmAfdGetStatusA("localhost", pStatus), "Failed to retrieve VmAfd Status.");
        return pStatus.getValue();
    }

    private static void checkError(int errorCode, String msg)
    {
        if( errorCode != 0 )
        {
            throw new VmAfdException(errorCode, msg);
        }
    }
}
