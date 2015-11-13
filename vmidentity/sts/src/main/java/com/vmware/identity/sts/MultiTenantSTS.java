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
package com.vmware.identity.sts;

import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseCollectionType;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenResponseType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;

public interface MultiTenantSTS {

   /**
    * @param tenantName
    *           of the current tenant executing the request. Cannot be null.
    * @param requestSecurityTokenResponse
    *           challenge request body. Cannot be null.
    * @param headerInfo
    *           request header. Cannot be null.
    * @throws AuthenticationFailedException
    *            when the user is not authenticated
    * @throws UnsupportedSecurityTokenException
    *            if the passed in security token is not supported
    * @throws InvalidRequestException
    *            if the request is invalid or malformed
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws NoSuchIdPException
    *            when there is no such IdP existing
    * @throws RequestFailedException
    *            if a system failure occurred
    * @return not null challenge response
    */
   public RequestSecurityTokenResponseCollectionType challenge(String tenantName,
      RequestSecurityTokenResponseType requestSecurityTokenResponse,
      SecurityHeaderType headerInfo) throws AuthenticationFailedException,
      UnsupportedSecurityTokenException, InvalidRequestException,
      RequestExpiredException, NoSuchIdPException, RequestFailedException;

   /**
    * @param tenantName
    *           of the current tenant executing the request. Cannot be null.
    * @param req
    *           issue request. Cannot be null.
    * @throws AuthenticationFailedException
    *            when the user is not authenticated
    * @throws UnsupportedSecurityTokenException
    *            if the passed in security token is not supported
    * @throws InvalidRequestException
    *            if the request is invalid or malformed
    * @throws UnableToRenewException
    *            if renew request cannot be satisfied when the token is not
    *            renewable.
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws NoSuchIdPException
    *            when there is no such IdP existing
    * @throws RequestFailedException
    *            if a system failure occurred
    * @return not null issue response
    */
   public RequestSecurityTokenResponseCollectionType issue(String tenantName,
      Request req) throws AuthenticationFailedException,
      UnsupportedSecurityTokenException, InvalidRequestException,
      UnableToRenewException, RequestExpiredException, NoSuchIdPException,
      RequestFailedException;

   /**
    * @param tenantName
    *           of the current tenant executing the request. Cannot be null.
    * @param req
    *           renew request. Cannot be null.
    * @throws AuthenticationFailedException
    *            when the user is not authenticated
    * @throws UnsupportedSecurityTokenException
    *            if the passed in security token is not supported
    * @throws InvalidRequestException
    *            if the request is invalid or malformed
    * @throws UnableToRenewException
    *            if renew request cannot be satisfied when the token is not
    *            renewable.
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws NoSuchIdPException
    *            when there is no such IdP existing
    * @throws RequestFailedException
    *            if a system failure occurred
    * @return not null renew response
    */
   public RequestSecurityTokenResponseType renew(String tenantName, Request req)
      throws AuthenticationFailedException, UnsupportedSecurityTokenException,
      InvalidRequestException, UnableToRenewException, RequestExpiredException,
      NoSuchIdPException, RequestFailedException;

   /**
    * @param tenantName
    *           of the current tenant executing the request. Cannot be null.
    * @param req
    *           validate request. Cannot be null.
    * @throws InvalidRequestException
    *            if the request is invalid or malformed
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws NoSuchIdPException
    *            when there is no such IdP existing
    * @throws RequestFailedException
    *            if a system failure occurred
    * @return not null validate response
    */
   public RequestSecurityTokenResponseType validate(String tenantName,
      Request req) throws InvalidRequestException, RequestExpiredException,
      NoSuchIdPException, RequestFailedException;

}
