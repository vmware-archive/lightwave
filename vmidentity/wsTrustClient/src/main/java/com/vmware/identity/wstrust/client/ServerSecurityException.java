/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client;

import com.vmware.vim.sso.client.BundleMessageSource.Key;
import com.vmware.vim.sso.client.exception.SsoRuntimeException;

/**
 * Signals that a client request has been rejected by the server because of a
 * perceived tampering or other issue with the security envelope of the received
 * message.
 *
 * <p>
 * Corresponds to wsse:InvalidSecurity fault code
 */
public class ServerSecurityException extends SsoRuntimeException {

    /**
     * Creates a new exception
     *
     * @param message
     *            message to appear in logs, unlocalized, required
     * @param serverMessage
     *            message to appear in logs, possibly unlocalized, required
     */
    public ServerSecurityException(String message, String serverMessage) {
        super(message, Key.POTENITAL_TAMPERING_OF_REQUEST, null, serverMessage);
    }

    private static final long serialVersionUID = -2969293111881297088L;
}
