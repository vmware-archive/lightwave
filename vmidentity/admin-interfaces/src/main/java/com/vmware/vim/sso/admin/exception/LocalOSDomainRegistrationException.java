/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * **********************************************************************
 * $Id$
 * $DateTime$
 * $Change$
 * $Author$
 * *********************************************************************/

package com.vmware.vim.sso.admin.exception;

/**
 * Thrown upon local OS domain registration operation for one of the following
 * reasons:
 * <ul>
 * <li>SSO server instance does not support local OS domain because it is
 * installed either in multisite or HA mode</li>
 * <li>the local OS domain has already been registered</li>
 * </ul>
 */
public class LocalOSDomainRegistrationException extends SystemException {

   private static final long serialVersionUID = -8808827872181942047L;

   @SuppressWarnings("deprecation")
   public LocalOSDomainRegistrationException(String message) {
      super(message);
   }

   @SuppressWarnings("deprecation")
   public LocalOSDomainRegistrationException(String message, Throwable cause) {
      super(message, cause);
   }

}
