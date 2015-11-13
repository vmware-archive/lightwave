/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown when an operation is denied because of insufficient privileges
 */
public class NoPermissionException extends SystemException {

   private static final long serialVersionUID = -7215417037749443581L;

   @SuppressWarnings("deprecation")
   public NoPermissionException(String message) {
      super(message);
   }

   @SuppressWarnings("deprecation")
   public NoPermissionException(String message, Throwable cause) {
      super(message, cause);
   }

}
