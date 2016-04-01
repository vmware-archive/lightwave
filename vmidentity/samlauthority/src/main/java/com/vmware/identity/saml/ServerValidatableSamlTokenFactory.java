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


import org.w3c.dom.Element;

import com.vmware.identity.saml.impl.ServerValidatableSamlTokenImpl;

/**
 * A factory for ServerValidatableSamlToken.
 */
public class ServerValidatableSamlTokenFactory
{
   /**
    * Create a ServerValidatableSamlToken object from DOM Element, performing
    * syntactic and semantical validation of the XML tree. Note that signature
    * validation will not be performed. Caller has to call .validate before other
    * methods are usable.
    * <p>
    * The token will retain a <i>copy</i> of the original element (not the
    * element itself).
    *
    * @param tokenRoot
    *           The root element of the subtree containing the SAML token.
    * @return The parsed Token object
    * @throws InvalidTokenException
    *            Indicates syntactic (e.g. contains invalid elements or missing
    *            required elements) or semantic (e.g. subject name in unknown
    *            format) error, expired or not yet valid token.
    */
   public ServerValidatableSamlToken parseToken(Element tokenRoot)
      throws InvalidTokenException {
      return new ServerValidatableSamlTokenImpl(tokenRoot);
   }
}