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
package com.vmware.identity.sts.ws.handlers;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

import javax.servlet.http.HttpServletRequest;
import javax.xml.namespace.QName;
import javax.xml.ws.handler.MessageContext;
import javax.xml.ws.handler.soap.SOAPHandler;
import javax.xml.ws.handler.soap.SOAPMessageContext;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsContextFactory;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsContextScope;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.ws.TenantExtractor;
import com.vmware.identity.sts.ws.WsConstants;

/**
 * This class is used for setup logging context.
 *
 */
public class LogContextHandler implements SOAPHandler<SOAPMessageContext>
{
    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory
        .getLogger(LogContextHandler.class);
    private IDiagnosticsContextScope _diagCtxt;

    public LogContextHandler()
    {
        this._diagCtxt = null;
    }

    @Override
    public boolean handleMessage(SOAPMessageContext context)
    {
        Validate.notNull(context, "SOAPMessageContext should not be null.");

        Boolean outbound =
            (Boolean) context.get(MessageContext.MESSAGE_OUTBOUND_PROPERTY);

        if (outbound != null && outbound.equals(Boolean.TRUE))
        {
            return true;
        }
        else
        {
            String tenant = null;
            String correlationId = null;

            // http://docs.oracle.com/javase/7/docs/api/javax/xml/ws/handler/MessageContext.html :
            //     static final String HTTP_REQUEST_HEADERS
            //         Standard property: HTTP request headers.
            //         Type: java.util.Map<java.lang.String, java.util.List<java.lang.String>>
            //
            //     static final String SERVLET_REQUEST
            //         Standard property: servlet request object.
            //         Type: javax.servlet.http.HttpServletRequest

            HttpServletRequest request = (HttpServletRequest)
                (context.get(MessageContext.SERVLET_REQUEST));

            Validate.notNull(request, "HttpServletRequest should not be null.");

            @SuppressWarnings("unchecked")
            Map<String, List<String>> headers = (Map<String, List<String>>)(context.get(MessageContext.HTTP_REQUEST_HEADERS));
            if (headers!= null)
            {
                List<String> correlationIds = headers.get(WsConstants.ACTIVITY_CORRELATION_ID_CUSTOM_HEADER);
                if ( (correlationIds != null) && (correlationIds.size() > 1) )
                {
                    correlationId = correlationIds.get(0);
                    correlationId = LogContextHandler.removeNewline(correlationId);
                    correlationId = LogContextHandler.truncate(correlationId, 200);
                }
            }

            if ( ( correlationId == null ) || (correlationId.isEmpty()) )
            {
                correlationId = UUID.randomUUID().toString();
                logger.debug("unable to extract correlation id from request. generated new correllation id [{}]", correlationId);
            }
            else
            {
                logger.debug("extracted correlation id [{}] from the request", correlationId);
            }

            try
            {
                tenant = TenantExtractor.extractTenantName(request.getPathInfo());
                tenant = LogContextHandler.removeNewline(tenant);
                tenant = LogContextHandler.truncate(tenant, 200);
                logger.debug("extracted tenant [{}] from the request", tenant);
            }
            catch(NoSuchIdPException ex)
            {
                logger.error("failed to extract tenant from the request", ex);
            }

            if ( ( tenant == null ) || (tenant.isEmpty()) )
            {
                tenant = WsConstants.DEFAULT_TENANT;
                logger.debug("unable to extract explicit tenant name from request. Using default tenant marker [{}].", tenant);
            }
            else
            {
                logger.debug("extracted tenant name [{}] from the request", tenant);
            }

            this._diagCtxt = DiagnosticsContextFactory.createContext(correlationId, tenant);
        }

        return true;
    }

    @Override
    public void close(MessageContext context)
    {
        // cleanup diag ctxt
        if(this._diagCtxt != null)
        {
            this._diagCtxt.close();
            this._diagCtxt = null;
        }
    }

    @Override
    public boolean handleFault(SOAPMessageContext context)
    {
        // nothing specific here
        return true;
    }

    @Override
    public Set<QName> getHeaders()
    {
        // we are not handling any SOAP headers
        return Collections.<QName>emptySet();
    }

    static String removeNewline(String str)
    {
        String result;
        if (str == null)
        {
            result = str;
        }
        else
        {
            result = str.replaceAll("(\\r|\\n)", "");
        }
        return result;
    }

    static String truncate(String str, int maxLength)
    {
        String result;
        if (str == null || str.length() <= maxLength)
        {
            result = str;
        }
        else
        {
            result = str.substring(0, maxLength);
            result += "...";
        }
        return result;
    }
}
