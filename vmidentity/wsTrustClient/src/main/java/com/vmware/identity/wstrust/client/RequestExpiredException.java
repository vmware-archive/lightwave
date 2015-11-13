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
 * Signals that a client request has been rejected by the server because of
 * expired lifetime of the request
 *
 * <p>
 * Communication to the SSO Server is protected from replay attacks with several
 * measures, incl. invalidating requests older by a period of time (10 minutes
 * by default)
 *
 * <p>
 * Corresponds to wsse:MessageExpired fault code
 */
public class RequestExpiredException extends SsoRuntimeException {

    public RequestExpiredException(String message) {
        super(message, Key.REQUEST_EXPIRED, null);
    }

    private static final long serialVersionUID = 1631480979926496345L;
}
