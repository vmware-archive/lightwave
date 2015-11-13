/* **********************************************************************
 * Copyright 2013 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso.admin.exception;

/**
 * Indicates that native AD Identity Source registration cannot be performed for
 * one of the following reasons:
 * <ul>
 * <li>SSO server machine is not joined to AD domain</li>
 * <li>other native AD Identity Source has already been registered</li>
 * <li>other LDAP AD Identity Sources has already been registered</li>
 * <li>the LDAP AD Identity Sources cannot be replaced by a native AD Identity
 * Source without loss of functionality</li>
 * </ul>
 */
public class NativeADRegistrationException extends ServiceException {

   private static final long serialVersionUID = 8894008357764625568L;

   public NativeADRegistrationException(String message) {
      super(message);
   }

   public NativeADRegistrationException(String message, Throwable cause) {
      super(message, cause);
   }
}
