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

package com.vmware.identity.openidconnect.server;

/**
 * @author Yehia Zayour
 */
public enum Flow {
    AUTHZ_CODE,
    IMPLICIT,
    IMPLICIT_ID_TOKEN_ONLY,
    PASSWORD,
    SOLUTION_USER_CREDS,
    CLIENT_CREDS,
    PERSON_USER_CERT,
    GSS_TICKET,
    SECURID,
    REFRESH_TOKEN;

    public boolean isImplicit() {
        return this.equals(IMPLICIT) || this.equals(IMPLICIT_ID_TOKEN_ONLY);
    }

    public boolean isAuthzEndpointFlow() {
        return
                this.equals(Flow.AUTHZ_CODE) ||
                this.equals(Flow.IMPLICIT) ||
                this.equals(Flow.IMPLICIT_ID_TOKEN_ONLY);
    }

    public boolean isTokenEndpointFlow() {
        return
                this.equals(Flow.AUTHZ_CODE) ||
                this.equals(Flow.PASSWORD) ||
                this.equals(Flow.CLIENT_CREDS) ||
                this.equals(Flow.SOLUTION_USER_CREDS) ||
                this.equals(Flow.PERSON_USER_CERT) ||
                this.equals(Flow.GSS_TICKET) ||
                this.equals(Flow.SECURID) ||
                this.equals(Flow.REFRESH_TOKEN);
    }
}