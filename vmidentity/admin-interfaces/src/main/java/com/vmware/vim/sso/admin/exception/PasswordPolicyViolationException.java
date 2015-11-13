/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown to indicate that the supplied password does not satisfy the Password
 * Policy currently effective on the server.
 */
public class PasswordPolicyViolationException extends ServiceException {
   private static final long serialVersionUID = 923716114223110030L;

   public PasswordPolicyViolationException(String message, Throwable cause) {
      super(message, cause);
   }

   public PasswordPolicyViolationException(String message) {
      super(message);
   }
}
