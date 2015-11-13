/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

import java.security.cert.X509Certificate;

/**
 * Thrown to indicate an error during certificate delete operation.
 */
public class CertificateDeletionException extends ServiceException {
   private static final long serialVersionUID = -2189201183557054923L;

   private final X509Certificate certificate;

   public CertificateDeletionException(X509Certificate certificate) {
      super(getDefaultMessage(certificate));
      this.certificate = certificate;
   }

   public X509Certificate getCertificate() {
      return certificate;
   }

   private static String getDefaultMessage(X509Certificate cert) {
      assert cert != null;
      return String.format("Certificate '%s' cannot be deleted.", cert
         .getSubjectDN());
   }
}
