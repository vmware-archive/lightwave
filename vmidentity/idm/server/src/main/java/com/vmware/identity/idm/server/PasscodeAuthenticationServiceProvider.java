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
package com.vmware.identity.idm.server;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.ServiceLoader;

import com.vmware.identity.auth.passcode.spi.AuthenticationException;
import com.vmware.identity.auth.passcode.spi.AuthenticationSessionFactory;
import com.vmware.identity.auth.passcode.spi.SessionFactoryProperties;
import com.vmware.identity.auth.passcode.spi.SessionFactoryProvider;

/**
 * This class provides the means to get an instances of the
 * AuthenticationSessionFactory implementation.
 *
 * @author aantochi
 *
 */
public class PasscodeAuthenticationServiceProvider {

    private static final PasscodeAuthenticationServiceProvider INSTANCE = new PasscodeAuthenticationServiceProvider();
    private ServiceLoader<SessionFactoryProvider> loader;

    private PasscodeAuthenticationServiceProvider() {
        loader = ServiceLoader.load(SessionFactoryProvider.class);
    }

    public static PasscodeAuthenticationServiceProvider getInstance() {
        return INSTANCE;
    }

    /**
     * This method returns the list of registered providers of the specific
     * type.
     *
     * @param type
     *            - The provider type
     * @return - list of the providers that correspond to the type
     */
    public List<SessionFactoryProvider> getProviders(String type) {
        Iterator<SessionFactoryProvider> iterator = loader.iterator();
        List<SessionFactoryProvider> providers = new ArrayList<>();

        while (iterator.hasNext()) {
            SessionFactoryProvider factory = iterator.next();
            if (factory.getType().equalsIgnoreCase(type)) {
                providers.add(factory);
            }
        }

        return providers;
    }

    /**
     * This method traverses the list of registered providers and a new object
     * that implements the AuthenticationSessionFactory from the first provider
     * that supports the specified type is returned.
     *
     * @param type
     *            - The provider type
     * @param settings
     *            - Settings used for initializing the authentication session
     *            factory implementation
     * @return - AuthenticationSessionFactory implementation
     * @throws AuthenticationException
     */
    public AuthenticationSessionFactory getAuthenticationSessionFactory(String type, SessionFactoryProperties settings)
            throws AuthenticationException {
        Iterator<SessionFactoryProvider> iterator = loader.iterator();

        while (iterator.hasNext()) {
            SessionFactoryProvider factory = iterator.next();
            if (factory.getType().equalsIgnoreCase(type)) {
                return factory.createSessionFactoryInstance(settings);
            }
        }

        throw new AuthenticationException("No provider found");
    }
}
