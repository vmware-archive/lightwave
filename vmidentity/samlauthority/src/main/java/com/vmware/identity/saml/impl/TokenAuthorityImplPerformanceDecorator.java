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
import com.vmware.identity.saml.SamlToken;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.util.ExtractionUtil;

/**
 * The class provides a decorated instance of {@code TokenAuthority}
 * with performance measurement support for its method.
 */
public final class TokenAuthorityImplPerformanceDecorator extends
        PerformanceDecorator implements TokenAuthority
{
    private final TokenAuthority decorated;
    private final IPerfDataSink perfDataSink;

    /**
     * c'tor for the decorating {@code TokenAuthority}
     * @param aDecorated       cannot be null
     * @param aPerfDataSink    cannot be null
     */
    public TokenAuthorityImplPerformanceDecorator(
            TokenAuthority aDecorated,
            IPerfDataSink aPerfDataSink){
        Validate.notNull(aDecorated);
        Validate.notNull(aPerfDataSink);

        this.decorated = aDecorated;
        this.perfDataSink = aPerfDataSink;

    }

    @Override
    public SamlToken issueToken(final SamlTokenSpec spec){
        Validate.notNull(spec);

        return exec(new CallableDecorator<SamlToken>() {
                        @Override
                        public SamlToken call() {
                            return decorated.issueToken(spec);
                        }
                        @Override
                        public PerfBucketKey getPerfBucketKey()
                        {
                            return new PerfBucketKey(
                                    PerfMeasurementPoint.IssueToken,
                                    ExtractionUtil.extractDomainNameFromTokenSpec(spec));
                        }
                    },
                    perfDataSink);
    }

}
