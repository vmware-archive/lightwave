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
 * Represents the case what the STS server response was not recognized as valid
 * SOAP message, containing a WS-Trust payload in a WS-Security envelope.
 */
public class MalformedResponseException extends SsoRuntimeException {

    private static final long serialVersionUID = -1156692791259969416L;

    public MalformedResponseException(String message, Key messageKey, Throwable cause) {
        super(message, messageKey, cause);
    }

}
