/* **********************************************************************
 * Copyright 2010 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Thrown to indicate that a Domain with the given name or alias already exists.
 */
public class DuplicateDomainNameException extends ServiceException {

   /**
    * Name of the duplicate domain.
    */
   private final String domainName;
   private final String domainAlias;
   private static final long serialVersionUID = -4284865881381641922L;

   public DuplicateDomainNameException(String name) {
      this(name, null);
   }
   public DuplicateDomainNameException(String name, String alias) {
      super(getDefaultMessage(name, alias));
      assert (null != name && !name.trim().isEmpty());
      this.domainName = name;
      this.domainAlias = alias;
   }

   public DuplicateDomainNameException(String message, String name, String alias) {
      super(message);
      assert (null != name && !name.trim().isEmpty());
      this.domainName = name;
      this.domainAlias = alias;
   }

   public DuplicateDomainNameException(Throwable t, String name) {
      super(getDefaultMessage(name, null), t);
      assert (null != name && !name.trim().isEmpty());
      this.domainName = name;
      this.domainAlias = null;
   }

   public String getDomainName() {
      return domainName;
   }

   public String getDomainAlias() {
      return domainAlias;
   }

   private static String getDefaultMessage(String name, String alias) {
      return String.format("Domain with name '%s' and alias '%s' already exists.", name, alias);
   }

}
