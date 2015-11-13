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

package com.vmware.identity.openidconnect.client;

import com.vmware.identity.cdc.CdcDCEntry;
import com.vmware.identity.cdc.CdcGenericException;
import com.vmware.identity.cdc.CdcSession;

/**
 * @author Yehia Zayour
 */
class ClientDCCache implements AutoCloseable {
    private static final int CDC_FLAGS = 0;
    private final String domainName;
    private final CdcSession cdcSession;

    ClientDCCache(String domainName, CdcSession cdcSession) {
        this.domainName = domainName; // nullable
        this.cdcSession = cdcSession; // nullable to allow for MockClientDCCache to extend this class
    }

    String getAvailableDC() throws OIDCClientException {
        CdcDCEntry entry;
        try {
            entry = this.cdcSession.getAffinitizedDC(this.domainName, CDC_FLAGS);
        } catch (CdcGenericException e) {
            throw new OIDCClientException("CDC exception", e);
        }
        if (entry == null) {
            throw new OIDCClientException("getAffinitizedDC returned null");
        }
        return entry.dcName;
    }

    @Override
    public void close() {
        this.cdcSession.close();
    }
}
