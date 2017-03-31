/*
 *
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
 *
 */
package com.vmware.identity.idm.server;

import org.apache.commons.daemon.Daemon;
import org.apache.commons.daemon.DaemonContext;
import org.apache.commons.daemon.DaemonInitException;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.diagnostics.VmEvent;
/**
*  This is used only in context of Vsphere. 
*  Responsible for starting IDM service which currently exists only in Vsphere and not in Lightwave. This
*  class resides in Lightwave code base only for the purpose of code sync. This shall be removed eventually
*  on unblocking potential blockers in removal of IDM service.
**/

public class IdmServer implements Daemon {

    private static final int ERROR_SERVICE_NOT_ACTIVE = 0x462; // 1062
    private static final int ERROR_FAIL_SHUTDOWN = 0x15F; // 351

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmServer.class);
    private static Object serviceLock = new Object();

    public static void main(String[] args) throws Exception {
        startserver(args);
    }

    /**
     * Start the IDM server and continue running until
     * {@link #stopserver(String[])} is called.
     * <p>
     * Note: Used when deployed as a Windows service with Apache Commons Procrun.
     * </p>
     *
     * @param args arguments for the server.
     */
    public static void startserver(String[] args) {
        try {
            initialize();

            synchronized (serviceLock) {
                logger.debug("IDM Server is ready and waiting...");
                serviceLock.wait();
            }
        } catch (Throwable t) {
            try {
                shutdown();
            } catch (Throwable t2) {
                // Do nothing since we were shutting down anyway
            } finally {
                logger.debug("Hard exiting service...");
                System.exit(ERROR_SERVICE_NOT_ACTIVE);
            }
        }
    }

    /**
     * Stop the IDM server when it has been started with
     * {@link #startserver(String[])}.
     * <p>
     * Note: Used when deployed as a Windows service with Apache Commons Procrun.
     * </p>
     *
     * @param args arguments from prunsrv.
     */
    public static void stopserver(String[] args) {
        try {
            shutdown();

            synchronized(serviceLock) {
                logger.debug("Notifying service lock...");
                serviceLock.notifyAll();
            }
        } catch (Throwable t) {
            logger.debug("Hard exiting the service...");
            System.exit(ERROR_FAIL_SHUTDOWN);
        }
    }

    /**
     * Frees any resources allocated by this {@code Daemon} such as file
     * descriptors or sockets.
     * <p>
     * Note: Used when deployed on Linux as a daemon.
     * </p>
     */
    @Override
    public void destroy() {
    }

    /**
     * Initializes this {@code Daemon} instance.
     * <p>
     * Note: Used when deployed on Linux as a daemon.
     * </p>
     *
     * @param context context to initialize the daemon.
     */
    @Override
    public void init(DaemonContext context) throws DaemonInitException, Exception {
    }

    /**
     * Starts the operation of this {@code Daemon} instance.
     * <p>
     * Note: Used when deployed on Linux as a daemon.
     * </p>
     */
    @Override
    public void start() throws Exception {
        initialize();
    }

    /**
     * Stops the operation of this {@code Daemon} instance.
     * <p>
     * Note: Used when deployed on Linux as a daemon.
     * </p>
     */
    @Override
    public void stop() throws Exception {
        shutdown();
    }

    /**
     * Initialize the IDM service.
     *
     * @throws Exception when something goes wrong with initialization.
     */
    private static void initialize() throws Exception {
        try (IDiagnosticsContextScope diagCtxt = DiagnosticsContextFactory.createContext("IDM Startup", "")){
            logger.info("Starting IDM Server...");
            logger.info(VmEvent.SERVER_STARTED, "IDM Server has started");
        } catch (Throwable t) {
            logger.error(VmEvent.SERVER_FAILED_TOSTART, "IDM Server has failed to start", t);
            throw t;
        }
    }

    /**
     * Shutdown the IDM service.
     *
     * @throws Exception when something goes wrong with shutdown.
     */
    private static void shutdown() throws Exception {
        try(IDiagnosticsContextScope diagCtxt = DiagnosticsContextFactory.createContext("IDM Shutdown", "")) {
            logger.info("Stopping IDM Server...");
            logger.info("IDM Server has stopped");
        } catch (Throwable t) {
            logger.error(VmEvent.SERVER_ERROR, "IDM Server failed to stop", t);
            throw t;
        }
    }
}
