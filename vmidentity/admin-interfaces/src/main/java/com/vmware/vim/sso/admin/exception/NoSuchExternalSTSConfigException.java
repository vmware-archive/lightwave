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
 * This exception is thrown by service methods to indicate the specified
 * TrustedSTS is not configured.
 */
public class NoSuchExternalSTSConfigException extends ServiceException {

   private static final long serialVersionUID = 5908906696389186047L;
   private final String missingTrustedSTSName;

   /**
    * @param missingTrustedSTSName name of the TrustedSTS config that is missing
    */
   public NoSuchExternalSTSConfigException(String missingTrustedSTSName) {
      super(getDefaultMessage(missingTrustedSTSName));
      assert missingTrustedSTSName != null && !missingTrustedSTSName.trim().isEmpty();
      this.missingTrustedSTSName = missingTrustedSTSName;
   }

   public String getMissingTrustedSTSName() {
      return missingTrustedSTSName;
   }

   private static String getDefaultMessage(String missingTrustedSTSName) {
      return String.format("Trusted STS config %s doesn't exist.", missingTrustedSTSName);
   }
}
