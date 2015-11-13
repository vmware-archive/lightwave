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
package com.vmware.identity.idm.server.provider;

import com.vmware.identity.interop.idm.UserInfo;

public class GSSAuthResult {
    private final String contextId;
    private final UserInfo userInfo;
    private final byte[] serverLeg;

    public GSSAuthResult(String contextId, UserInfo userInfo) {

        this.userInfo = userInfo;
        this.contextId = contextId;
        this.serverLeg = null;
     }

    public GSSAuthResult(String contextId, byte[] serverLeg) {

        this.userInfo = null;
        this.contextId = contextId;
        this.serverLeg = serverLeg;
     }

    public String getContextId() {
        return contextId;
    }

    public UserInfo getUserInfo() {
        return userInfo;
    }

    public byte[] getServerLeg() {
        return serverLeg;
    }
}
