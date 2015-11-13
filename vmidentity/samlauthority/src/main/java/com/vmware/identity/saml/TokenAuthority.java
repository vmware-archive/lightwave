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
package com.vmware.identity.saml;

/**
 * Implementations of this interface should be able to issue valid SAML tokens.
 */
public interface TokenAuthority {

   /**
    * Issues a SAML token based on the provided specification.
    *
    * @param spec
    * @throws UnsupportedTokenLifetimeException
    *            when there is a requested token start lifetime which is in the
    *            past or future, or when the signing certificate is not valid at
    *            the token issue instant
    * @throws DelegationException
    *            when delegation fails due to not allowed delegation or invalid
    *            delegate
    * @throws RenewException
    *            if renew request cannot be satisfied when the token is not
    *            renew-able.
    * @return {@link SamlToken} containing the SAML token
    */
   SamlToken issueToken(SamlTokenSpec spec);

}
