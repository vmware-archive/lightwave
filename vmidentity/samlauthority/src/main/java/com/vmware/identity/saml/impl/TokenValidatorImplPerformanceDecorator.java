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

package com.vmware.identity.saml.impl;


import org.apache.commons.lang.Validate;

import com.vmware.identity.performanceSupport.CallableDecorator;
import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.performanceSupport.PerfBucketKey;
import com.vmware.identity.performanceSupport.PerfMeasurementPoint;
import com.vmware.identity.performanceSupport.PerformanceDecorator;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.InvalidSignatureException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.util.ExtractionUtil;

/**
 * This class provides a decorated instance of {@code TokenValidator}
 * with performance measurement support
 *
 */
public final class TokenValidatorImplPerformanceDecorator
extends PerformanceDecorator
implements TokenValidator
{
    private final TokenValidator decorated;
    private final IPerfDataSink perfDataSink;

    /**
     * c'tor for the decorating {@code TokenValidator}
     * @param aDecorated       cannot be null
     * @param aPerfDataSink    cannot be null
     */
    public TokenValidatorImplPerformanceDecorator(
            TokenValidator aDecorated,
            IPerfDataSink aPerfDataSink) {
        Validate.notNull(aDecorated);
        Validate.notNull(aPerfDataSink);

        decorated = aDecorated;
        perfDataSink =aPerfDataSink;
    }

    @Override
    public ServerValidatableSamlToken validate(final ServerValidatableSamlToken token)
            throws InvalidSignatureException, InvalidTokenException,
            SystemException {
        Validate.notNull(token);

        return exec(new CallableDecorator<ServerValidatableSamlToken>() {

                        @Override
                        public ServerValidatableSamlToken call() {
                            return decorated.validate(token);
                        }

                        @Override
                        public PerfBucketKey getPerfBucketKey()
                        {
                            return new PerfBucketKey(
                                        PerfMeasurementPoint.Validate,
                                        ExtractionUtil.extractDomainNameFromSamlToken(token));
                        }
                    },
                    perfDataSink);
    }
}
