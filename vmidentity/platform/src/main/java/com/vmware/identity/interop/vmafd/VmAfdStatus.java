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

public enum VmAfdStatus
{
    VMAFD_STATUS_UNKNOWN(0),
    VMAFD_STATUS_INITIALIZING(1),
    VMAFD_STATUS_PAUSED(2),
    VMAFD_STATUS_RUNNING(3),
    VMAFD_STATUS_STOPPING(4),
    VMAFD_STATUS_STOPPED(5);

    int _status;

    private VmAfdStatus(int status)
    {
        this._status = status;
    }

    public int getCode()
    {
        return this._status;
    }
}
