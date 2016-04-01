/*
 *
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 *
 */
package com.vmware.identity.auth.passcode.spi;

/**
 * This class defines the Service Provider Interface (SPI) for
 * AuthenticationSession class. All the abstract methods in this class must be
 * implemented by the service provider who supplies the implementation of
 * passcode authentication.
 *
 * @author aantochi
 *
 */
public interface AuthenticationSession {

    /**
     * Authenticate user based on the user id and shared secret
     *
     * @param userId
     *            The userId for used for authentication
     * @param passcode
     *            contains the passcode for authentication
     * @return returns AuthenticationResult which contains the authentication
     *         status code
     *
     * @throws AuthenticationException
     */
    public AuthenticationResult authenticate(String userId, AuthenticationSecret passcode)
            throws AuthenticationException;

    /**
     * Method to get the authentication status of the authentication session
     *
     * @return
     * @throws AuthenticationException
     */
    public AuthenticationResult getAuthenticationStatus() throws AuthenticationException;

    /**
     * Method to provide the next authentication secret for multi step
     * authentication
     *
     * @param passcode
     * @return
     * @throws AuthenticationException
     */
    public AuthenticationResult nextAuthenticationStep(AuthenticationSecret passcode) throws AuthenticationException;

    public void closeSession() throws AuthenticationException;

}
