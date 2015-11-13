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

import java.util.List;

/**
 * @see ValidatableSamlToken
 * exposes information about token issuer and extended information on token delegates.
 */
public interface ValidatableSamlTokenEx extends ValidatableSamlToken
{
    /**
     * The issuer element of a token provides information about the issuer of a
     * SAML assertion.
     * @return Assertion's issuer.
     */
    public IssuerNameId getIssuerNameId();

    /**
     *  @see SamlToken.getDelegationChain()
     *  The information is extended with the subject format.
     */
    public List<TokenDelegateEx> getDelegationChainEx();

    /**
     * Implementations of this class will act as containers for a single
     * intermediary/delegate represented by the assertion.
     */
    public interface TokenDelegateEx extends TokenDelegate{

       /**
        * Identifies the delegate together with its format.
        * @return the delegate
        */
       public SubjectNameId getSubjectNameId();
    }
}
