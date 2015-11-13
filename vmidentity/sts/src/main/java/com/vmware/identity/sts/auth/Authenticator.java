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
package com.vmware.identity.sts.auth;

import com.vmware.identity.sts.AuthenticationFailedException;
import com.vmware.identity.sts.NoSuchIdPException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.RequestFailedException;
import com.vmware.identity.sts.UnsupportedSecurityTokenException;

/**
 * Insert your comment for Authenticator here
 */
public interface Authenticator {

   /**
    * Authenticates an user using the token(s) carried in the request.
    *
    * The WS-Trust specification does not mandate how STS should respond to
    * requests containing more than one tokens ('claims') that prove its
    * identity. It is said that <i>
    * "(l.265) Verify that the claims in the token are sufficient to comply with the policy and that the message conforms to the policy."
    * </i>. Therefore the implementations are free to apply their own policy.
    *
    * @param req
    *           client request, required
    * @return not null result if the authentication has passed successfully, or
    *         null if there are no tokens found in the request
    * @throws AuthenticationFailedException
    *            if authentication token(s) are found but the credentials are
    *            invalid
    * @throws UnsupportedSecurityTokenException
    *            if some of the authentication tokens is not supported
    * @throws NoSuchIdPException
    *            if the IdP is missing
    * @throws RequestFailedException
    *            when request is valid but authentication is not successful
    *            because of other reasons
    */
   Result authenticate(Request req) throws AuthenticationFailedException,
      UnsupportedSecurityTokenException, NoSuchIdPException,
      RequestFailedException;
}
