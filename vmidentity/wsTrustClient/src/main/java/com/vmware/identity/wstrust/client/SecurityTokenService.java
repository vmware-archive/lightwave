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

import java.util.concurrent.Future;

import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.SamlToken;

/**
 * The Security Token Service interface defines several methods for obtaining
 * and validating SAML tokens. Before using the methods
 * SecurityTokenServiceConfig should be properly initialized.
 * <p>
 * Note: Everywhere in this class the expected format for <code>subject</code>
 * parameter value is as follows:
 * <ul>
 * <li>UPN formatted string for remote users</li>
 * <li>just the name of the SSO users that are members of the default domain</li>
 * </ul>
 */
public interface SecurityTokenService {

    /**
     * Acquires an authentication token from the token service.
     *
     * @param clientCredential
     *            The client credential for which the token will be issued.
     *            Cannot be <code>null</code>
     * @param tokenSpec
     *            The requested properties of the authentication token. Note
     *            that the properties of the token returned will not necessarily
     *            match the requested properties. Cannot be <code>null</code>
     *
     * @return The authentication token. Cannot be <code>null</code>
     *
     * @throws InvalidTokenException
     *             when a valid token could not be extracted from the server
     *             response
     * @throws TokenRequestRejectedException
     *             when the server rejects the token request, usually due to
     *             incorrect credentials
     * @throws SsoRequestException
     *             when an internal error had occurred in the client
     * @throws ServerCommunicationException
     *             when server is missing, has failed to respond or responded in
     *             an unexpected way
     * @throws ServerSecurityException
     *             when server found errors in security metadata of the request
     *             possibly caused by tampering during transport
     * @throws RequestExpiredException
     *             when the server rejects a request with expired lifetime
     * @throws CertificateValidationException
     *             when cannot establish SSL connection with the STS server
     *             because STS SSL certificate is not trusted
     * @throws TimeSynchronizationException
     *             when the request fails due to a detected difference between
     *             the client and server time
     */
    public SamlToken acquireToken(Credential clientCredential, TokenSpec tokenSpec) throws InvalidTokenException,
            TokenRequestRejectedException, SsoRequestException, ServerCommunicationException, ServerSecurityException,
            RequestExpiredException, CertificateValidationException, TimeSynchronizationException;

    /**
     * Acquires asynchronously an authentication token from the token service.
     *
     * @param credential
     *            The client credential for which the token will be issued.
     *            Cannot be <code>null</code>
     * @param password
     *            The password of the authenticating subject. Cannot be
     *            <code>null</code>
     * @param tokenSpec
     *            The requested properties of the authentication token. Note
     *            that the properties of the token returned will not necessarily
     *            match the requested properties. Cannot be <code>null</code>
     * @param asyncHandler
     *            callback parameter. Cannot be <code>null</code>
     *
     * @return Future that represents the result of the asynchronous call.
     *         Cannot be <code>null</code>
     */
    public Future<SamlToken> acquireTokenAsync(Credential clientCredential, TokenSpec tokenSpec,
            AsyncHandler<SamlToken> asyncHandler);

    /**
     * Acquires asynchronously an authentication token from the token service.
     *
     * @param credential
     *            The client credential for which the token will be issued.
     *            Cannot be <code>null</code>
     * @param password
     *            The password of the authenticating subject. Cannot be
     *            <code>null</code>
     * @param tokenSpec
     *            The requested properties of the authentication token. Note
     *            that the properties of the token returned will not necessarily
     *            match the requested properties. Cannot be <code>null</code>
     *
     * @return Future that represents the result of the asynchronous call.
     *         Cannot be <code>null</code>
     */
    public Future<SamlToken> acquireTokenAsync(Credential clientCredential, TokenSpec tokenSpec);

