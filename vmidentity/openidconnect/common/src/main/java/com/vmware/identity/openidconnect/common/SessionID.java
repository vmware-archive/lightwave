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

package com.vmware.identity.openidconnect.common;

import com.nimbusds.oauth2.sdk.id.Identifier;

/**
 * @author Yehia Zayour
 */
public final class SessionID extends Identifier {
    private static final long serialVersionUID = 2015_05_29L;

    public SessionID(String value) {
        super(value);
    }

    public SessionID(int byteLength) {
        super(byteLength);
    }

    public SessionID() {
        super();
    }

    @Override
    public boolean equals(Object object) {
        return object instanceof SessionID && this.toString().equals(object.toString());
    }
}
