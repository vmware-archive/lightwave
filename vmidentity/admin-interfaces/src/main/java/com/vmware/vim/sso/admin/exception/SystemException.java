/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Base exception indicating bad condition of the system which prevents normal
 * completion of the given request. System errors might be caused by hardware
 * failures, software defects (bugs), network problems or when the service is
 * down.
 */
//TODO make this class abstract
public class SystemException extends RuntimeException implements Error {
   private static final long serialVersionUID = -4798752964609464366L;

   @Deprecated
   public SystemException(String message) {
      super(message);
   }

   @Deprecated
   public SystemException(String message, Throwable cause) {
      super(message, cause);
   }

   @Deprecated
   public SystemException(Throwable t) {
      super(t);
   }
}
