/* **********************************************************************
 * Copyright 2014 VMware, Inc.  All rights reserved.
 * *********************************************************************/

package com.vmware.vim.sso.admin.exception;

/**
 * This exception is thrown by service methods to indicate that the specified
 * config does not exist.
 */
public class NoSuchConfigException extends ServiceException
{
    private static final long serialVersionUID = -1958364046661516980L;

    private final String missingConfigName;

   /**
    * Create a new exception for a missing configuration
    *
    * @param missingConfigName
    *           Name of the missing configuration
    */
   public NoSuchConfigException(String missingConfigName) {
       super(getDefaultMessage(missingConfigName));
       assert missingConfigName != null && !missingConfigName.trim().isEmpty();
       this.missingConfigName = missingConfigName;
   }

   public String getMissingConfigName() {
       return missingConfigName;
   }

   private static String getDefaultMessage(String missingConfigName) {
       return String.format("Config %s doesn't exist.", missingConfigName);
   }
}
