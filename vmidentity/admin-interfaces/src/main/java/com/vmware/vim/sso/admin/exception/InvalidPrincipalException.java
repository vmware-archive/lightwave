/* **********************************************************************
 * Copyright 2004 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods meaning incorrect <i>user id</i>
 * or <i>group id</i> argument
 */
public class InvalidPrincipalException extends ServiceException {

   private static final long serialVersionUID = -4633171543482214996L;
   private final String principal;

   public InvalidPrincipalException(Throwable cause, String principal) {
      super(getDefaultMessage(principal), cause);
      assert(null != principal);
      this.principal = principal;
   }

   public InvalidPrincipalException(String principal) {
      super(getDefaultMessage(principal));
      assert(null != principal);
      this.principal = principal;
   }

   public static String getDefaultMessage(String principal) {
      return "The specified principal (" + principal + ") is invalid.";
   }

   public String getPrincipal() {
      return principal;
   }
}
