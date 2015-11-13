/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.idp.exception;

import com.vmware.vim.sso.admin.impl.util.ValidateUtil;

/**
 * Thrown to indicate that there's no IDP associated with a given name.
 */
public final class IdpNotFoundException extends ServiceException {

   private static final long serialVersionUID = 7421750026093775687L;

   private final String _idpName;

   public IdpNotFoundException(Throwable cause, String idpName) {
      super(getDefaultMessage(idpName), cause);

      ValidateUtil.validateNotEmpty(idpName, "IDP name");
      _idpName = idpName;
   }

   public IdpNotFoundException(String idpName) {
      super(getDefaultMessage(idpName));

      ValidateUtil.validateNotEmpty(idpName, "IDP name");
      _idpName = idpName;
   }

   public String getIdpName() {
      return _idpName;
   }

   private static String getDefaultMessage(String idpName) {
      return String.format("Cannot find IDP with name '%s'.", idpName);
   }

}
