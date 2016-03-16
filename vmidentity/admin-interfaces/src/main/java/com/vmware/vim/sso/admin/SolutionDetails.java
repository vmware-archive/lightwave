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
package com.vmware.vim.sso.admin;

import java.security.cert.X509Certificate;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Attributes for solution types of users; A composite part of
 * {@link SolutionUser}. This class is immutable.
 */
public final class SolutionDetails extends PrincipalDetails {

   /**
    * Certificate
    */
   private final X509Certificate _certificate;

   /**
    * Constructs solution details by certificate
    *
    * @param certificate
    *           certificate; requires {@code non-null} value
    */
   public SolutionDetails(X509Certificate certificate) {
      super(null);
      ValidateUtil.validateNotNull(certificate, "certificate");
      _certificate = certificate;
   }

   /**
    * Constructs solution details by certificate and description
    *
    * @param certificate
    *           certificate; requires {@code non-null} value
    * @param description
    *           the description to set; requires {@code non-null} value
    */
   public SolutionDetails(X509Certificate certificate, String description) {

      super(description);

      ValidateUtil.validateNotNull(certificate, "certificate");
      ValidateUtil.validateNotNull(description, "description");

      _certificate = certificate;
   }

   /**
    * Retrieve solution's certificate
    *
    * @return a valid certificate
    */
   public X509Certificate getCertificate() {
      return _certificate;
   }

   /**
    * {@inheritDoc}
    */
   @Override
   protected Object[] getIdentityState() {
      return new Object[] { getDescription(), getCertificate() };
   }

}
