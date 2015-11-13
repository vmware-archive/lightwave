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

import com.vmware.vim.sso.client.ValidatableSamlToken;

/**
 * Implementations of this interface should be able to validate already created
 * token.
 */
public interface TokenValidator {

   /**
    * Validates if given token is valid.
    *
    * @param token
    *           {@link ValidatableSamlToken} object. Cannot be null.
    * @return validated token
    * @throws InvalidSignatureException
    *            when token signature cannot is not valid
    * @throws InvalidTokenException
    *            when token cannot be validated.
    * @throws SystemException
    *            when system error occurs
    */
    ServerValidatableSamlToken validate(ServerValidatableSamlToken token)
      throws InvalidSignatureException, InvalidTokenException, SystemException;
}
