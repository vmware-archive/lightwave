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

import com.vmware.identity.cdc.CdcFactory;
import com.vmware.identity.cdc.CdcGenericException;
import com.vmware.identity.cdc.CdcSession;

/**
 * @author Yehia Zayour
 */
class ClientDCCacheFactory {
    private final String domainName;

    ClientDCCacheFactory(String domainName) {
        this.domainName = domainName; // nullable
    }

    ClientDCCache createSession() throws OIDCClientException {
        CdcSession cdcSession;
        try {
            cdcSession = CdcFactory.createCdcSessionViaIPC(); // connect to local vmafd daemon
        } catch (CdcGenericException e) {
            throw new OIDCClientException("CDC exception", e);
        }
        return new ClientDCCache(this.domainName, cdcSession);
    }
}