    /**
     * Validates a token. The authentication service will be contacted to verify
     * that the subject in the token is still valid. Service providers should
     * always validate the subjects of tokens that may have been retrieved from
     * persistent storage.
     *
     * @param token
     *            The token to validate. Cannot be <code>null</code>
     *
     * @return true if the token is valid, false otherwise.
     *
     * @throws InvalidTokenException
     *             when a valid token could not be extracted from the server
     *             response
     * @throws TokenRequestRejectedException
     *             when the server rejects the token request, usually due to
     *             incorrect credentials
     * @throws SsoRequestException
     *             when an internal error had occurred in the client
     * @throws ServerCommunicationException
     *             when server is missing, has failed to respond or responded in
     *             an unexpected way
     * @throws ServerSecurityException
     *             when server found errors in security metadata of the request
     *             possibly caused by tampering during transport
     * @throws RequestExpiredException
     *             when the server rejects a request with expired lifetime
     * @throws CertificateValidationException
     *             when cannot establish SSL connection with the STS server
     *             because STS SSL certificate is not trusted
     * @throws TimeSynchronizationException
     *             when the request fails due to a detected difference between
     *             the client and server time
     */
    public boolean validateToken(SamlToken token) throws InvalidTokenException, TokenRequestRejectedException,
            SsoRequestException, ServerCommunicationException, ServerSecurityException, RequestExpiredException,
            CertificateValidationException, TimeSynchronizationException;

    /**
     * Validates asynchronously a token. The authentication service will be
     * contacted to verify that the subject in the token is still valid. Service
     * providers should always validate the subjects of tokens that may have
     * been retrieved from persistent storage.
     *
     * @param token
     *            The token to validate. Cannot be <code>null</code>
     * @param asyncHandler
     *            callback parameter. Cannot be <code>null</code>
     *
     * @return Future that represents the result of the asynchronous call.
     *         Cannot be <code>null</code>
     */
    public Future<Boolean> validateTokenAsync(SamlToken token, AsyncHandler<Boolean> asyncHandler);

    /**
     * Validates asynchronously a token. The authentication service will be
     * contacted to verify that the subject in the token is still valid. Service
     * providers should always validate the subjects of tokens that may have
     * been retrieved from persistent storage.
     *
     * @param token
     *            The token to validate. Cannot be <code>null</code>
     *
     * @return Future that represents the result of the asynchronous call.
     *         Cannot be <code>null</code>
     */
    public Future<Boolean> validateTokenAsync(SamlToken token);

    /**
     * Requests renewing the token from the token service. Requesters should
     * demonstrate that they are authorized to use the token by signing the
     * request. Only holder of key tokens can be renewed.
     *
     * @param token
     *            The token to renew. Cannot be null.
     * @param tokenLifetimeSec
     *            The requested lifetime period for the 'new' token in seconds
     *            from now; positive number is required
     * @return The renewed token. Cannot be null.
     *
     * @throws InvalidTokenException
     *             when a valid token could not be extracted from the server
     *             response
     * @throws TokenRequestRejectedException
     *             when the server rejects the token request, usually due to
     *             incorrect credentials
     * @throws SsoRequestException
     *             when an internal error had occurred in the client
     * @throws ServerCommunicationException
     *             when server is missing, has failed to respond or responded in
     *             an unexpected way
     * @throws ServerSecurityException
     *             when server found errors in security metadata of the request
     *             possibly caused by tampering during transport
     * @throws RequestExpiredException
     *             when the server rejects a request with expired lifetime
     * @throws CertificateValidationException
     *             when cannot establish SSL connection with the STS server
     *             because STS SSL certificate is not trusted
     * @throws TimeSynchronizationException
     *             when the request fails due to a detected difference between
     *             the client and server time
     */
    public SamlToken renewToken(SamlToken token, long tokenLifetimeSec) throws InvalidTokenException,
            TokenRequestRejectedException, SsoRequestException, ServerCommunicationException, ServerSecurityException,
            RequestExpiredException, CertificateValidationException, TimeSynchronizationException;

    /**
     * Asynchronous version of {@link #renewToken(SamlToken, long)}. All
     * constraints defined in the synchronous method version also apply here.
     *
     * @param token
     *            The token to renew. Cannot be null.
     * @param tokenLifetimeSec
     *            token lifetime period in seconds from now; positive number is
     *            required
     * @param asyncHandler
     *            callback parameter. Cannot be null.
     * @return Future that represents the result of the asynchronous call.
     *         Cannot be null.
     */
    public Future<SamlToken> renewTokenAsync(SamlToken token, long tokenLifetimeSec,
            AsyncHandler<SamlToken> asyncHandler);

    /**
     * Asynchronous version of {@link #renewToken(SamlToken, long)}. All
     * constraints defined in the synchronous method version also apply here.
     *
     * @param token
     *            The token to renew. Cannot be null.
     * @param tokenLifetimeSec
     *            token lifetime period in seconds from now; positive number is
     *            required
     * @return Future that represents the result of the asynchronous call.
     *         Cannot be null.
     */
    public Future<SamlToken> renewTokenAsync(SamlToken token, long tokenLifetimeSec);
}
