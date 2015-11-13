/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

/**
 * Thrown on client creation when the the provided authentication data is not
 * valid
 */
public final class InvalidCredentialsException extends SystemException {

   private static final long serialVersionUID = 8582107109899435238L;

   public InvalidCredentialsException(String message) {
      super(message);
   }

}
