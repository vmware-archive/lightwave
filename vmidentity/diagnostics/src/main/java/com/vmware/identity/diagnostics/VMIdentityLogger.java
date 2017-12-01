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

import java.net.InetAddress;
import java.net.UnknownHostException;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Marker;
import org.apache.logging.log4j.ThreadContext;
import org.apache.logging.log4j.message.EntryMessage;
import org.apache.logging.log4j.message.Message;
import org.apache.logging.log4j.message.MessageFactory;
import org.apache.logging.log4j.util.MessageSupplier;
import org.apache.logging.log4j.util.Supplier;

class VMIdentityLogger extends IDiagnosticsLogger{

    private static final long serialVersionUID = 7607601306494261885L;
    private final Logger _logger;
    private final Logger _evtLogger;
    private static final String _prefix = "vmevent.";
    /**
     * System's hostname property, which will use by log4j
     */
    private static final String HOSTNAME_PROPERTY_NAME = "hostname";

    public VMIdentityLogger(Class<?> theClass)
    {
        this._logger = LogManager.getLogger(theClass);
        this._evtLogger = LogManager.getLogger(_prefix + theClass.getName());
        String hostname = System.getProperty(HOSTNAME_PROPERTY_NAME);
        if ((hostname == null || hostname.isEmpty())) {
            try {
                System.setProperty(HOSTNAME_PROPERTY_NAME, InetAddress.getLocalHost().getHostName());
            } catch (UnknownHostException e) {
                this._logger.warn("Unable to set hostname system property. Hostname cannot be resolved.");
            }
        }
    }

    @Override
    public void debug(String message) {
        this._logger.debug(message);
    }

    @Override
    public void debug(String message, Object... args) {
        this._logger.debug(message, args);
    }

    @Override
    public void error(String message) {
        this._logger.error(message);
    }

    @Override
    public void error(String message, Object... args) {
        this._logger.error(message, args);
    }

    @Override
    public void info(String message) {
        this._logger.info(message);
    }

    @Override
    public void info(String message, Object... args) {
        this._logger.info(message, args);
    }

    @Override
    public void trace(String message) {
        this._logger.trace(message);
    }

    @Override
    public void trace(String message, Object... args) {
        this._logger.trace(message, args);
    }

    @Override
    public void warn(String message) {
        this._logger.warn(message);
    }

    @Override
    public void warn(String message, Object... args) {
        this._logger.warn(message, args);
    }

    @Override
    public void catching(Throwable arg0) {
        this._logger.catching(arg0);

}

    @Override
    public void catching(Level arg0, Throwable arg1) {
        this._logger.catching(arg0, arg1);
    }

    @Override
    public void debug(Message arg0) {
        this._logger.debug(arg0);
    }

    @Override
    public void debug(Object arg0) {
        this._logger.debug(arg0);
    }

    @Override
    public void debug(Marker arg0, Message arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.debug(arg0, arg1);
                this._logger.debug(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.debug(arg0, arg1);
        }
    }

    @Override
    public void debug(Marker arg0, Object arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.debug(arg0, arg1);
                this._logger.debug(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.debug(arg0, arg1);
        }
    }

    @Override
    public void debug(Marker arg0, String arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.debug(arg0, arg1);
                this._logger.debug(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.debug(arg0, arg1);
        }
    }

    @Override
    public void debug(Message arg0, Throwable arg1) {
        this._logger.debug(arg0, arg1);
    }

    @Override
    public void debug(Object arg0, Throwable arg1) {
        this._logger.debug(arg0, arg1);
    }

    @Override
    public void debug(String arg0, Throwable arg1) {
        this._logger.debug(arg0, arg1);
    }

    @Override
    public void debug(Marker arg0, Message arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.debug(arg0, arg1, arg2);
                this._logger.debug(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.debug(arg0, arg1, arg2);
        }
    }

    @Override
    public void debug(Marker arg0, Object arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.debug(arg0, arg1, arg2);
                this._logger.debug(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.debug(arg0, arg1, arg2);
        }
    }

