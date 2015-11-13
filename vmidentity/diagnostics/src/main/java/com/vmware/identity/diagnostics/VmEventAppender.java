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

import java.io.Serializable;
import java.util.Map;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.core.Layout;
import org.apache.logging.log4j.core.Filter;
import org.apache.logging.log4j.core.LogEvent;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.core.appender.AbstractAppender;
import org.apache.logging.log4j.core.config.plugins.Plugin;
import org.apache.logging.log4j.core.config.plugins.PluginAttribute;
import org.apache.logging.log4j.core.config.plugins.PluginElement;
import org.apache.logging.log4j.core.config.plugins.PluginFactory;

/*
 * Defines and exposes custom VM appender as a plugin.
 */
@Plugin(name="VMEventAppender",
category="Core",
elementType="appender",
printObject=true)
public final class VmEventAppender extends AbstractAppender
{
    private static final String _hostName = "localhost";
    private static final Logger _logger = LogManager.getLogger(VmEventAppender.class);

    private VmEventCategory _category;

    public VmEventAppender(String name, Filter filter, Layout<? extends Serializable> layout, boolean ignoreExceptions,VmEventCategory category)
    {
        super(name, filter, layout, true);
        if(category!= null)
            this._category = category;
        else
            this.setcategory("VMEVENT_CATEGORY_UNSPECIFIED");

    }

    /*
     * Creates custom appender for the VM Event logging
     */
    @PluginFactory
    public static VmEventAppender createAppender(@PluginAttribute("name") String name,
            @PluginAttribute("ignoreExceptions") boolean ignoreExceptions,
            @PluginElement("Layout") Layout<? extends Serializable> layout,
            @PluginElement("Filters") Filter filter,@PluginAttribute("Category") VmEventCategory category) {
        return new VmEventAppender(name, filter, layout, true, category);
    }

    /*
     * (non-Javadoc)
     * @see org.apache.logging.log4j.core.Appender#append(org.apache.logging.log4j.core.LogEvent)
     *
     * Appends the event log info, when called from the log4j xml.
     */
    @Override
    public void append(LogEvent logEvent)
    {
        try
        {
            Object correlationId = null;
            Object tenantName = null;
            IVmEvent marker = null;
            Map<String,String> map = logEvent.getContextMap();
            correlationId = map.get(DiagnosticsConstants.CorrelationIdMdcKey);
            tenantName = map.get(DiagnosticsConstants.TenantNameMdcKey);
            String markerObject = map.get(DiagnosticsConstants.EventIdMarkerMdcKey);
            if(markerObject != null)
                marker = VmEvent.valueOf(markerObject);

            Level level = logEvent.getLevel();
            Object logMessage = logEvent.getMessage();

            String detailedInfo = null;
            Throwable info = logEvent.getThrown();
            // we should allow for more information to be included in the details, not just throwable...
            if (info != null)
            {
                detailedInfo = info.getMessage();
            }

            logEvent(
                DiagnosticsConstants.VmEventSource, getString(tenantName), marker, level, this._category,
                getString(logMessage), detailedInfo, getString(correlationId), logEvent.getTimeMillis()
            );
        }
        catch(Exception ex)
        {
            _logger.error("Failed to log an event into event log.", ex);
        }
        catch(Error err)
        {
            _logger.error("Failed to log an event into event log.", err);
        }
    }


    public String getcategory()
    {
        return this._category.name();
    }

    public void setcategory(String category)
    {
        if ( ( category != null ) && ( category.isEmpty() == false ) )
        {
            try
            {
                this._category = VmEventCategory.valueOf(category.toUpperCase());
            }
            catch(IllegalArgumentException ex)
            {}
        }
    }

    private static void logEvent(
        String source, String tenant, IVmEvent eventId, Level level, VmEventCategory category,
        String message, String details, String correlationId, long timestamp)
    {
        // addEvent( String hostName, String source, String user /*this is tenant*/
        //           int eventid, int type /*level*/, int category, String text, String detailText,
        //           String correlationId, long timestamp)
        // we might remove the log below, and we need to switch to new eventlog api once available.
        if(_logger.isInfoEnabled())
        {
            _logger.info(
                String.format("EventLog: source=[%s], tenant=[%s], eventid=[%s], level=[%s], category=[%s], text=[%s], detailText=[%s], corelationId=[%s], timestamp=[%d]",
                source, tenant, eventId.getEventName(), level.toString(), category.name(),
                message, details, correlationId, timestamp)
            );
        }
    }

    private static String getString(Object objString)
    {
        return ( (objString != null) ? objString.toString() : null);
    }
}
