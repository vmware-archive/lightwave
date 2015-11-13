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
package com.vmware.identity.diagnostics;

import org.apache.logging.log4j.Marker;

public enum VmEvent implements IVmEvent {

    // to be extended by a list of needed event ids
    SERVER_STARTED(1),
    SERVER_FAILED_TOSTART(2),
    USER_NAME_PWD_AUTH_FAILED(3),
    USER_NAME_PWD_AUTH_SUCCEEDED(4),
    GSS_AUTH_FAILED(5),
    GSS_AUTH_SUCCEEDED(6),
    SERVER_ERROR(7),
    CLOCK_SKEW_ERROR(8),
    TLSCLIENT_AUTH_FAILED(9),
    TLSCLIENT_AUTH_SUCCEEDED(10);

    private int _eventId;

    private VmEvent(int eventId)
    {
        _eventId = eventId;
    }
    public int getEventId()
    {
        return _eventId;
    }

    @Override
    public String getName() {
        return this.name();
    }

    @Override
    public String getEventName() {
        return this.name();
    }

    @Override
    public boolean remove(Marker arg0) {
        throw new UnsupportedOperationException("Not Implemented");
    }

    @Override
    public boolean isInstanceOf(Marker arg0) {
        if(arg0 instanceof IVmEvent)
            return true;
        else
            return false;
    }

    @Override
    public boolean isInstanceOf(String arg0) {
        if(arg0.equalsIgnoreCase(this.getEventName()))
            return true;
        else
            return false;
    }

    /*
     * Returns current Marker
     */
    @Override
    public Marker addParents(Marker... arg0) {
        throw new UnsupportedOperationException("Not Implemented");
    }

    @Override
    public Marker[] getParents() {
        throw new UnsupportedOperationException("Not Implemented");
    }

    @Override
    public boolean hasParents() {
        throw new UnsupportedOperationException("Not Implemented");
    }

    /*
     * Returns current Marker
     */
    @Override
    public Marker setParents(Marker... arg0) {
        throw new UnsupportedOperationException("Not Implemented");
    }
}
