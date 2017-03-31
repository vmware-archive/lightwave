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

import org.apache.logging.log4j.ThreadContext;

public class DiagnosticsContextFactory
{
    public static IDiagnosticsContextScope createContext( String correlationId, String tenantName )
    {
        return new DiagnosticsContext( correlationId, tenantName );
    }

    public static IDiagnosticsContext getCurrentDiagnosticsContext()
    {
        return new DiagnosticsContextCopy();
    }

    private static class DiagnosticsContext implements IDiagnosticsContextScope
    {
        private final String _previousCorrelationId;
        private final String _previousTenantName;

        public DiagnosticsContext( String correlationId, String tenantName )
        {
            // save the current context
            _previousCorrelationId = ThreadContext.get(DiagnosticsConstants.CorrelationIdMdcKey);
            _previousTenantName = ThreadContext.get(DiagnosticsConstants.TenantNameMdcKey);

            if ( correlationId == null )
            {
                correlationId = "";
            }
            if ( tenantName == null )
            {
                tenantName = "";
            }

            ThreadContext.put( DiagnosticsConstants.CorrelationIdMdcKey, correlationId );
            ThreadContext.put( DiagnosticsConstants.TenantNameMdcKey, tenantName );
        }

        @Override
        public void close()
        {
            ThreadContext.remove( DiagnosticsConstants.CorrelationIdMdcKey );
            ThreadContext.remove( DiagnosticsConstants.TenantNameMdcKey );

            // restore the previous context
            if (_previousCorrelationId != null) {
                ThreadContext.put(DiagnosticsConstants.CorrelationIdMdcKey, _previousCorrelationId);
            }
            if (_previousTenantName != null) {
                ThreadContext.put(DiagnosticsConstants.TenantNameMdcKey, _previousTenantName);
            }
        }
    }

    private static class DiagnosticsContextCopy implements IDiagnosticsContext
    {
        private final String _correlationId;
        private final String _tenantName;

        public DiagnosticsContextCopy()
        {

            this._correlationId = ThreadContext.get(DiagnosticsConstants.CorrelationIdMdcKey);
            this._tenantName = ThreadContext.get(DiagnosticsConstants.TenantNameMdcKey);
        }

        @Override
        public String getCorrelationId()
        {
            return this._correlationId;
        }

        @Override
        public String getTenantName()
        {
            return this._tenantName;
        }
    }
}