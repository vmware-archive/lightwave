/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

/**
 * Base exception indicating bad condition of the system which prevents normal
 * completion of the given request. System errors might be caused by hardware
 * failures, software defects (bugs), network problems or when the service is
 * down.
 */
public abstract class SystemException extends RuntimeException {

   private static final long serialVersionUID = -5334186756507932465L;

   public SystemException(String message, Throwable cause) {
      super(message, cause);
   }

   public SystemException(String message) {
      super(message);
   }

   public SystemException(Throwable cause) {
      super(cause);
   }

}
