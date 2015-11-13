/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown to indicate that no SMTP configuration is set on the server
 *
 * @deprecated Since SSO 2.0 this exception has been deprecated.
 */
@Deprecated
public class SmtpConfigNotSetException extends ServiceException {

   private static final long serialVersionUID = 1512854391943177349L;

   /**
    * @deprecated Since SSO 2.0 SmtpConfigNotSetException has been deprecated.
    */
   @Deprecated
   public SmtpConfigNotSetException(String message) {
      super(message);
   }

   /**
    * @deprecated Since SSO 2.0 SmtpConfigNotSetException has been deprecated.
    */
   @Deprecated
   public SmtpConfigNotSetException(String message, Throwable cause) {
      super(message, cause);
   }
}