    @Override
    public void debug(Marker arg0, String arg1, Object... arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.debug(arg0, arg1, arg2);
                this._logger.debug(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.debug(arg0, arg1, arg2);
        }
    }

    @Override
    public void debug(Marker arg0, String arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.debug(arg0, arg1, arg2);
                this._logger.debug(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.debug(arg0, arg1, arg2);
        }
    }

    @Override
    public void entry() {
        this._logger.entry();
    }

    @Override
    public void entry(Object... arg0) {
        this._logger.entry(arg0);
    }

    @Override
    public void error(Message arg0) {
        this._logger.error(arg0);
    }

    @Override
    public void error(Object arg0) {
        this._logger.error(arg0);
    }

    @Override
    public void error(Marker arg0, Message arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.error(arg0, arg1);
                this._logger.error(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1);
        }
    }

    @Override
    public void error(Marker arg0, Object arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.error(arg0, arg1);
                this._logger.error(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1);
        }
    }

    @Override
    public void error(Marker arg0, String arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.error(arg0, arg1);
                this._logger.error(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1);
        }
    }

    @Override
    public void error(Message arg0, Throwable arg1) {
        this._logger.error(arg0, arg1);
    }

    @Override
    public void error(Object arg0, Throwable arg1) {
        this._logger.error(arg0, arg1);
    }

    @Override
    public void error(String arg0, Throwable arg1) {
        this._logger.error(arg0, arg1);
    }

    @Override
    public void error(Marker arg0, Message arg1, Throwable arg2) {
        this._logger.error(arg0, arg1, arg2);
    }

    @Override
    public void error(Marker arg0, Object arg1, Throwable arg2) {
        this._logger.error(arg0, arg1, arg2);
    }

    @Override
    public void error(Marker arg0, String arg1, Object... arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.error(arg0, arg1, arg2);
                this._logger.error(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1, arg2);
        }
    }

    @Override
    public void error(Marker arg0, String arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.error(arg0, arg1, arg2);
                this._logger.error(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1, arg2);
        }
    }

    @Override
    public void exit() {
        this._logger.exit();
    }

    @Override
    public <R> R exit(R arg0) {
        return this._logger.exit(arg0);
    }

    @Override
    public void fatal(Message arg0) {
        this._logger.fatal(arg0);
    }

    @Override
    public void fatal(Object arg0) {
        this._logger.fatal(arg0);
    }

    @Override
    public void fatal(String arg0) {
        this._logger.fatal(arg0);
    }

    @Override
    public void fatal(Marker arg0, Message arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.fatal(arg0, arg1);
                this._logger.fatal(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.fatal(arg0, arg1);
        }
    }

    @Override
    public void fatal(Marker arg0, Object arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.fatal(arg0, arg1);
                this._logger.fatal(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.fatal(arg0, arg1);
        }
    }

    @Override
    public void fatal(Marker arg0, String arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.fatal(arg0, arg1);
                this._logger.fatal(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.fatal(arg0, arg1);
        }
    }

    @Override
    public void fatal(Message arg0, Throwable arg1) {
        this._logger.fatal(arg0, arg1);
    }

    @Override
    public void fatal(Object arg0, Throwable arg1) {
        this._logger.fatal(arg0, arg1);
    }

    @Override
    public void fatal(String arg0, Object... arg1) {
        this._logger.fatal(arg0, arg1);
    }

    @Override
    public void fatal(String arg0, Throwable arg1) {
        this._logger.fatal(arg0, arg1);
    }

    @Override
    public void fatal(Marker arg0, Message arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.fatal(arg0, arg1, arg2);
                this._logger.fatal(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.fatal(arg0, arg1, arg2);
        }
    }

    @Override
    public void fatal(Marker arg0, Object arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.fatal(arg0, arg1, arg2);
                this._logger.fatal(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.fatal(arg0, arg1, arg2);
        }
    }

    @Override
    public void fatal(Marker arg0, String arg1, Object... arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.fatal(arg0, arg1, arg2);
                this._logger.fatal(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.fatal(arg0, arg1, arg2);
        }
    }

    @Override
    public void fatal(Marker arg0, String arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.fatal(arg0, arg1, arg2);
                this._logger.fatal(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1, arg2);
        }
    }

    @Override
    public MessageFactory getMessageFactory() {
        return this._logger.getMessageFactory();
    }

    @Override
    public String getName() {
        return this._logger.getName();
    }

    @Override
    public void info(Message arg0) {
        this._logger.info(arg0);
    }

    @Override
    public void info(Object arg0) {
        this._logger.info(arg0);
    }

    @Override
    public void info(Marker arg0, Message arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.info(arg0, arg1);
                this._logger.info(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.info(arg0, arg1);
        }
    }

    @Override
    public void info(Marker arg0, Object arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.info(arg0, arg1);
                this._logger.info(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.info(arg0, arg1);
        }
    }

    @Override
    public void info(Marker arg0, String arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
               this._evtLogger.info(arg0, arg1);
                this._logger.info(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.info(arg0, arg1);
        }
    }

    @Override
    public void info(Message arg0, Throwable arg1) {
        this._logger.info(arg0, arg1);
    }

    @Override
    public void info(Object arg0, Throwable arg1) {
        this._logger.info(arg0, arg1);
    }

    @Override
    public void info(String arg0, Throwable arg1) {
        this._logger.info(arg0, arg1);
    }

    @Override
    public void info(Marker arg0, Message arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.info(arg0, arg1);
                this._logger.info(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1, arg2);
        }
    }

    @Override
    public void info(Marker arg0, Object arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.info(arg0, arg1);
                this._logger.info(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1, arg2);
        }
    }

    @Override
    public void info(Marker arg0, String arg1, Object... arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.error(arg0, arg1);
                this._logger.error(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1, arg2);
        }
    }

    @Override
    public void info(Marker arg0, String arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.info(arg0, arg1);
                this._logger.info(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1, arg2);
        }
    }

    @Override
    public boolean isDebugEnabled() {
        return this._logger.isDebugEnabled();
    }

    @Override
    public boolean isDebugEnabled(Marker arg0) {
        return this._logger.isDebugEnabled(arg0);
    }

    @Override
    public boolean isEnabled(Level arg0) {
        return this._logger.isEnabled(arg0);
    }

    @Override
    public boolean isEnabled(Level arg0, Marker arg1) {
        return this._logger.isEnabled(arg0, arg1);
    }

    @Override
    public boolean isErrorEnabled() {
        return this._logger.isErrorEnabled();
    }

    @Override
    public boolean isErrorEnabled(Marker arg0) {
        return this._logger.isErrorEnabled(arg0);
    }

    @Override
    public boolean isFatalEnabled() {
        return this._logger.isFatalEnabled();
    }

    @Override
    public boolean isFatalEnabled(Marker arg0) {
        return this._logger.isFatalEnabled(arg0);
    }

    @Override
    public boolean isInfoEnabled() {
        return this._logger.isInfoEnabled();
    }

    @Override
    public boolean isInfoEnabled(Marker arg0) {
        return this._logger.isInfoEnabled(arg0);
    }

    @Override
    public boolean isTraceEnabled() {
        return this._logger.isTraceEnabled();
    }

    @Override
    public boolean isTraceEnabled(Marker arg0) {
        return this._logger.isTraceEnabled();
    }

    @Override
    public boolean isWarnEnabled() {
        return this._logger.isWarnEnabled();
    }

    @Override
    public boolean isWarnEnabled(Marker arg0) {
        return this._logger.isWarnEnabled(arg0);
    }

    @Override
    public void log(Level arg0, Message arg1) {
        this._logger.log(arg0, arg1);
    }

    @Override
    public void log(Level arg0, Object arg1) {
        this._logger.log(arg0, arg1);
    }

    @Override
    public void log(Level arg0, String arg1) {
        this._logger.log(arg0, arg1);
    }

    @Override
    public void log(Level arg0, Marker arg1, Message arg2) {
        this._logger.log(arg0, arg1, arg2);
    }

    @Override
    public void log(Level arg0, Marker arg1, Object arg2) {
        this._logger.log(arg0, arg1, arg2);
    }

    @Override
    public void log(Level arg0, Marker arg1, String arg2) {
        this._logger.log(arg0, arg1, arg2);
    }

    @Override
    public void log(Level arg0, Message arg1, Throwable arg2) {
        this._logger.log(arg0, arg1, arg2);
    }

    @Override
    public void log(Level arg0, Object arg1, Throwable arg2) {
        this._logger.log(arg0, arg1, arg2);
    }

    @Override
    public void log(Level arg0, String arg1, Object... arg2) {
        this._logger.log(arg0, arg1, arg2);
    }

    @Override
    public void log(Level arg0, String arg1, Throwable arg2) {
        this._logger.log(arg0, arg1, arg2);
    }

    @Override
    public void log(Level arg0, Marker arg1, Message arg2, Throwable arg3) {
        this._logger.log(arg0, arg1, arg2, arg3);
    }

    @Override
    public void log(Level arg0, Marker arg1, Object arg2, Throwable arg3) {
        this._logger.log(arg0, arg1, arg2, arg3);
    }

    @Override
    public void log(Level arg0, Marker arg1, String arg2, Object... arg3) {
        this._logger.log(arg0, arg1, arg2, arg3);
    }

    @Override
    public void log(Level arg0, Marker arg1, String arg2, Throwable arg3) {
        this._logger.log(arg0, arg1, arg2, arg3);
    }

    @Override
    public void printf(Level arg0, String arg1, Object... arg2) {
        this._logger.printf(arg0, arg1, arg2);
    }

    @Override
    public void printf(Level arg0, Marker arg1, String arg2, Object... arg3) {
        this._logger.printf(arg0, arg1, arg2, arg3);
    }

    @Override
    public <T extends Throwable> T throwing(T arg0) {
        return this._logger.throwing(arg0);
    }

    @Override
    public <T extends Throwable> T throwing(Level arg0, T arg1) {
        return this._logger.throwing(arg0, arg1);
    }

    @Override
    public void trace(Message arg0) {
        this._logger.trace(arg0);
    }

    @Override
    public void trace(Object arg0) {
        this._logger.trace(arg0);
    }

    @Override
    public void trace(Marker arg0, Message arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.trace(arg0, arg1);
                this._logger.trace(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1);
        }
    }

    @Override
    public void trace(Marker arg0, Object arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.trace(arg0, arg1);
                this._logger.trace(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1);
        }
    }

    @Override
    public void trace(Marker arg0, String arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.trace(arg0, arg1);
                this._logger.trace(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.error(arg0, arg1);
        }
    }

    @Override
    public void trace(Message arg0, Throwable arg1) {
        this._logger.trace(arg0, arg1);
    }

    @Override
    public void trace(Object arg0, Throwable arg1) {
        this._logger.trace(arg0, arg1);
    }

    @Override
    public void trace(String arg0, Throwable arg1) {
        this._logger.trace(arg0, arg1);
    }

    @Override
    public void trace(Marker arg0, Message arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.trace(arg0, arg1, arg2);
                this._logger.trace(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.trace(arg0, arg1, arg2);
        }
    }

    @Override
    public void trace(Marker arg0, Object arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.trace(arg0, arg1, arg2);
                this._logger.trace(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.trace(arg0, arg1, arg2);
        }
    }

    @Override
    public void trace(Marker arg0, String arg1, Object... arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.trace(arg0, arg1, arg2);
                this._logger.trace(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.trace(arg0, arg1, arg2);
        }
    }

    @Override
    public void trace(Marker arg0, String arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.trace(arg0, arg1, arg2);
                this._logger.trace(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.trace(arg0, arg1, arg2);
        }
    }

    @Override
    public void warn(Message arg0) {
        this._logger.warn(arg0);
    }

    @Override
    public void warn(Object arg0) {
        this._logger.warn(arg0);
    }

    @Override
    public void warn(Marker arg0, Message arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.warn(arg0, arg1);
                this._logger.warn(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.warn(arg0, arg1);
        }
    }

    @Override
    public void warn(Marker arg0, Object arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.warn(arg0, arg1);
                this._logger.warn(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.warn(arg0, arg1);
        }
    }

    @Override
    public void warn(Marker arg0, String arg1) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.warn(arg0, arg1);
                this._logger.warn(arg0, arg1);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.warn(arg0, arg1);
        }
    }

    @Override
    public void warn(Message arg0, Throwable arg1) {
        this._logger.warn(arg0, arg1);
    }

    @Override
    public void warn(Object arg0, Throwable arg1) {
        this._logger.warn(arg0, arg1);
    }

    @Override
    public void warn(String arg0, Throwable arg1) {
        this._logger.warn(arg0, arg1);
    }

    @Override
    public void warn(Marker arg0, Message arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.warn(arg0, arg1, arg2);
                this._logger.warn(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.warn(arg0, arg1, arg2);
        }
    }

    @Override
    public void warn(Marker arg0, Object arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.warn(arg0, arg1, arg2);
                this._logger.warn(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.warn(arg0, arg1, arg2);
        }
    }

    @Override
    public void warn(Marker arg0, String arg1, Object... arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.warn(arg0, arg1, arg2);
                this._logger.warn(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.warn(arg0, arg1, arg2);
        }
    }

    @Override
    public void warn(Marker arg0, String arg1, Throwable arg2) {
        if ( arg0 instanceof IVmEvent )
        {
            ThreadContext.put(DiagnosticsConstants.EventIdMarkerMdcKey, arg0.getName());
            try
            {
                this._evtLogger.warn(arg0, arg1, arg2);
                this._logger.warn(arg0, arg1, arg2);
            }
            finally
            {
                ThreadContext.remove(DiagnosticsConstants.EventIdMarkerMdcKey);
            }
        }
        else
        {
            this._logger.warn(arg0, arg1, arg2);
        }
    }

    @Override
    public void logMessage(String arg0, Level arg1, Marker arg2, Message arg3,
            Throwable arg4) {
        this._logger.log(arg1, arg2, arg0);
    }

    @Override
    public Level getLevel() {
        return this._logger.getLevel();
    }

    @Override
    public boolean isEnabled(Level arg0, Marker arg1, String arg2) {
        return this._logger.isEnabled(arg0);
    }

    @Override
    public boolean isEnabled(Level arg0, Marker arg1, Message arg2,
            Throwable arg3) {
        return this._logger.isEnabled(arg0,arg1);
    }

    @Override
    public boolean isEnabled(Level arg0, Marker arg1, Object arg2,
            Throwable arg3) {
        return this._logger.isEnabled(arg0,arg1);
    }

    @Override
    public boolean isEnabled(Level arg0, Marker arg1, String arg2,
            Throwable arg3) {
        return this._logger.isEnabled(arg0,arg1);
    }

    @Override
    public boolean isEnabled(Level arg0, Marker arg1, String arg2,
            Object... arg3) {
        return this._logger.isEnabled(arg0);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public boolean isEnabled(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        return this._logger.isEnabled(level, marker);
    }

    @Override
    public void debug(Marker marker, CharSequence message) {
        // dispatch to this._logger
        this._logger.debug(marker, message);
    }

    @Override
    public void debug(Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.debug(marker, message, t);
    }

    @Override
    public void debug(CharSequence message) {
        // dispatch to this._logger
        this._logger.debug(message);
    }

    @Override
    public void debug(CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.debug(message, t);
    }

    @Override
    public void debug(Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.debug(msgSupplier);
    }

    @Override
    public void debug(Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.debug(msgSupplier, t);
    }

    @Override
    public void debug(Marker marker, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.debug(marker, msgSupplier);
    }

    @Override
    public void debug(Marker marker, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.debug(marker, message, paramSuppliers);
    }

    @Override
    public void debug(Marker marker, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.debug(marker, msgSupplier, t);
    }

    @Override
    public void debug(String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.debug(message, paramSuppliers);
    }

    @Override
    public void debug(Marker marker, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.debug(marker, msgSupplier);
    }

    @Override
    public void debug(Marker marker, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.debug(marker, msgSupplier, t);
    }

    @Override
    public void debug(MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.debug(msgSupplier);
    }

    @Override
    public void debug(MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.debug(msgSupplier, t);
    }

    @Override
    public void debug(Marker marker, String message, Object p0) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2, p3);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void debug(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.debug(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void debug(String message, Object p0) {
        // dispatch to this._logger
        this._logger.debug(message, p0);
    }

    @Override
    public void debug(String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2, p3);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2, p3, p4);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void debug(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.debug(message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void error(Marker marker, CharSequence message) {
        // dispatch to this._logger
        this._logger.error(marker, message);
    }

    @Override
    public void error(Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.error(marker, message, t);
    }

    @Override
    public void error(CharSequence message) {
        // dispatch to this._logger
        this._logger.error(message);
    }

    @Override
    public void error(CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.error(message, t);
    }

    @Override
    public void error(Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.error(msgSupplier);
    }

    @Override
    public void error(Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.error(msgSupplier, t);
    }

    @Override
    public void error(Marker marker, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.error(marker, msgSupplier);
    }

    @Override
    public void error(Marker marker, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.error(marker, message, paramSuppliers);
    }

    @Override
    public void error(Marker marker, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.error(marker, msgSupplier, t);
    }

    @Override
    public void error(String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.error(message, paramSuppliers);
    }

    @Override
    public void error(Marker marker, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.error(marker, msgSupplier);
    }

    @Override
    public void error(Marker marker, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.error(marker, msgSupplier, t);
    }

    @Override
    public void error(MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.error(msgSupplier);
    }

    @Override
    public void error(MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.error(msgSupplier, t);
    }

    @Override
    public void error(Marker marker, String message, Object p0) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2, p3);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void error(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.error(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void error(String message, Object p0) {
        // dispatch to this._logger
        this._logger.error(message, p0);
    }

    @Override
    public void error(String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2, p3);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2, p3, p4);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void error(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.error(message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void fatal(Marker marker, CharSequence message) {
        // dispatch to this._logger
        this._logger.fatal(marker, message);
    }

    @Override
    public void fatal(Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, t);
    }

    @Override
    public void fatal(CharSequence message) {
        // dispatch to this._logger
        this._logger.fatal(message);
    }

    @Override
    public void fatal(CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.fatal(message, t);
    }

    @Override
    public void fatal(Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.fatal(msgSupplier);
    }

    @Override
    public void fatal(Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.fatal(msgSupplier, t);
    }

    @Override
    public void fatal(Marker marker, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.fatal(marker, msgSupplier);
    }

    @Override
    public void fatal(Marker marker, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, paramSuppliers);
    }

    @Override
    public void fatal(Marker marker, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.fatal(marker, msgSupplier, t);
    }

    @Override
    public void fatal(String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.fatal(message, paramSuppliers);
    }

    @Override
    public void fatal(Marker marker, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.fatal(marker, msgSupplier);
    }

    @Override
    public void fatal(Marker marker, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.fatal(marker, msgSupplier, t);
    }

    @Override
    public void fatal(MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.fatal(msgSupplier);
    }

    @Override
    public void fatal(MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.fatal(msgSupplier, t);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2, p3);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void fatal(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.fatal(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void fatal(String message, Object p0) {
        // dispatch to this._logger
        this._logger.fatal(message, p0);
    }

    @Override
    public void fatal(String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2, p3);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2, p3, p4);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void fatal(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.fatal(message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void info(Marker marker, CharSequence message) {
        // dispatch to this._logger
        this._logger.info(marker, message);
    }

    @Override
    public void info(Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.info(marker, message, t);
    }

    @Override
    public void info(CharSequence message) {
        // dispatch to this._logger
        this._logger.info(message);
    }

    @Override
    public void info(CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.info(message, t);
    }

    @Override
    public void info(Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.info(msgSupplier);
    }

    @Override
    public void info(Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.info(msgSupplier, t);
    }

    @Override
    public void info(Marker marker, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.info(marker, msgSupplier);
    }

    @Override
    public void info(Marker marker, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.info(marker, message, paramSuppliers);
    }

    @Override
    public void info(Marker marker, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.info(marker, msgSupplier, t);
    }

    @Override
    public void info(String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.info(message, paramSuppliers);
    }

    @Override
    public void info(Marker marker, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.info(marker, msgSupplier);
    }

    @Override
    public void info(Marker marker, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.info(marker, msgSupplier, t);
    }

    @Override
    public void info(MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.info(msgSupplier);
    }

    @Override
    public void info(MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.info(msgSupplier, t);
    }

    @Override
    public void info(Marker marker, String message, Object p0) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2, p3);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void info(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.info(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void info(String message, Object p0) {
        // dispatch to this._logger
        this._logger.info(message, p0);
    }

    @Override
    public void info(String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2, p3);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2, p3, p4);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void info(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.info(message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void log(Level level, Marker marker, CharSequence message) {
        // dispatch to this._logger
        this._logger.log(level, marker, message);
    }

    @Override
    public void log(Level level, Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, t);
    }

    @Override
    public void log(Level level, CharSequence message) {
        // dispatch to this._logger
        this._logger.log(level, message);
    }

    @Override
    public void log(Level level, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.log(level, message, t);
    }

    @Override
    public void log(Level level, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.log(level, msgSupplier);
    }

    @Override
    public void log(Level level, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.log(level, msgSupplier, t);
    }

    @Override
    public void log(Level level, Marker marker, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.log(level, marker, msgSupplier);
    }

    @Override
    public void log(Level level, Marker marker, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, paramSuppliers);
    }

    @Override
    public void log(Level level, Marker marker, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.log(level, marker, msgSupplier, t);
    }

    @Override
    public void log(Level level, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.log(level, message, paramSuppliers);
    }

    @Override
    public void log(Level level, Marker marker, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.log(level, marker, msgSupplier);
    }

    @Override
    public void log(Level level, Marker marker, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.log(level, marker, msgSupplier, t);
    }

    @Override
    public void log(Level level, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.log(level, msgSupplier);
    }

    @Override
    public void log(Level level, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.log(level, msgSupplier, t);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2, p3);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void log(Level level, Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.log(level, marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void log(Level level, String message, Object p0) {
        // dispatch to this._logger
        this._logger.log(level, message, p0);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2, p3);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void log(Level level, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.log(level, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void trace(Marker marker, CharSequence message) {
        // dispatch to this._logger
        this._logger.trace(marker, message);
    }

    @Override
    public void trace(Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.trace(marker, message, t);
    }

    @Override
    public void trace(CharSequence message) {
        // dispatch to this._logger
        this._logger.trace(message);
    }

    @Override
    public void trace(CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.trace(message, t);
    }

    @Override
    public void trace(Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.trace(msgSupplier);
    }

    @Override
    public void trace(Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.trace(msgSupplier, t);
    }

    @Override
    public void trace(Marker marker, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.trace(marker, msgSupplier);
    }

    @Override
    public void trace(Marker marker, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.trace(marker, message, paramSuppliers);
    }

    @Override
    public void trace(Marker marker, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.trace(marker, msgSupplier, t);
    }

    @Override
    public void trace(String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.trace(message, paramSuppliers);
    }

    @Override
    public void trace(Marker marker, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.trace(marker, msgSupplier);
    }

    @Override
    public void trace(Marker marker, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.trace(marker, msgSupplier, t);
    }

    @Override
    public void trace(MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.trace(msgSupplier);
    }

    @Override
    public void trace(MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.trace(msgSupplier, t);
    }

    @Override
    public void trace(Marker marker, String message, Object p0) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2, p3);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void trace(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.trace(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void trace(String message, Object p0) {
        // dispatch to this._logger
        this._logger.trace(message, p0);
    }

    @Override
    public void trace(String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2, p3);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2, p3, p4);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void trace(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.trace(message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public EntryMessage traceEntry() {
        // dispatch to this._logger
        return this._logger.traceEntry();
    }

    @Override
    public EntryMessage traceEntry(String format, Object... params) {
        // dispatch to this._logger
        return this._logger.traceEntry(format, params);
    }

    @Override
    public EntryMessage traceEntry(Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        return this._logger.traceEntry(paramSuppliers);
    }

    @Override
    public EntryMessage traceEntry(String format, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        return this._logger.traceEntry(format, paramSuppliers);
    }

    @Override
    public EntryMessage traceEntry(Message message) {
        // dispatch to this._logger
        return this._logger.traceEntry(message);
    }

    @Override
    public void traceExit() {
        // dispatch to this._logger
        this._logger.traceExit();
    }

    @Override
    public <R> R traceExit(R result) {
        // dispatch to this._logger
        return this._logger.traceExit(result);
    }

    @Override
    public <R> R traceExit(String format, R result) {
        // dispatch to this._logger
        return this._logger.traceExit(format, result);
    }

    @Override
    public void traceExit(EntryMessage message) {
        // dispatch to this._logger
        this._logger.traceExit(message);
    }

    @Override
    public <R> R traceExit(EntryMessage message, R result) {
        // dispatch to this._logger
        return this._logger.traceExit(message, result);
    }

    @Override
    public <R> R traceExit(Message message, R result) {
        // dispatch to this._logger
        return this._logger.traceExit(message, result);
    }

    @Override
    public void warn(Marker marker, CharSequence message) {
        // dispatch to this._logger
        this._logger.warn(marker, message);
    }

    @Override
    public void warn(Marker marker, CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.warn(marker, message, t);
    }

    @Override
    public void warn(CharSequence message) {
        // dispatch to this._logger
        this._logger.warn(message);
    }

    @Override
    public void warn(CharSequence message, Throwable t) {
        // dispatch to this._logger
        this._logger.warn(message, t);
    }

    @Override
    public void warn(Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.warn(msgSupplier);
    }

    @Override
    public void warn(Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.warn(msgSupplier, t);
    }

    @Override
    public void warn(Marker marker, Supplier<?> msgSupplier) {
        // dispatch to this._logger
        this._logger.warn(marker, msgSupplier);
    }

    @Override
    public void warn(Marker marker, String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.warn(marker, message, paramSuppliers);
    }

    @Override
    public void warn(Marker marker, Supplier<?> msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.warn(marker, msgSupplier, t);
    }

    @Override
    public void warn(String message, Supplier<?>... paramSuppliers) {
        // dispatch to this._logger
        this._logger.warn(message, paramSuppliers);
    }

    @Override
    public void warn(Marker marker, MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.warn(marker, msgSupplier);
    }

    @Override
    public void warn(Marker marker, MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.warn(marker, msgSupplier, t);
    }

    @Override
    public void warn(MessageSupplier msgSupplier) {
        // dispatch to this._logger
        this._logger.warn(msgSupplier);
    }

    @Override
    public void warn(MessageSupplier msgSupplier, Throwable t) {
        // dispatch to this._logger
        this._logger.warn(msgSupplier, t);
    }

    @Override
    public void warn(Marker marker, String message, Object p0) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2, p3);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2, p3, p4);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void warn(Marker marker, String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.warn(marker, message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

    @Override
    public void warn(String message, Object p0) {
        // dispatch to this._logger
        this._logger.warn(message, p0);
    }

    @Override
    public void warn(String message, Object p0, Object p1) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2, Object p3) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2, p3);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2, Object p3, Object p4) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2, p3, p4);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2, p3, p4, p5);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2, p3, p4, p5, p6);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2, p3, p4, p5, p6, p7);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }

    @Override
    public void warn(String message, Object p0, Object p1, Object p2, Object p3, Object p4, Object p5, Object p6, Object p7, Object p8, Object p9) {
        // dispatch to this._logger
        this._logger.warn(message, p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }

}
