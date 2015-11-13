/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

/**
 * Thrown when SSO user was not authenticated in case authentication is
 * required.
 */
public class NotAuthenticatedException extends ServiceException {

   private static final long serialVersionUID = 9089948302504799370L;

   public NotAuthenticatedException(String message) {
      super(message);
   }

   public NotAuthenticatedException(String message, Throwable cause) {
      super(message, cause);
   }

}
