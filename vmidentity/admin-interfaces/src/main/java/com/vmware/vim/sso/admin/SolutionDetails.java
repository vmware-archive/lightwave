/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
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
