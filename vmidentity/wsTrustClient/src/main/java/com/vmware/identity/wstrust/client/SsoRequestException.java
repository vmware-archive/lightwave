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
 * This class represents an internal error that had occurred in the client and
 * the client was unable to recover.
 */
public final class SsoRequestException extends SsoRuntimeException {

    private static final long serialVersionUID = 5484189818251277598L;

    public SsoRequestException(String message) {
        super(message);
    }

    public SsoRequestException(String message, Throwable cause) {
        super(message, cause);
    }

    public SsoRequestException(String logMessage, Key messageKey, Throwable cause, Object... messageDetails) {
        super(logMessage, messageKey, cause, messageDetails);
    }

}
