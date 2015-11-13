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

import com.vmware.identity.performanceSupport.CallableDecorator;
import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.performanceSupport.PerfBucketKey;
import com.vmware.identity.performanceSupport.PerfMeasurementPoint;
import com.vmware.identity.performanceSupport.PerformanceDecorator;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;
import com.vmware.identity.sts.util.MessageExtractionUtil;

/**
 * The class provides a decorated instance of {@code Authenticator}
 * with performance measurement support
 *
 */
final class CompositeAuthenticatorPerformanceDecorator
extends PerformanceDecorator
implements Authenticator {

   private final Authenticator decorated;
   private final IPerfDataSink perfDataSink;

   /**
    * c'tor for the decorating {@code Authenticator}
    * @param aDecorated       cannot be null
    * @param aPerfDataSink    cannot be null
    */
   public CompositeAuthenticatorPerformanceDecorator(
         Authenticator aDecorated,
         IPerfDataSink aPerfDataSink) {
      assert aDecorated != null;
      assert aPerfDataSink != null;

      decorated = aDecorated;
      perfDataSink = aPerfDataSink;
   }

   @Override
   public Result authenticate(final Request req)
         throws AuthenticationFailedException,
         UnsupportedSecurityTokenException, NoSuchIdPException,
         RequestFailedException {
      assert req != null;

      return exec(new CallableDecorator<Result>() {
                     @Override
                     public Result call() {
                        return decorated.authenticate(req);
                     }
                     @Override
                     public PerfBucketKey getPerfBucketKey() {

                        return new PerfBucketKey(
                              PerfMeasurementPoint.CompositeAuthenticate,
                              MessageExtractionUtil.extractUsernameFromSecurityHeader(
                                    req.getHeader()));
                     }
                  },
                  perfDataSink);
   }
}
