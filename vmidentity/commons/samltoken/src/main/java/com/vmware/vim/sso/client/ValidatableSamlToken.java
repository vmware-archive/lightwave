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
package com.vmware.vim.sso.client;

import java.security.cert.X509Certificate;

import com.vmware.vim.sso.client.exception.InvalidSignatureException;
import com.vmware.vim.sso.client.exception.InvalidTimingException;
import com.vmware.vim.sso.client.exception.InvalidTokenException;
import com.vmware.vim.sso.client.exception.MalformedTokenException;

/**
 * Instances of this interface are not guaranteed to be valid (i.e. signed by a
 * trusted authority, within lifetime range). Trying to invoke any method
 * different than validate should result in a runtime exception.
 */
public interface ValidatableSamlToken extends SamlToken {

   /**
    * Validates that the token is signed using a trusted certificate and is
    * within the lifetime range
    *
    * @param trustedRootCertificates
    *           List of trusted root STS certificates that ValidatableSamlToken
    *           will use when validating the token's signature. Required.
    * @param clockToleranceSec
    *           Tolerate that many seconds of discrepancy between the token's
    *           sender clock and the local system clock when validating the
    *           token's start and expiration time. This effectively "expands"
    *           the token's validity period with the given number of seconds.
    * @return
    * @throws InvalidSignatureException
    *            when the signature cannot be verified.
    * @throws InvalidTimingException
    *            when times in the token are malformed, invalid or divergent at
    *            the time of validation
    * @throws MalformedTokenException
    *            when the token or some of its elements are malformed
    * @throws InvalidTokenException
    *            if the token or some of its elements is invalid or malformed
    */
   public void validate(X509Certificate[] trustedRootCertificates,
      long clockToleranceSec) throws InvalidTokenException;
}
