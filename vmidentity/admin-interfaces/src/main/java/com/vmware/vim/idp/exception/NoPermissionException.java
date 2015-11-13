/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

/**
 * Thrown when an operation is denied because of insufficient privileges.
 */
public class NoPermissionException extends SystemException {

   private static final long serialVersionUID = 636511272228424381L;

   public NoPermissionException(String message) {
      super(message);
   }

}
