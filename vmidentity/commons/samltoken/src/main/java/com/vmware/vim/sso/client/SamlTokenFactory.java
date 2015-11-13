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

import org.w3c.dom.Element;

import com.vmware.vim.sso.client.exception.InvalidTokenException;

/**
 * Implementations of this interface will serve as a SAML token factories
 */
public interface SamlTokenFactory {

   /**
    * Create a SamlToken object from DOM Element, performing syntactic and
    * semantical validation of the XML tree.
    * <p>
    * The token will retain a <i>copy</i> of the original element (not the
    * element itself).
    *
    * @param tokenRoot
    *           The root element of the subtree containing the SAML token.
    * @param trustedRootCertificates
    *           The public signing certificate(s) of the security token service,
    *           needed for token validation. Must not be {@code null}, there
    *           must be at least one certificate, and none of the supplied
    *           certificates may be {@code null}.
    * @param clockToleranceSec
    *           Tolerate that many seconds of discrepancy between the token's
    *           sender clock and the local system clock when validating the
    *           token's start and expiration time. This effectively "expands"
    *           the token's validity period with the given number of seconds.
    * @return The parsed and validated Token object
    * @throws InvalidTokenException
    *            Indicates syntactic (e.g. contains invalid elements or missing
    *            required elements) or semantic (e.g. subject name in unknown
    *            format) error, expired or not yet valid token or failure to
    *            validate the signature against the trustedRootCertificates.
    */
   public SamlToken parseToken(Element tokenRoot,
      X509Certificate[] trustedRootCertificates, long clockToleranceSec) throws InvalidTokenException;

   /**
    * Create a SamlToken object from DOM Element.
    * <p>
    * This is a convenience overload of
    * {@link #parseToken(Element, X509Certificate[], long)} with clockTolerance
    * = 0.
    */
   public SamlToken parseToken(Element tokenRoot,
      X509Certificate... trustedRootCertificates) throws InvalidTokenException;

   /**
    * Create a SamlToken object from string representation, performing syntactic
    * and semantical validation of the XML tree.
    *
    * @param tokenXml
    *           The xml representation of a SAML token. Not {@code null}.
    * @param trustedRootCertificates
    *           The public signing certificate(s) of the security token service,
    *           needed for token validation. Must not be {@code null}, there
    *           must be at least one certificate, and none of the supplied
    *           certificates may be {@code null}.
    * @param clockToleranceSec
    *           Tolerate that many seconds of discrepancy between the token's
    *           sender clock and the local system clock when validating the
    *           token's start and expiration time. This effectively "expands"
    *           the token's validity period with the given number of seconds.
    * @return The parsed and validated Token object
    * @throws InvalidTokenException
    *            Indicates syntactic (e.g. contains invalid elements or missing
    *            required elements) or semantic (e.g. subject name in unknown
    *            format) error, expired or not yet valid token or failure to
    *            validate the signature against the trustedRootCertificates.
    */
   public SamlToken parseToken(String tokenXml,
      X509Certificate[] trustedRootCertificates, long clockToleranceSec)
      throws InvalidTokenException;

   /**
    * Create a SamlToken object from string representation.
    * <p>
    * This is a convenience overload of
    * {@link #parseToken(String, X509Certificate[], long)} with clockTolerance
    * = 0.
    */
   public SamlToken parseToken(String tokenXml,
      X509Certificate... trustedRootCertificates) throws InvalidTokenException;
}
