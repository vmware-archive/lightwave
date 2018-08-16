/*
 * Copyright © 2012-2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.heartbeat;

import java.util.ArrayList;
import java.util.List;

public class VmAfdHeartbeat implements AutoCloseable {
    private PointerRef hbHandle;
    private PointerRef serverHandle;
    private int port;
    private String serviceName;
    private String serverName;
    private String username;

    private static final String LOCALHOST = "__localhost__";
    private static final String LOCALUSER = "__localuser__";

    /**
     * Gets VmAfdHeartbeat hbHandle to start a heartbeat for a service
     * @param serviceName
     * @param port
     */
    public VmAfdHeartbeat(String serviceName, int port) {
        if (serviceName == null) {
            throw new IllegalArgumentException(String.format(
                    "Service Name cannot be NULL"));
        }

        this.port = port;
        this.serviceName = serviceName;
        this.serverHandle = null;
    }

    /**
     * Gets VmAfdHeartbeat hbHandle to query HeartbeatStatus of a server
     * @param server, null for localhost
     * @param username, null for localuser
     * @param password, can be null
     */
    public VmAfdHeartbeat(String server, String username, String password) {
        PointerRef pServer = new PointerRef();
        int error = 0;
        this.serverName = (server == null) ? LOCALHOST : server;
        this.username = (username == null) ? LOCALUSER : username;

        error = VmAfdHeartbeatAdapter.VmAfdOpenServerW(server, username, password, pServer);
        BAIL_ON_ERROR(error,"Error opening server '%s' for user '%s'", server, username);

        serverHandle = pServer;
        this.serviceName = null;
    }

    public HeartbeatStatus getHeartbeatStatus() {
        if (serverHandle == null) {
            BAIL_ON_ERROR(
                    VmAfdHeartbeatAdapter.ERROR_INVALID_PARAMETER,
                    "Failed to get Heartbeat Status- no server handle opened");
        }

        int error = 0;
        HeartbeatStatusNative hbStatusNative = new HeartbeatStatusNative();
        error = VmAfdHeartbeatAdapter.VmAfdGetHeartbeatStatusW(serverHandle, hbStatusNative);
        BAIL_ON_ERROR(
                error,
                "Getting Heartbeat Status failed. [Server: %s, User: %s]",
                this.serverName,
                this.username);

        return convertHbStatusNativeToHeartbeatStatus(hbStatusNative);
    }

    private HeartbeatStatus convertHbStatusNativeToHeartbeatStatus(HeartbeatStatusNative hbStatusNative) {
        if (hbStatusNative == null) {
            BAIL_ON_ERROR(VmAfdHeartbeatAdapter.ERROR_INVALID_PARAMETER, "Failed to convert Native Heartbeat status");
        }

        List<HeartbeatInfo> heartbeatInfo = new ArrayList<>();
        for (HeartbeatInfoNative info : hbStatusNative.hbInfoArr) {
            HeartbeatInfo hbInfo = new HeartbeatInfo(
                    info.serviceName,
                    info.port,
                    info.lastHeartbeat,
                    (info.isAlive != 0));
            heartbeatInfo.add(hbInfo);
        }

        return new HeartbeatStatus(heartbeatInfo, (hbStatusNative.isAlive != 0));
    }

    public void startBeating() throws VmAfdGenericException {
        if (hbHandle == null) {
            int error = 0;
            PointerRef pHandle = new PointerRef();

            error = VmAfdHeartbeatAdapter.VmAfdStartHeartBeatW(serviceName, port, pHandle);
            BAIL_ON_ERROR(error,"Error starting heartbeat for Service '%s' for port '%d'", serviceName, port);

            hbHandle = pHandle;
        }
    }

    public void stopBeating() {
        if (hbHandle != null) {
            VmAfdHeartbeatAdapter.VmAfdStopHeartbeat(hbHandle);
        }
        hbHandle = null;
    }

    protected void finalize() throws Throwable {
        try {
            stopBeating();
        } finally {
            super.finalize();
        }
    }

    @Override
    public void close() throws Exception {
        this.stopBeating();
    }

    private static void BAIL_ON_ERROR(final int error, final String format, Object...vargs) {
        switch (error) {
            case 0:
                break;
            default:
                throw new VmAfdGenericException(String.format(format, vargs), error);
        }
    }
}