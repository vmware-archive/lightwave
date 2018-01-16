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
package com.vmware.identity.idm;

import org.apache.commons.lang.Validate;

public class IdmServiceContextFactory {

    public static IIdmServiceContext getIdmServiceContext(String correlationId, String userId, String sessionId)
    {
        return new IdmServiceContext(correlationId, userId, sessionId);
    }

    public static IIdmServiceContext getIdmServiceContext(String correlationId)
    {
        return new IdmServiceContext(correlationId, "", "");
    }

    private static class IdmServiceContext extends IIdmServiceContext
    {
        private static final long serialVersionUID = -7109885382886664178L;
        private final String _correlationId;
        private final String _userId;
        private final String _sessionId;

        public IdmServiceContext(String correlationId, String userId, String sessionId)
        {
            Validate.notEmpty(correlationId, "Invalid correlationId. CorrelationId cannot be null or empty");
            this._correlationId = correlationId;
            this._userId = userId == null ? "" : userId;
            this._sessionId = sessionId == null ? "" : sessionId;
        }

        @Override
        public String getCorrelationId()
        {
            return this._correlationId;
        }

        @Override
        public String getUserId() {
            return this._userId;
        }

        @Override
        public String getSessionId() {
            return this._sessionId;
        }
    }
}
