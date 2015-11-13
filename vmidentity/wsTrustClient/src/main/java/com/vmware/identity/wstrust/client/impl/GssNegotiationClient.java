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

import java.util.concurrent.Future;

import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.SamlToken;
import com.vmware.identity.wstrust.client.AsyncHandler;
import com.vmware.identity.wstrust.client.CertificateValidationException;
import com.vmware.identity.wstrust.client.ServerCommunicationException;
import com.vmware.identity.wstrust.client.SsoRequestException;
import com.vmware.identity.wstrust.client.TimeSynchronizationException;
import com.vmware.identity.wstrust.client.TokenRequestRejectedException;

/**
 * Encapsulates the state of a GSS token negotiation. Instances of this
 * interface can be used to obtain token from STS using GSS authentication
 */
public interface GssNegotiationClient {

    /**
     * Sends the user's current authentication leg to the SSO server and returns
     * the server's authentication leg and security token (if the negotiation is
     * complete and the server has successfully authenticated the user).
     *
     * @param leg
     *            The user's authentication leg. Cannot be {@code null}
     * @return The not {@code null} result of the roundtrip.
     * @throws InvalidTokenException
     *             If the returned token is syntactically invalid.
     * @throws TokenRequestRejectedException
     *             If the request for token failed.
     * @throws SsoRequestException
     *             If some not user-induced error occurs.
     * @throws ServerCommunicationException
     *             If there is a problem during the communication with the
     *             remote server
     * @throws CertificateValidationException
     *             If the SSL authentication against the STS fails
     * @throws TimeSynchronizationException
     *             If the request fails due to a detected difference between the
     *             client and server time
     */
    public GssNegotiationResult negotiateToken(byte[] leg) throws InvalidTokenException, TokenRequestRejectedException,
            SsoRequestException, ServerCommunicationException, CertificateValidationException,
            TimeSynchronizationException;

    /**
     * Asynchronous with completion callback version of the
     * {@link #negotiateToken(byte[])} method
     *
     * @param leg
     *            The user's authentication leg. Cannot be {@code null}
     * @param handler
     *            Asynchronous completion callback. Cannot be {@code null}.
     *            Callers not interested in a completion callback should use
     *            {@link #negotiateTokenAsync(byte[])}
     * @return Future that represents the result of the asynchronous call
     * @see {@link #negotiateToken}, {@link SsoAsyncHandler}
     */
    public Future<GssNegotiationResult> negotiateTokenAsync(byte[] leg, AsyncHandler<GssNegotiationResult> handler);

    /**
     * Asynchronous without completion callback version of the
     * {@link #negotiateToken(byte[])} method.
     *
     * @param leg
     *            The user's authentication leg. Cannot be {@code null}
     * @return Future that represents the result of the asynchronous call
     **/
    public Future<GssNegotiationResult> negotiateTokenAsync(byte[] leg);

    /**
     * Represents the result of a single GSS roundtrip
     */
    public static interface GssNegotiationResult {
        /**
         * The security token returned by the server. This will only be
         * {@code non-NULL} if the negotiation has completed and the server has
         * authenticated the user.
         */
        public SamlToken getToken();

        /** The negotiation leg returned by the server */
        public byte[] getLeg();
    }

}
