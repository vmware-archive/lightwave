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

import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.InvalidSignatureException;
import com.vmware.identity.sts.MultiTenantSTS;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.STS;
import com.vmware.identity.sts.STSFactory;

public final class MultiTenantSTSImpl implements MultiTenantSTS {

   private final STSFactory stsFactory;

   public MultiTenantSTSImpl(STSFactory stsFactory) {
      assert stsFactory != null;
      this.stsFactory = stsFactory;
   }

   @Override
   public RequestSecurityTokenResponseCollectionType challenge(String tenantName,
      RequestSecurityTokenResponseType requestSecurityTokenResponse,
      SecurityHeaderType headerInfo) {
      assert tenantName != null;
      assert requestSecurityTokenResponse != null;
      assert headerInfo != null;

      STS sts = stsFactory.getSTS(tenantName);
      try {
         return sts.challenge(requestSecurityTokenResponse, headerInfo);
      } catch (InvalidSignatureException e) {
         throw new InvalidCredentialsException(e.getMessage(), e);
      }
   }

   @Override
   public RequestSecurityTokenResponseCollectionType issue(String tenantName,
      Request req) {
      assert tenantName != null;
      assert req != null;

      STS sts = stsFactory.getSTS(tenantName);
      try {
         return sts.issue(req);
      } catch (InvalidSignatureException e) {
         throw new InvalidCredentialsException(e.getMessage(), e);
      }
   }

   @Override
   public RequestSecurityTokenResponseType renew(String tenantName, Request req) {
      assert tenantName != null;
      assert req != null;

      STS sts = stsFactory.getSTS(tenantName);
      try {
         return sts.renew(req);
      } catch (InvalidSignatureException e) {
         throw new InvalidCredentialsException(e.getMessage(), e);
      }
   }

   @Override
   public RequestSecurityTokenResponseType validate(String tenantName,
      Request req) {
      assert tenantName != null;
      assert req != null;

      STS sts = stsFactory.getSTS(tenantName);
      try {
         return sts.validate(req);
      } catch (InvalidSignatureException e) {
         return RSTRBuilder.createValidateResponse(req, false);
      }
   }

}
