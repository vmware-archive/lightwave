/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown when the unexpected happened.
 */
public class Bug extends SystemException {
   private static final long serialVersionUID = -1538109311689564834L;

   @SuppressWarnings("deprecation")
   public Bug(Throwable t) {
      super(t);
   }

   @SuppressWarnings("deprecation")
   public Bug(String message) {
      super(message);
   }

   @SuppressWarnings("deprecation")
   public Bug(String message, Throwable cause) {
      super(message, cause);
   }
}
