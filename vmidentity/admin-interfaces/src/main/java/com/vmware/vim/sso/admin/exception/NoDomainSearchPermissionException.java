/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * **********************************************************************/

package com.vmware.vim.sso.admin.exception;

/**
 * Indicate that search is impossible due to lack of search rights in the
 * domain.
 */
public final class NoDomainSearchPermissionException extends SystemException {

   private static final long serialVersionUID = -370719619302641493L;

   public NoDomainSearchPermissionException(String message) {
      super(message);
   }

   public NoDomainSearchPermissionException(String message, Throwable cause) {
      super(message, cause);
   }
}

