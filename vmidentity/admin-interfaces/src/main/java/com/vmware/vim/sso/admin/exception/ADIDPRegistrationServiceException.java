package com.vmware.vim.sso.admin.exception;

/**
 * Abstract layer of exception indicating errors that are encountered
 * when adding AD IDS.
 */
public abstract class ADIDPRegistrationServiceException extends
      ServiceException {

   private static final long serialVersionUID = -6905548995268817881L;

   public ADIDPRegistrationServiceException(String message) {
      super(message);
   }

   public ADIDPRegistrationServiceException(String message, Throwable t) {
      super(message, t);
   }
}
