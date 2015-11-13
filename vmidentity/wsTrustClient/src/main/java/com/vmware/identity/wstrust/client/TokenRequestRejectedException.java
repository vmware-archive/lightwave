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
import com.vmware.vim.sso.client.exception.SsoException;

/**
 * This class represents an exception that occurred due to STS rejecting the
 * token request message. This happens in most of the cases because of a user
 * entering incorrect data in the request (for example incorrect user name or
 * password). The specific descendant type of exception expresses the rejection
 * reason.
 */
public abstract class TokenRequestRejectedException extends SsoException {

    private static final long serialVersionUID = 8168806664727128813L;

    public TokenRequestRejectedException(String message, Key messageKey, Object... messageDetails) {
        super(message, messageKey, null, messageDetails);
    }
}
