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
package com.vmware.identity.wstrust.client.impl;

import java.util.concurrent.Callable;

import com.vmware.vim.sso.client.exception.SsoException;
import com.vmware.vim.sso.client.exception.SsoRuntimeException;
import com.vmware.identity.wstrust.client.AsyncHandler;

/**
 * This class encapsulates logic related to how to create the {@link Callable}
 * needed for async methods
 */
abstract class AsyncCommand<T> implements Callable<T> {

    private final AsyncHandler<T> handler;

    protected AsyncCommand(AsyncHandler<T> handler) {
        this.handler = handler;
    }

    @Override
    public T call() throws Exception {
        T result = null;
        try {
            result = executeAction();
        } catch (Exception e) {
            handler.handleException(e);
            throw e; // re-throw so the future can get the exception
        }

        handler.handleResponse(result);
        return result;
    }

    /**
     * Executes the action needed for the current request
     *
     * @throws SsoException
     * @throws SsoRuntimeException
     */
    protected abstract T executeAction() throws SsoException;
}
