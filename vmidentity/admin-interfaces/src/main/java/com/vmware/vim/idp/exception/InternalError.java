/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

/**
 * Thrown to indicate some unexpected or internal failure has occurred in the
 * SSO Server.
 * <p>
 * This exception usually indicates a bug in the server, corrupt installation
 * file(s), corrupt database or incompatibility of the underlying platform (JVM,
 * OS, etc.).
 */
public class InternalError extends SystemException {

   private static final long serialVersionUID = -7413434409502789187L;

   public InternalError(String message) {
      super(message);
   }

   public InternalError(Throwable cause) {
      super(cause);
   }

   public InternalError(String message, Throwable cause) {
      super(message, cause);
   }

}
