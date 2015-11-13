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
 * A client request was rejected very likely due to significant difference
 * between the time on the server and on the client
 *
 * <p>
 * By default, the request are guaranteed to fail if the difference the client
 * more than 10 min behind the server or 1 minute ahead of it.
 */
public final class TimeSynchronizationException extends SsoRuntimeException {

    /**
     * Creates a time synchronization error, deduced from an message expired
     * server error which contradicts the time frame recorded on the client
     *
     * @param message
     *            message with tech. details, intended for logging, required
     * @param roundtripEstimateSeconds
     *            the request roundtrip time, recorded by the client
     * @param requestValiditySeconds
     *            the request validity expected by the client
     */
    public TimeSynchronizationException(String message, long roundtripEstimateSeconds, long requestValiditySeconds) {
        super(message, Key.TIME_SYNCHRONIZATION_ERROR, null, roundtripEstimateSeconds, requestValiditySeconds);
    }

    /**
     * Creates a time synchronization error
     *
     * @param message
     *            message with tech. details, intended for logging, required
     * @param errorType
     *            identifies the type of time sync. error, required
     */
    public TimeSynchronizationException(String message, Key errorType) {
        super(message, errorType, null /* no inner exception */);
    }

    private static final long serialVersionUID = 7597268543083491051L;
}
