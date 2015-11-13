/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * This exception is raised when the supplied password policy is invalid (e.g.
 * negative prohibitedPreviousPasswordsCount, minLength > maxLength in the
 * PasswordFormat, etc.)
 */
public class InvalidPasswordPolicyException extends ServiceException {

   private static final long serialVersionUID = -710030129836245974L;

   public InvalidPasswordPolicyException(String message) {
      super(message);
   }

   public InvalidPasswordPolicyException(String message, Throwable cause) {
      super(message, cause);
   }
}
