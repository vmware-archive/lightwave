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

/**
 * Implementation of this interface will be responsible for constructing
 * responses for all WS-Trust actions by given requests.
 */
public interface STS {

   /**
    *
    * @param requestSecurityTokenResponse
    *           challenge request body, mandatory.
    * @param headerInfo
    *           request header, mandatory.
    * @throws InvalidSignatureException
    *            when an authentication is via token and its signature cannot be
    *            validated against a set of trusted certificates
    * @throws AuthenticationFailedException
    *            when the user is not authenticated
    * @throws UnsupportedSecurityTokenException
    *            if the passed in security token is not supported
    * @throws InvalidRequestException
    *            if the wst:RST is invalid or malformed
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws InvalidTimeRangeException
    *            if the requested token validity time range is invalid or
    *            unsupported
    * @throws InvalidSecurityException
    *            if an error processing <wsse:Security> header occurs
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws RequestFailedException
    *            if a system failure occurred
    *
    * @return not null challenge response
    */
   public RequestSecurityTokenResponseCollectionType challenge(
      RequestSecurityTokenResponseType requestSecurityTokenResponse,
      SecurityHeaderType headerInfo) throws AuthenticationFailedException,
      InvalidSignatureException, UnsupportedSecurityTokenException,
      InvalidRequestException, RequestExpiredException,
      InvalidTimeRangeException, InvalidSecurityException, NoSuchIdPException,
      RequestFailedException;

   /**
    *
    * @param req
    *           issue request, mandatory.
    * @throws InvalidSignatureException
    *            when an authentication is via token and its signature cannot be
    *            validated against a set of trusted certificates
    * @throws AuthenticationFailedException
    *            when the user is not authenticated
    * @throws UnsupportedSecurityTokenException
    *            if the passed in security token is not supported
    * @throws InvalidRequestException
    *            if the wst:RST is invalid or malformed
    * @throws UnableToRenewException
    *            if renew request cannot be satisfied when the token is not
    *            renew-able.
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws InvalidTimeRangeException
    *            if the requested token validity time range is invalid or
    *            unsupported
    * @throws InvalidSecurityException
    *            if an error processing <wsse:Security> header occurs
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws RequestFailedException
    *            if a system failure occurred
    * @return not null issue response
    */
   public RequestSecurityTokenResponseCollectionType issue(Request req)
      throws AuthenticationFailedException, InvalidSignatureException,
      UnsupportedSecurityTokenException, InvalidRequestException,
      UnableToRenewException, RequestExpiredException,
      InvalidTimeRangeException, InvalidSecurityException, NoSuchIdPException,
      RequestFailedException;

   /**
    *
    * @param req
    *           renew request, mandatory.
    * @throws InvalidSignatureException
    *            when an authentication is via token and its signature cannot be
    *            validated against a set of trusted certificates
    * @throws AuthenticationFailedException
    *            when the user is not authenticated
    * @throws UnsupportedSecurityTokenException
    *            if the passed in security token is not supported
    * @throws InvalidRequestException
    *            if the wst:RST is invalid or malformed
    * @throws UnableToRenewException
    *            if renew request cannot be satisfied when the token is not
    *            renew-able.
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws InvalidTimeRangeException
    *            if the requested token validity time range is invalid or
    *            unsupported
    * @throws InvalidSecurityException
    *            if an error processing <wsse:Security> header occurs
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws RequestFailedException
    *            if a system failure occurred
    * @return not null renew response
    */
   public RequestSecurityTokenResponseType renew(Request req)
      throws AuthenticationFailedException, InvalidSignatureException,
      UnsupportedSecurityTokenException, InvalidRequestException,
      UnableToRenewException, RequestExpiredException,
      InvalidTimeRangeException, InvalidSecurityException, NoSuchIdPException,
      RequestFailedException;

   /**
    *
    * @param req
    *           validate request, mandatory.
    * @throws RequestExpiredException
    *            if the request has expired or is ahead of time
    * @throws InvalidSecurityException
    *            if an error processing <wsse:Security> header occurs
    * @throws InvalidSignatureException
    *            when an authentication is via token and its signature cannot be
    *            validated against a set of trusted certificates
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws RequestFailedException
    *            if a system failure occurred
    * @return not null validate response
    */
   public RequestSecurityTokenResponseType validate(Request req)
      throws RequestExpiredException, InvalidSecurityException,
      InvalidSignatureException, NoSuchIdPException, RequestFailedException;

}
