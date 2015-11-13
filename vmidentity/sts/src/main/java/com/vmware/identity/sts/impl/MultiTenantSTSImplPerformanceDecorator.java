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

package com.vmware.identity.sts.impl;

import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

import com.vmware.identity.performanceSupport.CallableDecorator;
import com.vmware.identity.performanceSupport.IPerfDataSink;
import com.vmware.identity.performanceSupport.PerfBucketKey;
import com.vmware.identity.performanceSupport.PerfMeasurementPoint;
import com.vmware.identity.performanceSupport.PerformanceDecorator;
import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.InvalidRequestException;
import com.vmware.identity.sts.MultiTenantSTS;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestExpiredException;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;
import com.vmware.identity.sts.util.MessageExtractionUtil;

/**
 * The class provides a decorated instance of {@code MultiTenantSTS}
 * with performance measurement support for its methods.
 *
 */
public final class MultiTenantSTSImplPerformanceDecorator
extends PerformanceDecorator
implements MultiTenantSTS
{
   private final MultiTenantSTS decorated;
   private final IPerfDataSink perfDataSink;

   /**
    * c'tor for the decorating {@code MultiTenantSTS}
    * @param aDecorated       cannot be null
    * @param aPerfDataSink    cannot be null
    */
   public MultiTenantSTSImplPerformanceDecorator(
         MultiTenantSTS aDecorated,
         IPerfDataSink aPerfDataSink) {
      assert aDecorated != null;
      assert aPerfDataSink != null;

      decorated = aDecorated;
      perfDataSink = aPerfDataSink;
   }

   @Override
   public RequestSecurityTokenResponseCollectionType challenge(
         final String tenantName,
         final RequestSecurityTokenResponseType requestSecurityTokenResponse,
         final SecurityHeaderType headerInfo) throws AuthenticationFailedException,
         UnsupportedSecurityTokenException, InvalidRequestException,
         RequestExpiredException, NoSuchIdPException, RequestFailedException {
      assert tenantName != null;
      assert requestSecurityTokenResponse != null;
      assert headerInfo != null;

      return exec(new CallableDecorator<RequestSecurityTokenResponseCollectionType>(){
                     @Override
                     public RequestSecurityTokenResponseCollectionType call(){
                        return decorated.challenge(
                              tenantName, requestSecurityTokenResponse, headerInfo);
                     }
                     @Override
                     public PerfBucketKey getPerfBucketKey() {

                        return new PerfBucketKey(
                                       PerfMeasurementPoint.SoapActionChallenge,
                                       MessageExtractionUtil.extractUsernameFromSecurityHeader(headerInfo));
                     }
                  },
                  perfDataSink);
   }

   @Override
   public RequestSecurityTokenResponseCollectionType issue(
         final String tenantName,
         final Request req) throws AuthenticationFailedException,
         UnsupportedSecurityTokenException, InvalidRequestException,
         RequestExpiredException, NoSuchIdPException, RequestFailedException {
      assert tenantName != null;
      assert req != null;

      return exec(new CallableDecorator<RequestSecurityTokenResponseCollectionType>(){
                     @Override
                     public RequestSecurityTokenResponseCollectionType call(){
                        return decorated.issue(tenantName, req);
                     }
                     @Override
                     public PerfBucketKey getPerfBucketKey() {
                        return new PerfBucketKey(
                              PerfMeasurementPoint.SoapActionIssue,
                              MessageExtractionUtil.extractUsernameFromSecurityHeader(
                                    req.getHeader()));
                     }
                  },
                  perfDataSink);
   }

   @Override
   public RequestSecurityTokenResponseType renew(
         final String tenantName,
         final Request req) throws AuthenticationFailedException,
         UnsupportedSecurityTokenException, InvalidRequestException,
         RequestExpiredException, NoSuchIdPException, RequestFailedException {
      assert tenantName != null;
      assert req != null;

      return exec(new CallableDecorator<RequestSecurityTokenResponseType>(){
                     @Override
                     public RequestSecurityTokenResponseType call(){
                        return decorated.renew(tenantName, req);
                     }
                     @Override
                     public PerfBucketKey getPerfBucketKey() {
                        return new PerfBucketKey(
                              PerfMeasurementPoint.SoapActionRenew,
                              MessageExtractionUtil.extractUsernameFromSecurityHeader(
                                    req.getHeader()));
                     }
                  },
                  perfDataSink);
   }

   @Override
   public RequestSecurityTokenResponseType validate(
         final String tenantName,
         final Request req) throws InvalidRequestException, RequestExpiredException,
         NoSuchIdPException, RequestFailedException {
      assert tenantName != null;
      assert req != null;

      return exec(new CallableDecorator<RequestSecurityTokenResponseType>() {
                     @Override
                     public RequestSecurityTokenResponseType call(){
                        return decorated.validate(tenantName, req);
                     }
                     @Override
                     public PerfBucketKey getPerfBucketKey() {
                        return new PerfBucketKey(
                              PerfMeasurementPoint.SoapActionValidate,
                              null);
                     }
                  },
                  perfDataSink);
   }
}
