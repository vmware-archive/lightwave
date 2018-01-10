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
    public static IDiagnosticsContextScope createContext( String correlationId, String tenantName, String userId, String sessionId )
    {
        return new DiagnosticsContext( correlationId, tenantName, userId, sessionId );
    }

    public static IDiagnosticsContextScope createContext( String correlationId, String tenantName ) {
        return new DiagnosticsContext( correlationId, tenantName, "", "");
    }

    public static IDiagnosticsContext getCurrentDiagnosticsContext()
    {
        return new DiagnosticsContextCopy();
    }

    public static void removeCurrentDiagnosticsContext() {
        ThreadContext.remove( DiagnosticsConstants.CorrelationIdMdcKey );
        ThreadContext.remove( DiagnosticsConstants.TenantNameMdcKey );
        ThreadContext.remove( DiagnosticsConstants.UserIdMdcKey );
        ThreadContext.remove( DiagnosticsConstants.SessionIdMdcKey );
    }

    public static void addContext(String key, String value) {
        ThreadContext.put( key, value );
    }

    private static class DiagnosticsContext implements IDiagnosticsContextScope
    {
        private final String _previousCorrelationId;
        private final String _previousTenantName;
        private final String _previousUserId;
        private final String _previousSessionId;

        public DiagnosticsContext( String correlationId, String tenantName, String userId, String sessionId )
        {
            // save the current context
            _previousCorrelationId = ThreadContext.get(DiagnosticsConstants.CorrelationIdMdcKey);
            _previousTenantName = ThreadContext.get(DiagnosticsConstants.TenantNameMdcKey);
            _previousUserId = ThreadContext.get(DiagnosticsConstants.UserIdMdcKey);
            _previousSessionId = ThreadContext.get(DiagnosticsConstants.SessionIdMdcKey);

            ThreadContext.put( DiagnosticsConstants.CorrelationIdMdcKey, correlationId == null ? "" : correlationId);
            ThreadContext.put( DiagnosticsConstants.TenantNameMdcKey, tenantName == null ? "" : tenantName );
            ThreadContext.put( DiagnosticsConstants.UserIdMdcKey, userId == null ? "" : userId );
            ThreadContext.put( DiagnosticsConstants.SessionIdMdcKey, sessionId == null ? "" : sessionId );
        }

        @Override
        public void close()
        {
            ThreadContext.remove( DiagnosticsConstants.CorrelationIdMdcKey );
            ThreadContext.remove( DiagnosticsConstants.TenantNameMdcKey );
            ThreadContext.remove( DiagnosticsConstants.UserIdMdcKey );
            ThreadContext.remove( DiagnosticsConstants.SessionIdMdcKey );

            // restore the previous context
            if (_previousCorrelationId != null) {
                ThreadContext.put(DiagnosticsConstants.CorrelationIdMdcKey, _previousCorrelationId);
            }
            if (_previousTenantName != null) {
                ThreadContext.put(DiagnosticsConstants.TenantNameMdcKey, _previousTenantName);
            }
            if (_previousUserId != null) {
                ThreadContext.put(DiagnosticsConstants.UserIdMdcKey, _previousUserId);
            }
            if (_previousSessionId != null) {
                ThreadContext.put(DiagnosticsConstants.SessionIdMdcKey, _previousSessionId);
            }
        }
    }

    private static class DiagnosticsContextCopy implements IDiagnosticsContext
    {
        private final String _correlationId;
        private final String _tenantName;
        private final String _userId;
        private final String _sessionId;

        public DiagnosticsContextCopy()
        {

            this._correlationId = ThreadContext.get(DiagnosticsConstants.CorrelationIdMdcKey);
            this._tenantName = ThreadContext.get(DiagnosticsConstants.TenantNameMdcKey);
            this._userId = ThreadContext.get(DiagnosticsConstants.UserIdMdcKey);
            this._sessionId = ThreadContext.get(DiagnosticsConstants.SessionIdMdcKey);
        }

        @Override
        public String getCorrelationId()
        {
            return this._correlationId == null ? "" : this._correlationId;
        }

        @Override
        public String getTenantName()
        {
            return this._tenantName == null ? "" : this._tenantName;
        }

        @Override
        public String getUserId()
        {
            return this._userId == null ? "" : this._userId;
        }

        @Override
        public String getSessionId()
        {
            return this._sessionId == null ? "" : this._sessionId;
        }
    }
}