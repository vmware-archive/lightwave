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

/**
 * Generic interface for async handler functions passed to SecurityTokenService
 * async methods.
 *
 * @param <T>
 */
public interface AsyncHandler<T> {

    /**
     * This method is invoked when the response is received and is ready to be
     * further processed
     *
     * @param response
     */
    public void handleResponse(T response);

    /**
     * This method is invoked if an exception is received
     *
     * @param exception
     *            - exception from async method execution
     *
     */
    public void handleException(Exception exception);
}
