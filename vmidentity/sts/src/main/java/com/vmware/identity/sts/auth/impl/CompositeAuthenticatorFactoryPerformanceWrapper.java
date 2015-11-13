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

package com.vmware.identity.sts.auth.impl;

import org.apache.commons.lang.Validate;

import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.AuthenticatorFactory;

/**
 * Wrapper of an instance of {@code AuthenticatorFactory} to provide a
 *  decorated {@code CompositeAuthenticator} with performance
 *  measurement support.
 *
 */
public final class CompositeAuthenticatorFactoryPerformanceWrapper implements
      AuthenticatorFactory {

   private final AuthenticatorFactory wrapped;
   private final IPerfDataSink perfDataSink;

   /**
    * c'tor with the wrapped {@code AuthenticatorFactory} instance and
    * {@code IPerfDataSink} to use for performance measurement support.
    *
    * @param aWrapped          cannot be null
    * @param aPerfDataSink     cannot be null
    */
   public CompositeAuthenticatorFactoryPerformanceWrapper(
         AuthenticatorFactory aWrappedFactory,
         IPerfDataSink aPerfDataSink) {
      Validate.notNull(aWrappedFactory);
      Validate.notNull(aPerfDataSink);

      this.wrapped = aWrappedFactory;
      this.perfDataSink = aPerfDataSink;
   }

   @Override
   public Authenticator getAuthenticator(String tenantName,
         TokenValidator validator) {
      Validate.notNull(tenantName);

      return new CompositeAuthenticatorPerformanceDecorator(
            wrapped.getAuthenticator(tenantName, validator),
            perfDataSink);
   }
}
