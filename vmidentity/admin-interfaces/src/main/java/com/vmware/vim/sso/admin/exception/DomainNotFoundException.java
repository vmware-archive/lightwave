/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown to indicate that there's no Domain associated with a given name.
 */
public class DomainNotFoundException extends ServiceException {
   private static final long serialVersionUID = 3834601685482158291L;
   /**
    * Name of the domain which could not be found;
    */
   private final String name;

   public DomainNotFoundException(Throwable cause, String name) {
      super(getDefaultMessage(name), cause);
      assert (null != name && !name.trim().isEmpty());
      this.name = name;
   }

   public DomainNotFoundException(String name) {
      super(getDefaultMessage(name));
      assert (null != name && !name.trim().isEmpty());
      this.name = name;
   }

   private static String getDefaultMessage(String name) {
      return String.format("Domain '%s' does not exist.", name);
   }

   public String getName() {
      return name;
   }
}
