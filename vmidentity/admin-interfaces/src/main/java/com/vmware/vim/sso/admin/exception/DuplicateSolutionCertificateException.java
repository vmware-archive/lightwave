/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin.exception;

/**
 * Thrown when the distinguished name ( DN ) of the provided solution
 * certificate is not unique.
 */
public class DuplicateSolutionCertificateException extends ServiceException {

   private static final long serialVersionUID = 5427613246143988975L;

   public DuplicateSolutionCertificateException(String message) {
      super(message);
   }

   public DuplicateSolutionCertificateException(String message, Throwable cause) {
      super(message, cause);
   }

}
