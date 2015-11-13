/* **********************************************************************
 * Copyright 2012 VMware, Inc.  All rights reserved.
 * **********************************************************************
 * $Id$
 * $DateTime$
 * $Change$
 * $Author$
 * *********************************************************************/

package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods to indicate Certification
 * configuration error.
 */
public class ExternalSTSCertChainInvalidTrustedPathException extends
      ServiceException {

   private static final long serialVersionUID = -2555889884074247140L;
   private final String name;

   /**
    * @param name
    */
   public ExternalSTSCertChainInvalidTrustedPathException(String name) {
      super(getDefaultMessage(name));
      assert name != null && !name.trim().isEmpty();
      this.name = name;
   }

   public String getName() {
      return name;
   }

   private static String getDefaultMessage(String name) {
      return String.format("Certpath for [%s] doesn't exist.", name);
   }
}
