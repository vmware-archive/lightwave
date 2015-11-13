/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown when connection with server cannot be established.
 */
public class ConnectionException extends SystemException {
   private static final long serialVersionUID = 6069612710681803594L;

   public ConnectionException(String message, Throwable cause) {
      super(message, cause);
   }

}
