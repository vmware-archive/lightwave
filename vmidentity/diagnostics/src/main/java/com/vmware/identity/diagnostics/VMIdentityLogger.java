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

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Marker;
import org.apache.logging.log4j.ThreadContext;
import org.apache.logging.log4j.message.Message;
import org.apache.logging.log4j.message.MessageFactory;

class VMIdentityLogger extends IDiagnosticsLogger{
    private final Logger _logger;
    private final Logger _evtLogger;
    private static final String _prefix = "vmevent.";

    public VMIdentityLogger(Class<?> theClass)
    {
        this._logger = LogManager.getLogger(theClass);
        this._evtLogger = LogManager.getLogger(_prefix + theClass.getName());
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

}
