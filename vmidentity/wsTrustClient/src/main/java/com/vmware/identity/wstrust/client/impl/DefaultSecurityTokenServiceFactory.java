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

import com.vmware.identity.wstrust.client.SecurityTokenService;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig;

/**
 * Factory class for obtaining {@link SecurityTokenService} implementations.
 * These implementations are not thread safe - use appropriate
 * synchronization/pooling.
 */
public final class DefaultSecurityTokenServiceFactory {

    /**
     * Returns a SecurityTokenService implementation. This interface provides
     * methods for various token related operations (i.e. acquireToken,
     * validateToken etc.)
     *
     * <p>
     * The SecurityTokenService implementation will always throw exceptions with
     * localized {@link Exception#getMessage()} text, using the locale from the
     * <code>config</code> parameter.
     *
     * @param config
     *            Cannot be <code>null</code>
     * @return SecurityTokenService instance. Cannot be <code>null</code>
     */
    public static SecurityTokenService getSecurityTokenService(SecurityTokenServiceConfig config) {

        return new SecurityTokenServiceImpl(new SoapBindingImpl(config.getConnectionConfig()
                .getSSLTrustedManagerConfig()), config);
    }

    private DefaultSecurityTokenServiceFactory() {
    }

}
