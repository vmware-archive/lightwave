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
package com.vmware.identity.rest.core.server.util;

import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;

import org.slf4j.bridge.SLF4JBridgeHandler;

/**
 * Listener to bridge between Jersey's default usage of java.util.logging
 * and SLF4J.
 *
 * @author Balaji Boggaram Ramanarayan
 * @author Travis Hall
 */
public class SLF4JListener implements ServletContextListener {

    @Override
    public void contextDestroyed(ServletContextEvent arg0) {
        SLF4JBridgeHandler.uninstall();
    }

    @Override
    public void contextInitialized(ServletContextEvent arg0) {
        SLF4JBridgeHandler.removeHandlersForRootLogger();
        SLF4JBridgeHandler.install();
    }

}
