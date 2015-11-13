/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown to indicate some unexpected or internal failure has occurred in the
 * SSO Server.
 * <p>
 * This exception usually indicates a bug in the server, corrupt installation
 * file(s), corrupt database or incompatibility of the underlying platform (JVM,
 * OS, etc.).
 */
@SuppressWarnings("deprecation")
public class InternalError extends SystemException {

   private static final long serialVersionUID = -5366022288056937572L;

   public InternalError(String message) {
      super(message);
   }

   public InternalError(String message, Throwable cause) {
      super(message, cause);
   }
}
