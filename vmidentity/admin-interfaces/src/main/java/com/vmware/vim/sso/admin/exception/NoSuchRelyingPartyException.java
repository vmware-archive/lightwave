/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown to indicate that the relying party mentioned does not exist.
 */
public final class NoSuchRelyingPartyException extends ServiceException {
   private static final long serialVersionUID = -2732893418459539538L;

   private final String name;

   public NoSuchRelyingPartyException(String name) {
      super(getDefaultMessage(name));
      assert name != null && !name.trim().isEmpty();
      this.name = name;
   }

   public String getName() {
      return name;
   }

   private static String getDefaultMessage(String name) {
      return String.format("Relying party %s doesn't exist.", name);
   }
}
