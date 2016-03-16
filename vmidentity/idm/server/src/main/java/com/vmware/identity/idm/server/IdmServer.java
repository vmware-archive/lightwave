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

import java.rmi.Naming;
import java.rmi.RMISecurityManager;
import java.rmi.registry.LocateRegistry;
import java.rmi.registry.Registry;
import java.rmi.server.RMIClientSocketFactory;
import java.rmi.server.RMIServerSocketFactory;
import java.rmi.server.RMISocketFactory;
import java.rmi.server.UnicastRemoteObject;

import org.apache.commons.daemon.Daemon;
import org.apache.commons.daemon.DaemonContext;
import org.apache.commons.daemon.DaemonInitException;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.diagnostics.VmEvent;
import com.vmware.identity.heartbeat.VmAfdHeartbeat;
import com.vmware.identity.idm.ILoginManager;
import com.vmware.identity.idm.Tenant;
import com.vmware.identity.idm.server.config.ConfigStoreFactory;
import com.vmware.identity.idm.server.config.IConfigStoreFactory;
import com.vmware.identity.idm.server.provider.IProviderFactory;
import com.vmware.identity.idm.server.provider.ProviderFactory;
import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.performanceSupport.PerfDataSink;

public class IdmServer implements Daemon {

    private static final String ALLOW_REMOTE_PROPERTY = "vmware.idm.allow.remote";

    private static final int ERROR_SERVICE_NOT_ACTIVE = 0x462; // 1062
    private static final int ERROR_FAIL_SHUTDOWN = 0x15F; // 351

    private static final String IDENTITY_MANAGER_BIND_NAME = "IdentityManager";
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmServer.class);
    private static final int reportHitCount = 100; // Trigger report by number of entries
    private static final int reportInterval = 5; // Trigger report by time interval in minutes
    private static Registry registry;
    private static Object serviceLock = new Object();
    private static IPerfDataSink perfDataSink;
    private static IdentityManager manager;
    private static ILoginManager loginManager;
    private static VmAfdHeartbeat heartbeat = new VmAfdHeartbeat(IDENTITY_MANAGER_BIND_NAME, Tenant.RMI_PORT);

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

    private static void startHeartbeat() {
        heartbeat.startBeating();
        logger.info("Heartbeat started");
    }

    private static void stopHeartbeat() {
        heartbeat.stopBeating();
        logger.info("Heartbeat stopped");
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
     * Retrieve the performance data sink for IDM.
     *
     * @return a performance data sink.
     */
    public static synchronized IPerfDataSink getPerfDataSinkInstance() {
        if (perfDataSink == null) {
            perfDataSink = new PerfDataSink(reportHitCount, reportInterval);
        }

        return perfDataSink;
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
            logger.debug("Creating RMI registry on port {}", Tenant.RMI_PORT);

            boolean allowRemoteConnections = Boolean.parseBoolean(System.getProperty(ALLOW_REMOTE_PROPERTY, "false"));

            if (allowRemoteConnections) {
                logger.warn("RMI registry is allowing remote connections!");
                registry = LocateRegistry.createRegistry(Tenant.RMI_PORT);
            } else {
                logger.debug("RMI registry is restricted to the localhost");
                RMIClientSocketFactory csf = RMISocketFactory.getDefaultSocketFactory();
                RMIServerSocketFactory ssf = new LocalRMIServerSocketFactory();
                registry = LocateRegistry.createRegistry(Tenant.RMI_PORT, csf, ssf);
            }

            // Assign a security manager, in the event that dynamic classes are loaded
            if (System.getSecurityManager() == null) {
                logger.debug("Creating RMI Security Manager...");
                System.setSecurityManager(new RMISecurityManager());
            }

            logger.debug("Creating Config Store factory...");
            IConfigStoreFactory cfgStoreFactory = new ConfigStoreFactory();

            logger.debug("Creating Identity Provider factory...");
            IProviderFactory providerFactory = new ProviderFactory();

            logger.debug("Checking VMware Directory Service...");
            ServerUtils.check_directory_service();

            logger.debug("Setting system properties...");
            System.setProperties(new ThreadLocalProperties(System.getProperties()));

            logger.debug("Creating Identity Manager instance...");
            manager = new IdentityManager(cfgStoreFactory, providerFactory);

            String rmiAddress = String.format("rmi://localhost:%d/%s", Tenant.RMI_PORT, IDENTITY_MANAGER_BIND_NAME);
            logger.debug("Binding to RMI address '{}'", rmiAddress);
            loginManager = new IdmLoginManager(manager);
            ILoginManager stub = (ILoginManager) UnicastRemoteObject.exportObject(loginManager, 0);
            Naming.rebind(rmiAddress, stub);

            startHeartbeat();

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

            if (registry != null) {
                logger.debug("Unbinding the registry...");
                registry.unbind(IDENTITY_MANAGER_BIND_NAME);
            }

            stopHeartbeat();

            logger.info("IDM Server has stopped");
        } catch (Throwable t) {
            logger.error(VmEvent.SERVER_ERROR, "IDM Server failed to stop", t);
            throw t;
        }
    }
}
