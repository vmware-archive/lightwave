/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
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
 */

package com.vmware.identity.saml;

import org.apache.commons.lang.Validate;

import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.saml.impl.TokenAuthorityImplPerformanceDecorator;
import com.vmware.identity.saml.impl.TokenValidatorImplPerformanceDecorator;

/**
 * Wrapper of an instance of {@code SamlAuthorityFactory} to provide
 * decorated {@code TokenAuthority}, {@code TokenValidator} and members of
 * {@code TokenServices} with performance measurement support.
 *
 */
public final class SamlAuthorityFactoryPerformanceWrapper implements SamlAuthorityFactory
{
    private final SamlAuthorityFactory wrapped;
    private final IPerfDataSink perfDataSink;

    /**
     * c'tor with the wrapped {@code SamlAuthorityFactory} instance and
     * {@code IPerfDataSink} to use for performance measurement support.
     *
     * @param aWrapped          cannot be null
     * @param aPerfDataSink     cannot be null
     */
    public SamlAuthorityFactoryPerformanceWrapper(
            SamlAuthorityFactory aWrapped,
            IPerfDataSink aPerfDataSink) {
        Validate.notNull(aWrapped);
        Validate.notNull(aPerfDataSink);

        wrapped = aWrapped;
        perfDataSink = aPerfDataSink;
    }

    @Override
    public TokenAuthority createTokenAuthority(String tenantName)
       throws NoSuchIdPException, SystemException {
       Validate.notEmpty(tenantName);

       return new TokenAuthorityImplPerformanceDecorator(
               wrapped.createTokenAuthority(tenantName),
               perfDataSink);
    }

    @Override
    public TokenValidator createTokenValidator(String tenantName)
       throws NoSuchIdPException, SystemException {
       Validate.notEmpty(tenantName);

       return new TokenValidatorImplPerformanceDecorator(
               wrapped.createTokenValidator(tenantName),
               perfDataSink);
    }

    @Override
    public TokenValidator createAuthnOnlyTokenValidator(String tenantName)
    throws NoSuchIdPException, SystemException {
        return new TokenValidatorImplPerformanceDecorator(
               wrapped.createAuthnOnlyTokenValidator(tenantName),
               perfDataSink);
    }

    @Override
    public TokenServices createTokenServices(String tenantName)
       throws NoSuchIdPException, SystemException {
        Validate.notEmpty(tenantName);

        return new TokenServices(createTokenAuthority(tenantName),
          createTokenValidator(tenantName),createAuthnOnlyTokenValidator(tenantName));
    }
}
