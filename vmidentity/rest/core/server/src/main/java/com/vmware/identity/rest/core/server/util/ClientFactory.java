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

import com.vmware.identity.idm.IIdmServiceContext;
import com.vmware.identity.idm.IdmServiceContextFactory;
import com.vmware.identity.idm.client.CasIdmClient;
import com.vmware.identity.idm.client.IServiceContextProvider;

public class ClientFactory {

    public static final String LOCAL_HOST = "localhost";

    private static String host = LOCAL_HOST;

    public static void setHost(String host) {
        ClientFactory.host = host;
    }

    public static CasIdmClient getClient() {
        return new CasIdmClient(host);
    }

    public static CasIdmClient getClient(String correlationId) {
        return new CasIdmClient(host, new ServiceContextProvider(correlationId));
    }

    private static class ServiceContextProvider extends IServiceContextProvider {
        private String id;

        public ServiceContextProvider(String id) {
            this.id = id;
        }

        @Override
        public IIdmServiceContext getServiceContext() {
            return IdmServiceContextFactory.getIdmServiceContext(id);
        }

    }

}
