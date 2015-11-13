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

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;

import org.w3c.dom.Element;

import com.vmware.identity.token.impl.Constants;
import com.vmware.identity.token.impl.SamlTokenImpl;
import com.vmware.vim.sso.client.exception.InvalidTokenException;

/**
 * Factory providing methods for token operations. Not thread safe.
 */
public final class ValidatableTokenFactory {

   private static final JAXBContext _jaxbContext = createJaxbContext();

   /**
    * Create a ValidatableSamlToken object from DOM Element, performing
    * syntactic and semantical validation of the XML tree. Note that signature
    * validation will not be performed.
    * <p>
    * The token will retain a <i>copy</i> of the original element (not the
    * element itself).
    *
    * @param tokenRoot
    *           The root element of the subtree containing the SAML token.
    * @return The parsed and validated Token object
    * @throws InvalidTokenException
    *            Indicates syntactic (e.g. contains invalid elements or missing
    *            required elements) or semantic (e.g. subject name in unknown
    *            format) error, expired or not yet valid token.
    */
   public ValidatableSamlToken parseValidatableToken(Element tokenRoot)
      throws InvalidTokenException {
      return new SamlTokenImpl(tokenRoot, _jaxbContext);
   }

   /**
    * Create a ValidatableSamlToken object from DOM Element, performing
    * syntactic and semantical validation of the XML tree. Note that signature
    * validation will not be performed.
    * <p>
    * The token will retain a <i>copy</i> of the original element (not the
    * element itself).
    *
    * @param tokenRoot
    *           The root element of the subtree containing the SAML token.
    * @return The parsed and validated Token object
    * @throws InvalidTokenException
    *            Indicates syntactic (e.g. contains invalid elements or missing
    *            required elements) or semantic (e.g. subject name in unknown
    *            format) error, expired or not yet valid token.
    */
   public ValidatableSamlTokenEx parseValidatableTokenEx(Element tokenRoot)
      throws InvalidTokenException {
      return new SamlTokenImpl(tokenRoot, _jaxbContext, true);
   }

   /**
    * @return {@link JAXBContext} for {@link Constants#ASSERTION_JAXB_PACKAGE}
    */
   private static JAXBContext createJaxbContext() {
      try {
         return JAXBContext.newInstance(Constants.ASSERTION_JAXB_PACKAGE);
      } catch (JAXBException e) {
         throw new IllegalStateException("Cannot initialize JAXBContext.", e);
      }
   }
}
