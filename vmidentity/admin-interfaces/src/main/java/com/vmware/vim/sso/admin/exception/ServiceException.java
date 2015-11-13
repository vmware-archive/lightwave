/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Base exception indicating errors that are function of the input and the
 * current system state. In most of the cases the client can determine what is
 * wrong and knows how to recover.
 */
public abstract class ServiceException extends Exception implements Error {

   private static final long serialVersionUID = 7394465536783144111L;

   public ServiceException(String message, Throwable cause) {
      super(message, cause);
   }

   public ServiceException(String message) {
      super(message);
   }

}
