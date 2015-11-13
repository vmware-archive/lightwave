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
package com.vmware.identity.rest.core.client;

import org.apache.http.client.utils.URIBuilder;

import com.vmware.identity.cdc.CdcDCEntry;
import com.vmware.identity.cdc.CdcFactory;
import com.vmware.identity.cdc.CdcGenericException;
import com.vmware.identity.cdc.CdcSession;
import com.vmware.identity.rest.core.client.exceptions.ClientException;

/**
 * The {@code AffinitizedHostRetriever} utilizes the VMware Authentication Framework
 * to provide site-affinity. Note that VMware Authentication Framework must be available
 * and the client domain controller cache (CDC) libraries must be available on the
 * classpath.
 *
 * @see HostRetriever
 */
public class AffinitizedHostRetriever extends HostRetriever {

    private static final String DOMAIN_NAME = "";
    private static final int CDC_FLAGS = 0;

    /**
     * Construct an {@code AffinitizedHostRetriever}. Connections will be assumed
     * to not be over HTTPS and use the default port.
     */
    public AffinitizedHostRetriever() {
        super(NO_PORT, false);
    }

    /**
     * Construct an {@code AffinitizedHostRetriever} with a port. Connections will
     * be assumed to not be over HTTPS.
     *
     * @param port the port for the host.
     */
    public AffinitizedHostRetriever(int port) {
        super(port, false);
    }

    /**
     * Construct an {@code AffinitizedHostRetriever} with a security setting. Connections
     * will use the default port.
     *
     * @param secure whether to perform the request over HTTPS or not.
     */
    public AffinitizedHostRetriever(boolean secure) {
        super(NO_PORT, secure);
    }

    /**
     * Construct an {@code AffinitizedHostRetriever} with a port and security setting.
     *
     * @param port the port for the host.
     * @param secure whether to perform the request over HTTPS or not.
     */
    public AffinitizedHostRetriever(int port, boolean secure) {
        super(port, secure);
    }

    @Override
    public URIBuilder getURIBuilder() throws ClientException {
        CdcDCEntry entry = getAvailableDomainController();

        URIBuilder builder = new URIBuilder()
            .setScheme(getScheme())
            .setHost(entry.dcName);

        if (hasPort()) {
            builder.setPort(port);
        }

        return builder;
    }

    private CdcDCEntry getAvailableDomainController() throws ClientException {
        CdcDCEntry entry;

        CdcSession session = null;
        try {
            session = CdcFactory.createCdcSessionViaIPC();
            entry = session.getAffinitizedDC(DOMAIN_NAME, CDC_FLAGS);
        } catch (CdcGenericException e) {
            throw new ClientException("An error occurred with the client domain controller cache", e);
        } finally {
            if (session != null) {
                session.close();
            }
        }

        if (entry == null) {
            throw new ClientException("Client domain controller cache returned null");
        }

        return entry;
    }

}
