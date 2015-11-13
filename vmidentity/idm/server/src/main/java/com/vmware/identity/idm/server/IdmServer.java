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

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.diagnostics.VmEvent;
import com.vmware.identity.idm.server.config.ConfigStoreFactory;
import com.vmware.identity.idm.server.config.IConfigStoreFactory;
import com.vmware.identity.idm.server.provider.IProviderFactory;
import com.vmware.identity.idm.server.provider.ProviderFactory;
import com.vmware.identity.performanceSupport.PerfDataSink;
import org.apache.commons.daemon.Daemon;
import org.apache.commons.daemon.DaemonContext;
import org.apache.commons.daemon.DaemonInitException;

import com.vmware.identity.idm.Tenant;
import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.idm.ILoginManager;
import java.rmi.server.UnicastRemoteObject;


public class IdmServer implements Daemon
{
    private static final String IDENTITY_MANAGER_BIND_NAME = "IdentityManager";
    private static Registry registry = null;
    private static Object serviceLock = new Object();

    private static int           reportHitCount = 100; // Trigger report by number of entries
    private static int           reportInterval = 5;   // Trigger report by time interval
    private static IPerfDataSink perfDataSink;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmServer.class);

    public static void main (String[] argv)
    {
        IdmServer.startserver(argv);
    }

    public static void startserver (String[] argv)
    {
        try(IDiagnosticsContextScope diagCtxt = DiagnosticsContextFactory.createContext("4b3cb569-e80c-4702-9505-804a4e0f86d8", ""))
        {
            try
            {
                logger.info("IDM Server starting...");
                logger.debug("Creating RMI reguistry using port {}.", Tenant.RMI_PORT);

                registry = LocateRegistry.createRegistry(Tenant.RMI_PORT);

                // Assign a security manager, in the event that dynamic
                // classes are loaded
                if (System.getSecurityManager() == null)
                {
                    logger.debug("Creating RMISecurityManager...");
                    System.setSecurityManager ( new RMISecurityManager() );
                }

                logger.debug("Creating config store factory...");
                IConfigStoreFactory cfgStoreFactory = new ConfigStoreFactory();

                logger.debug("Creating identity provider factory");

                IProviderFactory providerFactory = new ProviderFactory();

                logger.debug("Checking vmware directory...");
                ServerUtils.check_directory_service();
                logger.debug("Successfully contact vmware directory.");

                logger.debug("Creating identity manager instance");

                // make system properties ThreadLocal
                System.setProperties(new ThreadLocalProperties(System
                                .getProperties()));

                IdentityManager manager = new IdentityManager(cfgStoreFactory, providerFactory);

                logger.debug("Binding to RMI Port rmi://localhost:{}/{}", Tenant.RMI_PORT, IDENTITY_MANAGER_BIND_NAME );
                ILoginManager idmloginManager = new IdmLoginManager(manager);
                ILoginManager stub =(ILoginManager) UnicastRemoteObject.exportObject(idmloginManager, 0);

                Naming.rebind (
                        String.format("rmi://localhost:%d/%s", Tenant.RMI_PORT, IDENTITY_MANAGER_BIND_NAME),
                        stub);

                logger.debug("Waiting to acquire service lock");

                synchronized (IdmServer.serviceLock)
                {
                    logger.info(VmEvent.SERVER_STARTED, "Idm Server has started.");
                    IdmServer.serviceLock.wait();
                }

                logger.debug("Idm Server is ready.");
            }
            catch(Throwable t)
            {
                logger.error(
                    VmEvent.SERVER_FAILED_TOSTART,
                    String.format( "Start server failed with '%s'.", t.getMessage() ),
                    t
                );
            }
        }
    }

    public static synchronized IPerfDataSink getPerfDataSinkInstance()
    {
        if (perfDataSink == null)
        {
            perfDataSink = new PerfDataSink(reportHitCount, reportInterval);
        }

        return perfDataSink;
    }

    public static void stopserver (String[] argv)
    {
        try(IDiagnosticsContextScope diagCtxt = DiagnosticsContextFactory.createContext("830ee15b-ab5a-4654-82b6-4d53bd0a3a72", ""))
        {
            try
            {
                logger.info("Stopping Idm Server.");

                if (registry != null)
                {
                    logger.debug("unbinding the registry...");
                    registry.unbind( IdmServer.IDENTITY_MANAGER_BIND_NAME );
                }

                synchronized(IdmServer.serviceLock)
                {
                    IdmServer.serviceLock.notifyAll();
                }
            }
            catch(Exception ex)
            {
                logger.error(
                    VmEvent.SERVER_ERROR,
                    String.format( "stopserver failed with '%s'", ex.getMessage() ),
                    ex
                );
            }
        }
    }

    @Override
    public void destroy()
    {
    }

    @Override
    public void init(DaemonContext arg0) throws DaemonInitException, Exception
    {
    }

    @Override
    public void start() throws Exception
    {
        IdmServer.startserver(null);
    }

    @Override
    public void stop() throws Exception
    {
        IdmServer.stopserver(null);
    }
}
