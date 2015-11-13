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

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;

import org.w3c.dom.Element;

import com.vmware.identity.token.impl.Constants;
import com.vmware.identity.token.impl.SamlTokenImpl;
import com.vmware.vim.sso.client.exception.InvalidTokenException;

/**
 * Factory providing methods for token operations. All instance methods are not
 * thread safe. SamlTokens produced by this factory are guaranteed to be valid.
 */
public final class DefaultTokenFactory implements SamlTokenFactory {

   private static final long DEFAULT_TOKEN_CLOCK_TOLERANCE = 0;

   private static final JAXBContext _jaxbContext = createJaxbContext();

   public DefaultTokenFactory() {
   }

   @Override
   public SamlToken parseToken(Element tokenRoot,
      X509Certificate[] trustedRootCertificates, long clockToleranceSec)
      throws InvalidTokenException {

      SamlTokenImpl samlTokenImpl = new SamlTokenImpl(tokenRoot, _jaxbContext);
      samlTokenImpl.validate(trustedRootCertificates, clockToleranceSec);
      return samlTokenImpl;
   }

   @Override
   public SamlToken parseToken(String tokenXml,
      X509Certificate[] trustedRootCertificates, long clockToleranceSec)
      throws InvalidTokenException {

      SamlTokenImpl samlTokenImpl = new SamlTokenImpl(tokenXml, _jaxbContext);
      samlTokenImpl.validate(trustedRootCertificates, clockToleranceSec);
      return samlTokenImpl;
   }

   @Override
   public SamlToken parseToken(String tokenXml,
      X509Certificate... trustedRootCertificates) throws InvalidTokenException {

      return parseToken(tokenXml, trustedRootCertificates,
         DEFAULT_TOKEN_CLOCK_TOLERANCE);
   }

   @Override
   public SamlToken parseToken(Element tokenRoot,
      X509Certificate... trustedRootCertificates) throws InvalidTokenException {

      return parseToken(tokenRoot, trustedRootCertificates,
         DEFAULT_TOKEN_CLOCK_TOLERANCE);
   }

   /**
    * Thread safe version of
    * {@link #parseToken(String, X509Certificate[], long)}
    */
   public static SamlToken createToken(String tokenXml,
      X509Certificate[] trustedRootCertificates, long clockToleranceSec)
      throws InvalidTokenException {

      SamlTokenImpl samlTokenImpl = new SamlTokenImpl(tokenXml, _jaxbContext);
      samlTokenImpl.validate(trustedRootCertificates, clockToleranceSec);
      return samlTokenImpl;
   }

   /**
    * Thread safe version of {@link #parseToken(String, X509Certificate...)}
    */
   public static SamlToken createToken(String tokenXml,
      X509Certificate... trustedRootCertificates) throws InvalidTokenException {

      return createToken(tokenXml, trustedRootCertificates,
         DEFAULT_TOKEN_CLOCK_TOLERANCE);
   }

   /**
    * Thread safe version of
    * {@link #parseToken(Element, X509Certificate[], long)}
    */
   public static SamlToken createTokenFromDom(Element tokenRoot,
      X509Certificate[] trustedRootCertificates, long clockToleranceSec)
      throws InvalidTokenException {

      SamlTokenImpl samlTokenImpl = new SamlTokenImpl(tokenRoot, _jaxbContext);
      samlTokenImpl.validate(trustedRootCertificates, clockToleranceSec);
      return samlTokenImpl;
   }

   /**
    * Thread safe version of {@link #parseToken(Element, X509Certificate...)}
    */
   public static SamlToken createTokenFromDom(Element tokenRoot,
      X509Certificate... trustedRootCertificates) throws InvalidTokenException {

      return createTokenFromDom(tokenRoot, trustedRootCertificates,
         DEFAULT_TOKEN_CLOCK_TOLERANCE);
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
