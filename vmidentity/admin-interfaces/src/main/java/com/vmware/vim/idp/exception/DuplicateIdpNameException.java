/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

/**
 * Thrown to indicate that an IDP with the given name or alias already exists.
 */
public final class DuplicateIdpNameException extends ServiceException {

   private static final long serialVersionUID = -6604096374387740369L;

   private final String _idpName;

   public DuplicateIdpNameException(Throwable cause, String idpName) {
      super(getDefaultMessage(idpName), cause);

      assert idpName != null && !idpName.trim().isEmpty();
      _idpName = idpName;
   }

   public String getIdpName() {
      return _idpName;
   }

   private static String getDefaultMessage(String idpName) {
      return String.format("IDP with given name '%s' already exists.", idpName);
   }

}
